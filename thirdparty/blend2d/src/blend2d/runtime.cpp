// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include "./api-build_p.h"
#include "./runtime_p.h"
#include "./support_p.h"

// PTHREAD_STACK_MIN would be defined either by <pthread.h> or <limits.h>.
#include <limits.h>

#ifndef BL_BUILD_NO_JIT
  #include <asmjit/asmjit.h>
#endif

// ============================================================================
// [Global Variables]
// ============================================================================

BLRuntimeContext blRuntimeContext;

// ============================================================================
// [BLRuntime - Build Information]
// ============================================================================

#define BL_STRINGIFY_WRAP(N) #N
#define BL_STRINGIFY(N) BL_STRINGIFY_WRAP(N)

static const BLRuntimeBuildInfo blRuntimeBuildInfo = {
  // Library Version.
  { BL_VERSION },

  // Build Type.
#ifdef BL_BUILD_DEBUG
  BL_RUNTIME_BUILD_TYPE_DEBUG,
#else
  BL_RUNTIME_BUILD_TYPE_RELEASE,
#endif

  // Baseline CPU features.
  0
#ifdef BL_TARGET_OPT_SSE2
  | BL_RUNTIME_CPU_FEATURE_X86_SSE2
#endif
#ifdef BL_TARGET_OPT_SSE3
  | BL_RUNTIME_CPU_FEATURE_X86_SSE3
#endif
#ifdef BL_TARGET_OPT_SSSE3
  | BL_RUNTIME_CPU_FEATURE_X86_SSSE3
#endif
#ifdef BL_TARGET_OPT_SSE4_1
  | BL_RUNTIME_CPU_FEATURE_X86_SSE4_1
#endif
#ifdef BL_TARGET_OPT_SSE4_2
  | BL_RUNTIME_CPU_FEATURE_X86_SSE4_2
#endif
#ifdef BL_TARGET_OPT_AVX
  | BL_RUNTIME_CPU_FEATURE_X86_AVX
#endif
#ifdef BL_TARGET_OPT_AVX2
  | BL_RUNTIME_CPU_FEATURE_X86_AVX2
#endif
  ,

  // Supported CPU features.
  0
#ifdef BL_BUILD_OPT_SSE2
  | BL_RUNTIME_CPU_FEATURE_X86_SSE2
#endif
#ifdef BL_BUILD_OPT_SSE3
  | BL_RUNTIME_CPU_FEATURE_X86_SSE3
#endif
#ifdef BL_BUILD_OPT_SSSE3
  | BL_RUNTIME_CPU_FEATURE_X86_SSSE3
#endif
#ifdef BL_BUILD_OPT_SSE4_1
  | BL_RUNTIME_CPU_FEATURE_X86_SSE4_1
#endif
#ifdef BL_BUILD_OPT_SSE4_2
  | BL_RUNTIME_CPU_FEATURE_X86_SSE4_2
#endif
#ifdef BL_BUILD_OPT_AVX
  | BL_RUNTIME_CPU_FEATURE_X86_AVX
#endif
#ifdef BL_BUILD_OPT_AVX2
  | BL_RUNTIME_CPU_FEATURE_X86_AVX2
#endif
  ,

  // Maximum image size.
  BL_RUNTIME_MAX_IMAGE_SIZE,

  // Maximum thread count.
  BL_RUNTIME_MAX_THREAD_COUNT,

  // Reserved
  { 0 },

  // Compiler Info.
#if defined(__INTEL_COMPILER)
  "ICC"
#elif defined(__clang_minor__)
  "Clang " BL_STRINGIFY(__clang_major__) "." BL_STRINGIFY(__clang_minor__)
#elif defined(__GNUC_MINOR__)
  "GCC "  BL_STRINGIFY(__GNUC__) "." BL_STRINGIFY(__GNUC_MINOR__)
#elif defined(_MSC_VER)
  "MSC"
#else
  "Unknown"
#endif
};

// ============================================================================
// [BLRuntime - System Information]
// ============================================================================

#ifndef BL_BUILD_NO_JIT
static BL_INLINE uint32_t blRuntimeDetectCpuFeatures(const asmjit::CpuInfo& asmCpuInfo) noexcept {
  uint32_t features = 0;

#if BL_TARGET_ARCH_X86
  if (asmCpuInfo.hasFeature(asmjit::x86::Features::kSSE2  )) features |= BL_RUNTIME_CPU_FEATURE_X86_SSE2;
  if (asmCpuInfo.hasFeature(asmjit::x86::Features::kSSE3  )) features |= BL_RUNTIME_CPU_FEATURE_X86_SSE3;
  if (asmCpuInfo.hasFeature(asmjit::x86::Features::kSSSE3 )) features |= BL_RUNTIME_CPU_FEATURE_X86_SSSE3;
  if (asmCpuInfo.hasFeature(asmjit::x86::Features::kSSE4_1)) features |= BL_RUNTIME_CPU_FEATURE_X86_SSE4_1;
  if (asmCpuInfo.hasFeature(asmjit::x86::Features::kSSE4_2)) features |= BL_RUNTIME_CPU_FEATURE_X86_SSE4_2;
  if (asmCpuInfo.hasFeature(asmjit::x86::Features::kAVX   )) features |= BL_RUNTIME_CPU_FEATURE_X86_AVX;
  if (asmCpuInfo.hasFeature(asmjit::x86::Features::kAVX2  )) features |= BL_RUNTIME_CPU_FEATURE_X86_AVX2;
#endif

  return features;
}
#endif

static BL_INLINE void blRuntimeInitSystemInfo(BLRuntimeContext* rt) noexcept {
  BLRuntimeSystemInfo& info = rt->systemInfo;

  info.cpuArch = BL_TARGET_ARCH_X86  ? BL_RUNTIME_CPU_ARCH_X86  :
                 BL_TARGET_ARCH_ARM  ? BL_RUNTIME_CPU_ARCH_ARM  :
                 BL_TARGET_ARCH_MIPS ? BL_RUNTIME_CPU_ARCH_MIPS : BL_RUNTIME_CPU_ARCH_UNKNOWN;

#ifndef BL_BUILD_NO_JIT
  const asmjit::CpuInfo& asmCpuInfo = asmjit::CpuInfo::host();
  info.cpuFeatures = blRuntimeDetectCpuFeatures(asmCpuInfo);
  info.coreCount = asmCpuInfo.hwThreadCount();
  info.threadCount = asmCpuInfo.hwThreadCount();
#endif

#ifdef _WIN32
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  info.threadStackSize = si.dwAllocationGranularity;
  info.allocationGranularity = si.dwAllocationGranularity;
#else
  #if defined(_SC_PAGESIZE)
  info.allocationGranularity = uint32_t(sysconf(_SC_PAGESIZE));
  #else
  info.allocationGranularity = uint32_t(getpagesize());
  #endif

  #if defined(PTHREAD_STACK_MIN)
  info.threadStackSize = uint32_t(PTHREAD_STACK_MIN);
  #elif defined(_SC_THREAD_STACK_MIN)
  info.threadStackSize = uint32_t(sysconf(_SC_THREAD_STACK_MIN));
  #else
  #pragma message("Missing 'BLRuntimeSystemInfo::minStackSize' implementation")
  info.threadStackSize = blMax<uint32_t>(info.allocationGranularity, 65536u);
  #endif
#endif

  // NOTE: It seems that on some archs 16kB stack-size is the bare minimum
  // even when sysconf() or PTHREAD_STACK_MIN report a smaller value. Even
  // if we don't need it we slighly increase the bare minimum to 32kB to
  // make it safer especially on archs that has a bit register file.
  info.threadStackSize = blAlignUp(
    blMax<uint32_t>(info.threadStackSize, 32768), info.allocationGranularity);;
}

static BL_INLINE void blRuntimeInitOptimizationInfo(BLRuntimeContext* rt) noexcept {
  BLRuntimeOptimizationInfo& info = rt->optimizationInfo;

#ifndef BL_BUILD_NO_JIT
  const asmjit::CpuInfo& asmCpuInfo = asmjit::CpuInfo::host();

#if BL_TARGET_ARCH_X86
  if (asmCpuInfo.isVendor("AMD")) {
    info.cpuVendor = BL_RUNTIME_CPU_VENDOR_AMD;
    info.cpuHints |= BL_RUNTIME_CPU_HINT_FAST_PSHUFB;
    info.cpuHints |= BL_RUNTIME_CPU_HINT_FAST_PMULLD;
  }
  else if (asmCpuInfo.isVendor("INTEL")) {
    info.cpuVendor = BL_RUNTIME_CPU_VENDOR_INTEL;
    info.cpuHints |= BL_RUNTIME_CPU_HINT_FAST_PSHUFB;
  }
  else if (asmCpuInfo.isVendor("VIA")) {
    info.cpuVendor = BL_RUNTIME_CPU_VENDOR_VIA;
    info.cpuHints |= BL_RUNTIME_CPU_HINT_FAST_PSHUFB;
    info.cpuHints |= BL_RUNTIME_CPU_HINT_FAST_PMULLD;
  }
  else {
    // Assume all other CPUs are okay.
    info.cpuHints |= BL_RUNTIME_CPU_HINT_FAST_PSHUFB;
    info.cpuHints |= BL_RUNTIME_CPU_HINT_FAST_PMULLD;
  }
#endif

#endif
}

// ============================================================================
// [BLRuntime - Initialization & Shutdown]
// ============================================================================

BLResult blRuntimeInit() noexcept {
  BLRuntimeContext* rt = &blRuntimeContext;
  if (blAtomicFetchAdd(&rt->refCount) != 0)
    return BL_SUCCESS;

  // Initialize system information - we need this first so we can properly
  // initialize everything that relies on system info (thread-pool, optimized
  // functions, etc...).
  blRuntimeInitSystemInfo(rt);

  // Initialize optimization information.
  blRuntimeInitOptimizationInfo(rt);

  // Call "Runtime Initialization" handlers.
  // - These would automatically install shutdown handlers when necessary.
  blThreadRtInit(rt);
  blThreadPoolRtInit(rt);
  blZeroAllocatorRtInit(rt);
  blMatrix2DRtInit(rt);
  blArrayRtInit(rt);
  blStringRtInit(rt);
  blPathRtInit(rt);
  blRegionRtInit(rt);
  blImageRtInit(rt);
  blImageCodecRtInit(rt);
  blImageScalerRtInit(rt);
  blPatternRtInit(rt);
  blGradientRtInit(rt);
  blFontRtInit(rt);
  blFontManagerRtInit(rt);

#if !defined(BL_BUILD_NO_FIXED_PIPE)
  blFixedPipeRtInit(rt);
#endif

#if !defined(BL_BUILD_NO_JIT)
  blPipeGenRtInit(rt);
#endif

  blContextRtInit(rt);

  return BL_SUCCESS;
}

BLResult blRuntimeShutdown() noexcept {
  BLRuntimeContext* rt = &blRuntimeContext;
  if (blAtomicFetchSub(&rt->refCount) != 1)
    return BL_SUCCESS;

  rt->shutdownHandlers.callInReverseOrder(rt);
  rt->shutdownHandlers.reset();

  return BL_SUCCESS;
}

// Static instance that calls `blRuntimeInit()` and `blRuntimeShutdown()`.
class BLRuntimeAutoInit {
public:
  inline BLRuntimeAutoInit() noexcept { blRuntimeInit(); }
  inline ~BLRuntimeAutoInit() noexcept { blRuntimeShutdown(); }
};
static BLRuntimeAutoInit blRuntimeAutoInit;

// ============================================================================
// [BLRuntime - Cleanup]
// ============================================================================

BLResult blRuntimeCleanup(uint32_t cleanupFlags) noexcept {
  BLRuntimeContext* rt = &blRuntimeContext;
  rt->cleanupHandlers.call(rt, cleanupFlags);
  return BL_SUCCESS;
}

// ============================================================================
// [BLRuntime - Query Info]
// ============================================================================

BLResult blRuntimeQueryInfo(uint32_t infoType, void* infoOut) noexcept {
  BLRuntimeContext* rt = &blRuntimeContext;

  switch (infoType) {
    case BL_RUNTIME_INFO_TYPE_BUILD: {
      BLRuntimeBuildInfo* buildInfo = static_cast<BLRuntimeBuildInfo*>(infoOut);
      memcpy(buildInfo, &blRuntimeBuildInfo, sizeof(BLRuntimeBuildInfo));
      return BL_SUCCESS;
    }

    case BL_RUNTIME_INFO_TYPE_SYSTEM: {
      BLRuntimeSystemInfo* systemInfo = static_cast<BLRuntimeSystemInfo*>(infoOut);
      memcpy(systemInfo, &rt->systemInfo, sizeof(BLRuntimeSystemInfo));
      return BL_SUCCESS;
    }

    case BL_RUNTIME_INFO_TYPE_MEMORY: {
      BLRuntimeMemoryInfo* memoryInfo = static_cast<BLRuntimeMemoryInfo*>(infoOut);
      memoryInfo->reset();
      rt->memoryInfoHandlers.call(rt, memoryInfo);
      return BL_SUCCESS;
    }

    default:
      return blTraceError(BL_ERROR_INVALID_VALUE);
  }
}

// ============================================================================
// [BLRuntime - Message]
// ============================================================================

BLResult blRuntimeMessageOut(const char* msg) noexcept {
#if defined(_WIN32)
  // Support both Console and GUI applications on Windows.
  OutputDebugStringA(msg);
#endif

  fputs(msg, stderr);
  return BL_SUCCESS;
}

BLResult blRuntimeMessageFmt(const char* fmt, ...) noexcept {
  va_list ap;
  va_start(ap, fmt);
  BLResult result = blRuntimeMessageVFmt(fmt, ap);
  va_end(ap);

  return result;
}

BLResult blRuntimeMessageVFmt(const char* fmt, va_list ap) noexcept {
  char buf[1024];
  vsnprintf(buf, BL_ARRAY_SIZE(buf), fmt, ap);
  return blRuntimeMessageOut(buf);
}

// ============================================================================
// [BLRuntime - Failure]
// ============================================================================

void blRuntimeFailure(const char* fmt, ...) noexcept {
  va_list ap;
  va_start(ap, fmt);
  blRuntimeMessageVFmt(fmt, ap);
  va_end(ap);

  abort();
}

void blRuntimeAssertionFailure(const char* file, int line, const char* msg) noexcept {
  blRuntimeMessageFmt("[Blend2D] ASSERTION FAILURE: '%s' at '%s' [line %d]\n", msg, file, line);
  abort();
}

// ============================================================================
// [BLRuntime - Alloc / Free]
// ============================================================================

// We use 'malloc/free' implementation at the moment.

void* blRuntimeAllocImpl(size_t implSize, uint16_t* memPoolDataOut) noexcept {
  *memPoolDataOut = 0;
  return malloc(implSize);
}

void* blRuntimeAllocAlignedImpl(size_t implSize, size_t alignment, uint16_t* memPoolDataOut) noexcept {
  *memPoolDataOut = 0;

  if (alignment <= BL_ALLOC_ALIGNMENT)
    return malloc(implSize);

  BL_ASSERT(blIsPowerOf2(alignment));
  void* ptr = malloc(implSize + alignment - BL_ALLOC_ALIGNMENT);

  if (!ptr)
    return nullptr;

  void* alignedPtr = blAlignUp(ptr, alignment);
  *memPoolDataOut = uint16_t(uintptr_t(alignedPtr) - uintptr_t(ptr));
  return alignedPtr;
}

BLResult blRuntimeFreeImpl(void* impl_, size_t implSize, uint32_t memPoolData) noexcept {
  BL_UNUSED(implSize);

  void* unalignedPtr = static_cast<void*>(static_cast<uint8_t*>(impl_) - memPoolData);
  free(unalignedPtr);
  return BL_SUCCESS;
}

void BL_CDECL blRuntimeDummyDestroyImplFunc(void* impl, void* destroyData) noexcept {
  BL_UNUSED(impl);
  BL_UNUSED(destroyData);
}

// ============================================================================
// [BLRuntime - ResultFrom{Win|Posix}Error]
// ============================================================================

#ifdef _WIN32

// Fix possible problems with MinGW not defining these.
#ifndef ERROR_DISK_QUOTA_EXCEEDED
  #define ERROR_DISK_QUOTA_EXCEEDED 0x0000050F
#endif

BLResult blResultFromWinError(uint32_t e) noexcept {
  switch (e) {
    case ERROR_SUCCESS                : return BL_SUCCESS;                       // 0x00000000
    case ERROR_INVALID_FUNCTION       : return BL_ERROR_NOT_PERMITTED;           // 0x00000001
    case ERROR_FILE_NOT_FOUND         : return BL_ERROR_NO_ENTRY;                // 0x00000002
    case ERROR_PATH_NOT_FOUND         : return BL_ERROR_NO_ENTRY;                // 0x00000003
    case ERROR_TOO_MANY_OPEN_FILES    : return BL_ERROR_TOO_MANY_OPEN_FILES;     // 0x00000004
    case ERROR_ACCESS_DENIED          : return BL_ERROR_ACCESS_DENIED;           // 0x00000005
    case ERROR_INVALID_HANDLE         : return BL_ERROR_INVALID_HANDLE;          // 0x00000006
    case ERROR_NOT_ENOUGH_MEMORY      : return BL_ERROR_OUT_OF_MEMORY;           // 0x00000008
    case ERROR_OUTOFMEMORY            : return BL_ERROR_OUT_OF_MEMORY;           // 0x0000000E
    case ERROR_INVALID_DRIVE          : return BL_ERROR_NO_ENTRY;                // 0x0000000F
    case ERROR_CURRENT_DIRECTORY      : return BL_ERROR_NOT_PERMITTED;           // 0x00000010
    case ERROR_NOT_SAME_DEVICE        : return BL_ERROR_NOT_SAME_DEVICE;         // 0x00000011
    case ERROR_NO_MORE_FILES          : return BL_ERROR_NO_MORE_FILES;           // 0x00000012
    case ERROR_WRITE_PROTECT          : return BL_ERROR_READ_ONLY_FS;            // 0x00000013
    case ERROR_NOT_READY              : return BL_ERROR_NO_MEDIA;                // 0x00000015
    case ERROR_CRC                    : return BL_ERROR_IO;                      // 0x00000017
    case ERROR_SEEK                   : return BL_ERROR_INVALID_SEEK;            // 0x00000019
    case ERROR_WRITE_FAULT            : return BL_ERROR_IO;                      // 0x0000001D
    case ERROR_READ_FAULT             : return BL_ERROR_IO;                      // 0x0000001E
    case ERROR_GEN_FAILURE            : return BL_ERROR_IO;                      // 0x0000001F
    case ERROR_SHARING_BUFFER_EXCEEDED: return BL_ERROR_TOO_MANY_OPEN_FILES;     // 0x00000024
    case ERROR_HANDLE_EOF             : return BL_ERROR_NO_MORE_DATA;            // 0x00000026
    case ERROR_HANDLE_DISK_FULL       : return BL_ERROR_NO_SPACE_LEFT;           // 0x00000027
    case ERROR_NOT_SUPPORTED          : return BL_ERROR_NOT_IMPLEMENTED;         // 0x00000032
    case ERROR_FILE_EXISTS            : return BL_ERROR_ALREADY_EXISTS;          // 0x00000050
    case ERROR_CANNOT_MAKE            : return BL_ERROR_NOT_PERMITTED;           // 0x00000052
    case ERROR_INVALID_PARAMETER      : return BL_ERROR_INVALID_VALUE;           // 0x00000057
    case ERROR_NET_WRITE_FAULT        : return BL_ERROR_IO;                      // 0x00000058
    case ERROR_DRIVE_LOCKED           : return BL_ERROR_BUSY;                    // 0x0000006C
    case ERROR_BROKEN_PIPE            : return BL_ERROR_BROKEN_PIPE;             // 0x0000006D
    case ERROR_OPEN_FAILED            : return BL_ERROR_OPEN_FAILED;             // 0x0000006E
    case ERROR_BUFFER_OVERFLOW        : return BL_ERROR_FILE_NAME_TOO_LONG;      // 0x0000006F
    case ERROR_DISK_FULL              : return BL_ERROR_NO_SPACE_LEFT;           // 0x00000070
    case ERROR_CALL_NOT_IMPLEMENTED   : return BL_ERROR_NOT_IMPLEMENTED;         // 0x00000078
    case ERROR_INVALID_NAME           : return BL_ERROR_INVALID_FILE_NAME;       // 0x0000007B
    case ERROR_NEGATIVE_SEEK          : return BL_ERROR_INVALID_SEEK;            // 0x00000083
    case ERROR_SEEK_ON_DEVICE         : return BL_ERROR_INVALID_SEEK;            // 0x00000084
    case ERROR_BUSY_DRIVE             : return BL_ERROR_BUSY;                    // 0x0000008E
    case ERROR_DIR_NOT_ROOT           : return BL_ERROR_NOT_ROOT_DEVICE;         // 0x00000090
    case ERROR_DIR_NOT_EMPTY          : return BL_ERROR_NOT_EMPTY;               // 0x00000091
    case ERROR_PATH_BUSY              : return BL_ERROR_BUSY;                    // 0x00000094
    case ERROR_TOO_MANY_TCBS          : return BL_ERROR_TOO_MANY_THREADS;        // 0x0000009B
    case ERROR_BAD_ARGUMENTS          : return BL_ERROR_INVALID_VALUE;           // 0x000000A0
    case ERROR_BAD_PATHNAME           : return BL_ERROR_INVALID_FILE_NAME;       // 0x000000A1
    case ERROR_SIGNAL_PENDING         : return BL_ERROR_BUSY;                    // 0x000000A2
    case ERROR_MAX_THRDS_REACHED      : return BL_ERROR_TOO_MANY_THREADS;        // 0x000000A4
    case ERROR_BUSY                   : return BL_ERROR_BUSY;                    // 0x000000AA
    case ERROR_ALREADY_EXISTS         : return BL_ERROR_ALREADY_EXISTS;          // 0x000000B7
    case ERROR_BAD_PIPE               : return BL_ERROR_BROKEN_PIPE;             // 0x000000E6
    case ERROR_PIPE_BUSY              : return BL_ERROR_BUSY;                    // 0x000000E7
    case ERROR_NO_MORE_ITEMS          : return BL_ERROR_NO_MORE_FILES;           // 0x00000103
    case ERROR_FILE_INVALID           : return BL_ERROR_NO_ENTRY;                // 0x000003EE
    case ERROR_NO_DATA_DETECTED       : return BL_ERROR_IO;                      // 0x00000450
    case ERROR_MEDIA_CHANGED          : return BL_ERROR_MEDIA_CHANGED;           // 0x00000456
    case ERROR_IO_DEVICE              : return BL_ERROR_NO_DEVICE;               // 0x0000045D
    case ERROR_NO_MEDIA_IN_DRIVE      : return BL_ERROR_NO_MEDIA;                // 0x00000458
    case ERROR_DISK_OPERATION_FAILED  : return BL_ERROR_IO;                      // 0x00000467
    case ERROR_TOO_MANY_LINKS         : return BL_ERROR_TOO_MANY_LINKS;          // 0x00000476
    case ERROR_DISK_QUOTA_EXCEEDED    : return BL_ERROR_NO_SPACE_LEFT;           // 0x0000050F
    case ERROR_INVALID_USER_BUFFER    : return BL_ERROR_BUSY;                    // 0x000006F8
    case ERROR_UNRECOGNIZED_MEDIA     : return BL_ERROR_IO;                      // 0x000006F9
    case ERROR_NOT_ENOUGH_QUOTA       : return BL_ERROR_OUT_OF_MEMORY;           // 0x00000718
    case ERROR_CANT_ACCESS_FILE       : return BL_ERROR_NOT_PERMITTED;           // 0x00000780
    case ERROR_CANT_RESOLVE_FILENAME  : return BL_ERROR_NO_ENTRY;                // 0x00000781
    case ERROR_OPEN_FILES             : return BL_ERROR_TRY_AGAIN;               // 0x00000961
  }

  // Pass the system error if it's below our error indexing.
  if (e < BL_ERROR_START_INDEX)
    return e;

  // Otherwise this is an unmapped system error code.
  return BL_ERROR_UNKNOWN_SYSTEM_ERROR;
}

#else

BLResult blResultFromPosixError(int e) noexcept {
  #define MAP(C_ERROR, BL_ERROR) case C_ERROR: return BL_ERROR

  switch (e) {
  #ifdef EACCES
    MAP(EACCES, BL_ERROR_ACCESS_DENIED);
  #endif
  #ifdef EAGAIN
    MAP(EAGAIN, BL_ERROR_TRY_AGAIN);
  #endif
  #ifdef EBADF
    MAP(EBADF, BL_ERROR_INVALID_HANDLE);
  #endif
  #ifdef EBUSY
    MAP(EBUSY, BL_ERROR_BUSY);
  #endif
  #ifdef EDQUOT
    MAP(EDQUOT, BL_ERROR_NO_SPACE_LEFT);
  #endif
  #ifdef EEXIST
    MAP(EEXIST, BL_ERROR_ALREADY_EXISTS);
  #endif
  #ifdef EFAULT
    MAP(EFAULT, BL_ERROR_INVALID_STATE);
  #endif
  #ifdef EFBIG
    MAP(EFBIG, BL_ERROR_FILE_TOO_LARGE);
  #endif
  #ifdef EINTR
    MAP(EINTR, BL_ERROR_INTERRUPTED);
  #endif
  #ifdef EINVAL
    MAP(EINVAL, BL_ERROR_INVALID_VALUE);
  #endif
  #ifdef EIO
    MAP(EIO, BL_ERROR_IO);
  #endif
  #ifdef EISDIR
    MAP(EISDIR, BL_ERROR_NOT_FILE);
  #endif
  #ifdef ELOOP
    MAP(ELOOP, BL_ERROR_SYMLINK_LOOP);
  #endif
  #ifdef EMFILE
    MAP(EMFILE, BL_ERROR_TOO_MANY_OPEN_FILES);
  #endif
  #ifdef ENAMETOOLONG
    MAP(ENAMETOOLONG, BL_ERROR_FILE_NAME_TOO_LONG);
  #endif
  #ifdef ENFILE
    MAP(ENFILE, BL_ERROR_TOO_MANY_OPEN_FILES_BY_OS);
  #endif
  #ifdef ENMFILE
    MAP(ENMFILE, BL_ERROR_NO_MORE_FILES);
  #endif
  #ifdef ENODATA
    MAP(ENODATA, BL_ERROR_NO_MORE_DATA);
  #endif
  #ifdef ENODEV
    MAP(ENODEV, BL_ERROR_NO_DEVICE);
  #endif
  #ifdef ENOENT
    MAP(ENOENT, BL_ERROR_NO_ENTRY);
  #endif
  #ifdef ENOMEDIUM
    MAP(ENOMEDIUM, BL_ERROR_NO_MEDIA);
  #endif
  #ifdef ENOMEM
    MAP(ENOMEM, BL_ERROR_OUT_OF_MEMORY);
  #endif
  #ifdef ENOSPC
    MAP(ENOSPC, BL_ERROR_NO_SPACE_LEFT);
  #endif
  #ifdef ENOSYS
    MAP(ENOSYS, BL_ERROR_NOT_IMPLEMENTED);
  #endif
  #ifdef ENOTBLK
    MAP(ENOTBLK, BL_ERROR_NOT_BLOCK_DEVICE);
  #endif
  #ifdef ENOTDIR
    MAP(ENOTDIR, BL_ERROR_NOT_DIRECTORY);
  #endif
  #ifdef ENOTEMPTY
    MAP(ENOTEMPTY, BL_ERROR_NOT_EMPTY);
  #endif
  #ifdef ENXIO
    MAP(ENXIO, BL_ERROR_NO_DEVICE);
  #endif
  #ifdef EOVERFLOW
    MAP(EOVERFLOW, BL_ERROR_VALUE_TOO_LARGE);
  #endif
  #ifdef EPERM
    MAP(EPERM, BL_ERROR_NOT_PERMITTED);
  #endif
  #ifdef EROFS
    MAP(EROFS, BL_ERROR_READ_ONLY_FS);
  #endif
  #ifdef ESPIPE
    MAP(ESPIPE, BL_ERROR_INVALID_SEEK);
  #endif
  #ifdef ETIMEDOUT
    MAP(ETIMEDOUT, BL_ERROR_TIMED_OUT);
  #endif
  #ifdef EXDEV
    MAP(EXDEV, BL_ERROR_NOT_SAME_DEVICE);
  #endif
  }

  #undef MAP

  // Pass the system error if it's below our error indexing.
  if (e != 0 && unsigned(e) < BL_ERROR_START_INDEX)
    return uint32_t(unsigned(e));
  else
    return BL_ERROR_UNKNOWN_SYSTEM_ERROR;
}
#endif
