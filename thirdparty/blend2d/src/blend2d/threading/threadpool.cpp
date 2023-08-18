// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include "../api-build_p.h"
#include "../bitarray_p.h"
#include "../runtime_p.h"
#include "../support_p.h"
#include "../threading/atomic_p.h"
#include "../threading/conditionvariable_p.h"
#include "../threading/mutex_p.h"
#include "../threading/threadpool_p.h"

#ifdef _WIN32
  #include <process.h>
#endif

// ============================================================================
// [Globals]
// ============================================================================

static BLThreadPoolVirt blThreadPoolVirt;

// ============================================================================
// [BLThreadPool - Internal]
// ============================================================================

class BLInternalThreadPool : public BLThreadPool {
public:
  enum : uint32_t { kMaxThreadCount = 64 };
  typedef BLFixedBitArray<BLBitWord, kMaxThreadCount> BitArray;

  volatile size_t refCount;
  volatile uint32_t stackSize;
  volatile uint32_t maxThreadCount;
  volatile uint32_t createdThreadCount;
  volatile uint32_t pooledThreadCount;
  volatile uint32_t acquiredThreadCount;
  volatile uint32_t destroyWaitTimeInMS;
  volatile uint32_t waitingOnDestroy;

  BLMutex mutex;
  BLConditionVariable destroyCondition;
  BLThreadAttributes threadAttributes;
  BitArray pooledThreadBits;
  BLThread* threads[kMaxThreadCount];

  // No need to explicitly initialize anything as it should be zero initialized.
  explicit BLInternalThreadPool(size_t initialRefCount = 1) noexcept
    : BLThreadPool { &blThreadPoolVirt },
      refCount(initialRefCount),
      stackSize(0),
      maxThreadCount(kMaxThreadCount),
      createdThreadCount(0),
      pooledThreadCount(0),
      acquiredThreadCount(0),
      destroyWaitTimeInMS(100),
      waitingOnDestroy(0),
      mutex(),
      destroyCondition(),
      threadAttributes {},
      pooledThreadBits {},
      threads {} { init(); }

  ~BLInternalThreadPool() noexcept {
    uint32_t numTries = 5;
    uint64_t waitTime = (uint64_t(destroyWaitTimeInMS) * 1000u) / numTries;

    do {
      cleanup();

      BLLockGuard<BLMutex> guard(mutex);
      if (blAtomicFetch(&createdThreadCount) != 0) {
        waitingOnDestroy = 1;
        if (destroyCondition.waitFor(mutex, waitTime) == BL_SUCCESS)
          break;
      }
    } while (--numTries);

    destroy();
  }

  BL_INLINE void init() noexcept {}
  BL_INLINE void destroy() noexcept {}
};

// ============================================================================
// [BLThreadPool - Create / Destroy]
// ============================================================================

static void blThreadPoolDestroy(BLInternalThreadPool* self) noexcept {
  self->~BLInternalThreadPool();
  free(self);
}

BLThreadPool* blThreadPoolCreate() noexcept {
  void* p = malloc(sizeof(BLInternalThreadPool));
  if (!p)
    return nullptr;
  return new(p) BLInternalThreadPool();
}

// ============================================================================
// [BLThreadPool - AddRef / Release]
// ============================================================================

static BLThreadPool* BL_CDECL blThreadPoolAddRef(BLThreadPool* self_) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  if (self->refCount != 0)
    blAtomicFetchAdd(&self->refCount);
  return self;
}

static BLResult BL_CDECL blThreadPoolRelease(BLThreadPool* self_) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  if (self->refCount != 0 && blAtomicFetchSub(&self->refCount) == 1)
    blThreadPoolDestroy(self);
  return BL_SUCCESS;
}

// ============================================================================
// [BLThreadPool - Properties]
// ============================================================================

static uint32_t BL_CDECL blThreadPoolMaxThreadCount(const BLThreadPool* self_) noexcept {
  const BLInternalThreadPool* self = static_cast<const BLInternalThreadPool*>(self_);
  return self->maxThreadCount;
}

static uint32_t BL_CDECL blThreadPoolPooledThreadCount(const BLThreadPool* self_) noexcept {
  const BLInternalThreadPool* self = static_cast<const BLInternalThreadPool*>(self_);
  return self->pooledThreadCount;
}

static BLResult BL_CDECL blThreadPoolSetThreadAttributes(BLThreadPool* self_, const BLThreadAttributes* attributes) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  BLLockGuard<BLMutex> guard(self->mutex);

  // Verify that the provided `stackSize` is okay.
  //   - POSIX   - Minimum stack size is `PTHREAD_STACK_MIN` and some
  //               implementations may enforce alignment to page-size.
  //   - WINDOWS - Minimum stack size is `SYSTEM_INFO::dwAllocationGranularity`,
  //               alignment should follow the granularity as well, however,
  //               WinAPI would align stack size if it's not properly aligned.
  uint32_t stackSize = attributes->stackSize;
  if (stackSize) {
    const BLRuntimeSystemInfo& si = blRuntimeContext.systemInfo;
    if (stackSize < si.threadStackSize || !blIsAligned(stackSize, si.allocationGranularity))
      return blTraceError(BL_ERROR_INVALID_VALUE);
  }

  self->threadAttributes = *attributes;
  return BL_SUCCESS;
}

// ============================================================================
// [BLThreadPool - Cleanup]
// ============================================================================

static void blThreadPoolThreadExitFunc(BLThread* thread, void* data) noexcept {
  BLInternalThreadPool* threadPool = static_cast<BLInternalThreadPool*>(data);
  thread->destroy();

  if (blAtomicFetchSub(&threadPool->createdThreadCount) == 1) {
    if (threadPool->mutex.protect([&]() { return threadPool->waitingOnDestroy; }))
      threadPool->destroyCondition.signal();
  }
}

static uint32_t BL_CDECL blThreadPoolCleanup(BLThreadPool* self_) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  BLLockGuard<BLMutex> guard(self->mutex);

  uint32_t n = 0;
  uint32_t bwIndex = 0;
  uint32_t pooledThreadCount = self->pooledThreadCount;

  if (!pooledThreadCount)
    return 0;

  do {
    BLBitWord mask = self->pooledThreadBits.data[bwIndex];
    BLBitWordIterator<BLBitWord> it(mask);

    while (it.hasNext()) {
      uint32_t threadIndex = bwIndex * blBitSizeOf<BLBitWord>() + it.next();
      BLThread* thread = self->threads[threadIndex];

      self->threads[threadIndex] = nullptr;
      thread->quit();

      n++;
    }
    self->pooledThreadBits.data[bwIndex] = 0;
  } while (++bwIndex < BLInternalThreadPool::BitArray::kFixedArraySize);

  self->pooledThreadCount = pooledThreadCount - n;
  return n;
}

// ============================================================================
// [BLThreadPool - Acquire / Release]
// ============================================================================

static void blThreadPoolReleaseThreadsInternal(BLInternalThreadPool* self, BLThread** threads, uint32_t n) noexcept {
  uint32_t i = 0;
  uint32_t bwIndex = 0;

  do {
    BLBitWord mask = self->pooledThreadBits.data[bwIndex] ^ blBitOnes<BLBitWord>();
    BLBitWordIterator<BLBitWord> it(mask);

    while (it.hasNext()) {
      uint32_t bit = it.next();
      mask ^= blBitAt<BLBitWord>(bit);

      uint32_t threadIndex = bwIndex * blBitSizeOf<BLBitWord>() + bit;
      BL_ASSERT(self->threads[threadIndex] == nullptr);

      BLThread* thread = threads[i];
      self->threads[threadIndex] = thread;

      if (++i >= n)
        break;
    }

    self->pooledThreadBits.data[bwIndex] = mask ^ blBitOnes<BLBitWord>();
  } while (i < n && ++bwIndex < BLInternalThreadPool::BitArray::kFixedArraySize);

  // This shouldn't happen. What is acquired must be released. If more threads
  // are released than acquired it means the API was used wrongly. Not sure we
  // want to recover.
  BL_ASSERT(i == n);

  self->pooledThreadCount += n;
  self->acquiredThreadCount -= n;
}

static uint32_t blThreadPoolAcquireThreadsInternal(BLInternalThreadPool* self, BLThread** threads, uint32_t n, uint32_t flags, BLResult* reasonOut) noexcept {
  BLResult reason = BL_SUCCESS;
  uint32_t nAcquired = 0;

  uint32_t pooledThreadCount = self->pooledThreadCount;
  uint32_t acquiredThreadCount = self->acquiredThreadCount;

  if (n > pooledThreadCount) {
    uint32_t createThreadCount = n - pooledThreadCount;
    uint32_t remainingThreadCount = self->maxThreadCount - (acquiredThreadCount + pooledThreadCount);

    if (createThreadCount > remainingThreadCount) {
      if (flags & BL_THREAD_POOL_ACQUIRE_FLAG_ALL_OR_NOTHING) {
        *reasonOut = BL_ERROR_THREAD_POOL_EXHAUSTED;
        return 0;
      }
      createThreadCount = remainingThreadCount;
    }

    while (nAcquired < createThreadCount) {
      reason = blThreadCreate(&threads[nAcquired], &self->threadAttributes, blThreadPoolThreadExitFunc, self);

      if (reason != BL_SUCCESS) {
        if (flags & BL_THREAD_POOL_ACQUIRE_FLAG_ALL_OR_NOTHING) {
          self->acquiredThreadCount += nAcquired;
          blAtomicFetchAdd(&self->createdThreadCount, nAcquired);

          blThreadPoolReleaseThreadsInternal(self, threads, nAcquired);
          *reasonOut = reason;
          return 0;
        }

        // Don't try again... The `reason` will be propagated to the caller.
        break;
      }

      nAcquired++;
    }

    blAtomicFetchAdd(&self->createdThreadCount, nAcquired);
  }

  uint32_t bwIndex = 0;
  uint32_t nAcqPrev = nAcquired;

  while (nAcquired < n) {
    BLBitWord mask = self->pooledThreadBits.data[bwIndex];
    BLBitWordIterator<BLBitWord> it(mask);

    while (it.hasNext()) {
      uint32_t bit = it.next();
      mask ^= blBitAt<BLBitWord>(bit);

      uint32_t threadIndex = bwIndex * blBitSizeOf<BLBitWord>() + bit;
      BLThread* thread = self->threads[threadIndex];

      BL_ASSERT(thread != nullptr);
      self->threads[threadIndex] = nullptr;

      threads[nAcquired] = thread;
      if (++nAcquired == n)
        break;
    };

    self->pooledThreadBits.data[bwIndex] = mask;
    if (++bwIndex >= BLInternalThreadPool::BitArray::kFixedArraySize)
      break;
  }

  self->pooledThreadCount -= nAcquired - nAcqPrev;
  self->acquiredThreadCount += nAcquired;

  *reasonOut = reason;
  return nAcquired;
}

static uint32_t BL_CDECL blThreadPoolAcquireThreads(BLThreadPool* self_, BLThread** threads, uint32_t n, uint32_t flags, BLResult* reason) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  BLLockGuard<BLMutex> guard(self->mutex);

  return blThreadPoolAcquireThreadsInternal(self, threads, n, flags, reason);
}

static void BL_CDECL blThreadPoolReleaseThreads(BLThreadPool* self_, BLThread** threads, uint32_t n) noexcept {
  BLInternalThreadPool* self = static_cast<BLInternalThreadPool*>(self_);
  BLLockGuard<BLMutex> guard(self->mutex);

  return blThreadPoolReleaseThreadsInternal(self, threads, n);
}

// ============================================================================
// [BLThreadPool - Global]
// ============================================================================

static BLWrap<BLInternalThreadPool> blGlobalThreadPool;
BLThreadPool* blThreadPoolGlobal() noexcept { return &blGlobalThreadPool; }

// ============================================================================
// [BLThreadPool - RuntimeInit]
// ============================================================================

static void BL_CDECL blThreadPoolRtShutdown(BLRuntimeContext* rt) noexcept {
  BL_UNUSED(rt);
  blGlobalThreadPool.destroy();
}

static void BL_CDECL blThreadPoolRtCleanup(BLRuntimeContext* rt, uint32_t cleanupFlags) noexcept {
  BL_UNUSED(rt);
  if (cleanupFlags & BL_RUNTIME_CLEANUP_THREAD_POOL)
    blGlobalThreadPool->cleanup();
}

void blThreadPoolRtInit(BLRuntimeContext* rt) noexcept {
  // BLThreadPool virtual table.
  blThreadPoolVirt.addRef = blThreadPoolAddRef;
  blThreadPoolVirt.release = blThreadPoolRelease;
  blThreadPoolVirt.maxThreadCount = blThreadPoolMaxThreadCount;
  blThreadPoolVirt.pooledThreadCount = blThreadPoolPooledThreadCount;
  blThreadPoolVirt.setThreadAttributes = blThreadPoolSetThreadAttributes;
  blThreadPoolVirt.cleanup = blThreadPoolCleanup;
  blThreadPoolVirt.acquireThreads = blThreadPoolAcquireThreads;
  blThreadPoolVirt.releaseThreads = blThreadPoolReleaseThreads;

  // BLThreadPool built-in global instance.
  BLThreadAttributes attrs {};
  attrs.stackSize = rt->systemInfo.threadStackSize;

  blGlobalThreadPool.init(0);
  blGlobalThreadPool->setThreadAttributes(attrs);

  rt->shutdownHandlers.add(blThreadPoolRtShutdown);
  rt->cleanupHandlers.add(blThreadPoolRtCleanup);
}

// ============================================================================
// [BLThreadPool - Unit Tests]
// ============================================================================

#if defined(BL_TEST)
struct ThreadTestData {
  uint32_t iter;
  volatile uint32_t counter;
  volatile bool waiting;
  BLMutex mutex;
  BLConditionVariable condition;

  ThreadTestData() noexcept
    : iter(0),
      counter(0),
      waiting(false) {}
};

static void BL_CDECL test_thread_entry(BLThread* thread, void* data_) noexcept {
  ThreadTestData* data = static_cast<ThreadTestData*>(data_);
  INFO("[#%u] Thread %p running\n", data->iter, thread);
}

static void BL_CDECL test_thread_done(BLThread* thread, void* data_) noexcept {
  ThreadTestData* data = static_cast<ThreadTestData*>(data_);
  INFO("[#%u] Thread %p done\n", data->iter, thread);

  if (blAtomicFetchSub(&data->counter) == 1)
    if (data->mutex.protect([&]() { return data->waiting; }))
      data->condition.signal();
}

UNIT(thread_pool) {
  BLInternalThreadPool* tp = static_cast<BLInternalThreadPool*>(blThreadPoolGlobal());
  ThreadTestData data;

  constexpr uint32_t kThreadCount = 4;
  BLThread* threads[kThreadCount];

  // Allocating more threads than an internal limit for must always fail.
  INFO("Trying to allocate very high number of threads that should fail");
  {
    BLResult reason;
    uint32_t n = tp->acquireThreads(nullptr, 1000000, BL_THREAD_POOL_ACQUIRE_FLAG_ALL_OR_NOTHING, &reason);

    EXPECT(n == 0);
    EXPECT(reason == BL_ERROR_THREAD_POOL_EXHAUSTED);
  }

  INFO("Repeatedly acquiring / releasing %u threads with a simple task", kThreadCount);
  for (uint32_t i = 0; i < 10; i++) {
    data.iter = i;

    INFO("[#%u] Acquiring %u threads from thread-pool", i, kThreadCount);
    BLResult reason;
    uint32_t n = tp->acquireThreads(threads, kThreadCount, 0, &reason);

    EXPECT(reason == BL_SUCCESS);
    EXPECT(n == kThreadCount);
    EXPECT(tp->createdThreadCount == kThreadCount);

    blAtomicStore(&data.counter, n);
    INFO("[#%u] Running %u threads", i, n);
    for (BLThread* thread : threads) {
      BLResult result = thread->run(test_thread_entry, test_thread_done, &data);
      EXPECT(result == BL_SUCCESS);
    }

    INFO("[#%u] Waiting and releasing", i);
    {
      BLLockGuard<BLMutex> guard(data.mutex);
      data.waiting = true;
      while (blAtomicFetch(&data.counter) != 0)
        data.condition.wait(data.mutex);
    }

    tp->releaseThreads(threads, n);
  }


  INFO("Cleaning up");
  tp->cleanup();

  INFO("Done");
}
#endif
