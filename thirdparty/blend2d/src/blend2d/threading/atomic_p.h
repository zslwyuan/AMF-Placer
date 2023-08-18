// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_THREADING_ATOMIC_P_H
#define BLEND2D_THREADING_ATOMIC_P_H

#include "../api-internal_p.h"

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

// ============================================================================
// [Atomics - Complements <blapi-impl.h>]
// ============================================================================

static BL_INLINE void blAtomicThreadFence(std::memory_order order = std::memory_order_release) noexcept {
  std::atomic_thread_fence(order);
}

template<typename T>
static BL_INLINE typename std::remove_volatile<T>::type blAtomicFetch(const T* p, std::memory_order order = std::memory_order_relaxed) noexcept {
  typedef typename BLInternal::StdInt<sizeof(T), 0>::Type RawT;
  return (typename std::remove_volatile<T>::type)((const std::atomic<RawT>*)p)->load(order);
}

template<typename T>
static BL_INLINE void blAtomicStore(T* p, typename std::remove_volatile<T>::type value, std::memory_order order = std::memory_order_release) noexcept {
  typedef typename BLInternal::StdInt<sizeof(T), 0>::Type RawT;
  return ((std::atomic<RawT>*)p)->store((RawT)value, order);
}

template<typename T>
static BL_INLINE bool blAtomicCompareExchange(T* ptr, typename std::remove_volatile<T>::type* expected,
                                                      typename std::remove_volatile<T>::type desired) noexcept {
  typedef typename std::remove_volatile<T>::type ValueType;
  return std::atomic_compare_exchange_strong(((std::atomic<ValueType>*)ptr), expected, desired);
}

template<typename T>
static BL_INLINE typename std::remove_volatile<T>::type blAtomicFetchOr(T* x, typename std::remove_volatile<T>::type value, std::memory_order order) noexcept {
  typedef typename std::remove_volatile<T>::type RawT;
  return ((std::atomic<RawT>*)x)->fetch_or(value, order);
}

// ============================================================================
// [BLAtomicUInt64Generator]
// ============================================================================

//! A highly and optimized 64-bit ID generator that can be used to generate IDs
//! of objects to uniquely identify them. The implementation uses atomics that
//! access a global counter and a thread-local variables that access a local
//! counter, which makes it faster than using atomics for each `next()` call.
//!
//! TODO: Use the new implementation that uses TLS instead of this.
struct alignas(BL_CACHE_LINE_SIZE) BLAtomicUInt64Generator {
#if BL_TARGET_HAS_ATOMIC_64B
  std::atomic<uint64_t> _counter;

  BL_INLINE void reset() noexcept { _counter = 0; }
  BL_INLINE uint64_t next() noexcept { return ++_counter; }
#else
  std::atomic<uint32_t> _hi;
  std::atomic<uint32_t> _lo;

  BL_INLINE void reset() noexcept {
    _hi = 0;
    _lo = 0;
  }

  BL_INLINE uint64_t next() noexcept {
    // This implementation doesn't always return an incrementing value as it's
    // not the point. The requirement is to never return the same value, so it
    // sacrifices one bit in `_lo` counter that would tell us to increment `_hi`
    // counter and try again.
    const uint32_t kThresholdLo32 = 0x80000000u;

    for (;;) {
      uint32_t hiValue = _hi.load();
      uint32_t loValue = ++_lo;

      // This MUST support even cases when the thread executing this function
      // right now is terminated. When we reach the threshold we increment
      // `_hi`, which would contain a new HIGH value that will be used
      // immediately, then we remove the threshold mark from LOW value and try
      // to get a new LOW and HIGH values to return.
      if (BL_UNLIKELY(loValue & kThresholdLo32)) {
        _hi++;

        // If the thread is interrupted here we only incremented the HIGH value.
        // In this case another thread that might call `next()` would end up
        // right here trying to clear `kThresholdLo32` from LOW value as well,
        // which is fine.
        _lo.fetch_and(uint32_t(~kThresholdLo32));
        continue;
      }

      return (uint64_t(hiValue) << 32) | loValue;
    }
  }
#endif
};

//! \}
//! \endcond

#endif // BLEND2D_THREADING_ATOMIC_P_H
