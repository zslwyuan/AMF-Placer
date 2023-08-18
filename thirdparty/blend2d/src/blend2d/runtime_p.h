// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_RUNTIME_P_H
#define BLEND2D_RUNTIME_P_H

#include "./api-internal_p.h"
#include "./runtime.h"

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

// ============================================================================
// [BLRuntime - FixedFuncArray]
// ============================================================================

//! Fixed array used by handlers, initial content should be zero initialized
//! by the linker as it's used only in a statically allocated `BLRuntimeContext`.
template<typename Func, size_t N>
struct BLFixedFuncArray {
  size_t size;
  Func data[N];

  BL_INLINE void reset() noexcept { size = 0; }

  BL_INLINE void add(Func func) noexcept {
    BL_ASSERT(size < N);
    data[size++] = func;
  }

  template<typename... Args>
  BL_INLINE void call(Args&&... args) noexcept {
    for (size_t i = 0; i < size; i++)
      data[i](std::forward<Args>(args)...);
  }

  template<typename... Args>
  BL_INLINE void callInReverseOrder(Args&&... args) noexcept {
    size_t i = size;
    while (i)
      data[--i](std::forward<Args>(args)...);
  }
};

// ============================================================================
// [BLRuntime - Optimization Info]
// ============================================================================

enum BLRuntimeCpuVendor : uint32_t {
  BL_RUNTIME_CPU_VENDOR_UNKNOWN = 0,
  BL_RUNTIME_CPU_VENDOR_AMD     = 1,
  BL_RUNTIME_CPU_VENDOR_INTEL   = 2,
  BL_RUNTIME_CPU_VENDOR_VIA     = 3
};

enum BLRuntimeCpuHints : uint32_t {
  BL_RUNTIME_CPU_HINT_FAST_AVX256 = 0x00000001u,
  BL_RUNTIME_CPU_HINT_FAST_PSHUFB = 0x00000010u,
  BL_RUNTIME_CPU_HINT_FAST_PMULLD = 0x00000020u
};

struct BLRuntimeOptimizationInfo {
  uint32_t cpuVendor;
  uint32_t cpuHints;

  BL_INLINE bool hasCpuHint(uint32_t hint) const noexcept { return (cpuHints & hint) != 0; }
  BL_INLINE bool hasFastAvx256() const noexcept { return hasCpuHint(BL_RUNTIME_CPU_HINT_FAST_AVX256); }
  BL_INLINE bool hasFastPshufb() const noexcept { return hasCpuHint(BL_RUNTIME_CPU_HINT_FAST_PSHUFB); }
  BL_INLINE bool hasFastPmulld() const noexcept { return hasCpuHint(BL_RUNTIME_CPU_HINT_FAST_PMULLD); }
};

// ============================================================================
// [BLRuntime - Context]
// ============================================================================

//! Blend2D runtime context.
//!
//! A singleton that is created at Blend2D startup and that can be used to query
//! various information about the library and its runtime.
struct BLRuntimeContext {
  //! Shutdown handler.
  typedef void (BL_CDECL* ShutdownFunc)(BLRuntimeContext* rt) BL_NOEXCEPT;
  //! Cleanup handler.
  typedef void (BL_CDECL* CleanupFunc)(BLRuntimeContext* rt, uint32_t cleanupFlags) BL_NOEXCEPT;
  //! MemoryInfo handler.
  typedef void (BL_CDECL* MemoryInfoFunc)(BLRuntimeContext* rt, BLRuntimeMemoryInfo* memoryInfo) BL_NOEXCEPT;

  //! Counts how many times `blRuntimeInit()` has been called.
  //!
  //! Returns the current initialization count, which is incremented every
  //! time a `blRuntimeInit()` is called and decremented every time a
  //! `blRuntimeShutdown()` is called.
  //!
  //! When this counter is incremented from 0 to 1 the library is initialized,
  //! when it's decremented to zero it will free all resources and it will no
  //! longer be safe to use.
  volatile size_t refCount;

  //! System information.
  BLRuntimeSystemInfo systemInfo;

  //! Extended optimization information.
  BLRuntimeOptimizationInfo optimizationInfo;

  // NOTE: There is only a limited number of handlers that can be added to the
  // context. The reason we do it this way is that for builds of Blend2D that
  // have conditionally disabled some features it's easier to have only `RtInit()`
  // handlers and let them register cleanup/shutdown handlers when needed.

  //! Shutdown handlers (always traversed from last to first).
  BLFixedFuncArray<ShutdownFunc, 8> shutdownHandlers;
  //! Cleanup handlers (always executed from first to last).
  BLFixedFuncArray<CleanupFunc, 8> cleanupHandlers;
  //! MemoryInfo handlers (always traversed from first to last).
  BLFixedFuncArray<MemoryInfoFunc, 8> memoryInfoHandlers;
};

//! Instance of a global runtime context.
BL_HIDDEN extern BLRuntimeContext blRuntimeContext;

// ============================================================================
// [BLRuntime - Architecture & Cpu Features]
// ============================================================================

// NOTE: Must be in anonymous namespace. When the compilation unit uses certain
// optimizations we constexpr the check and return `true` without checking CPU
// features as the compilation unit uses them anyway [at that point].
BL_DIAGNOSTIC_PUSH(BL_DIAGNOSTIC_NO_UNUSED_PARAMETERS)

//! Returns true if the target architecture is 32-bit.
static constexpr bool blRuntimeIs32Bit() noexcept { return BL_TARGET_ARCH_BITS < 64; }

namespace {

#ifdef BL_TARGET_OPT_SSE2
constexpr bool blRuntimeHasSSE2(BLRuntimeContext* rt) noexcept { return true; }
#else
inline bool blRuntimeHasSSE2(BLRuntimeContext* rt) noexcept { return (rt->systemInfo.cpuFeatures & BL_RUNTIME_CPU_FEATURE_X86_SSE2) != 0; }
#endif

#ifdef BL_TARGET_OPT_SSE3
constexpr bool blRuntimeHasSSE3(BLRuntimeContext* rt) noexcept { return true; }
#else
inline bool blRuntimeHasSSE3(BLRuntimeContext* rt) noexcept { return (rt->systemInfo.cpuFeatures & BL_RUNTIME_CPU_FEATURE_X86_SSE3) != 0; }
#endif

#ifdef BL_TARGET_OPT_SSSE3
constexpr bool blRuntimeHasSSSE3(BLRuntimeContext* rt) noexcept { return true; }
#else
inline bool blRuntimeHasSSSE3(BLRuntimeContext* rt) noexcept { return (rt->systemInfo.cpuFeatures & BL_RUNTIME_CPU_FEATURE_X86_SSSE3) != 0; }
#endif

#ifdef BL_TARGET_OPT_SSE4_1
constexpr bool blRuntimeHasSSE4_1(BLRuntimeContext* rt) noexcept { return true; }
#else
inline bool blRuntimeHasSSE4_1(BLRuntimeContext* rt) noexcept { return (rt->systemInfo.cpuFeatures & BL_RUNTIME_CPU_FEATURE_X86_SSE4_1) != 0; }
#endif

#ifdef BL_TARGET_OPT_SSE4_2
constexpr bool blRuntimeHasSSE4_2(BLRuntimeContext* rt) noexcept { return true; }
#else
inline bool blRuntimeHasSSE4_2(BLRuntimeContext* rt) noexcept { return (rt->systemInfo.cpuFeatures & BL_RUNTIME_CPU_FEATURE_X86_SSE4_2) != 0; }
#endif

#ifdef BL_TARGET_OPT_AVX
constexpr bool blRuntimeHasAVX(BLRuntimeContext* rt) noexcept { return true; }
#else
inline bool blRuntimeHasAVX(BLRuntimeContext* rt) noexcept { return (rt->systemInfo.cpuFeatures & BL_RUNTIME_CPU_FEATURE_X86_AVX) != 0; }
#endif

#ifdef BL_TARGET_OPT_AVX2
constexpr bool blRuntimeHasAVX2(BLRuntimeContext* rt) noexcept { return true; }
#else
inline bool blRuntimeHasAVX2(BLRuntimeContext* rt) noexcept { return (rt->systemInfo.cpuFeatures & BL_RUNTIME_CPU_FEATURE_X86_AVX2) != 0; }
#endif

} // {anonymous}

BL_DIAGNOSTIC_POP

// ============================================================================
// [BLRuntime - Impl]
// ============================================================================

BL_HIDDEN void BL_CDECL blRuntimeDummyDestroyImplFunc(void* impl, void* destroyData) noexcept;

// ============================================================================
// [BLRuntime - Utilities]
// ============================================================================

BL_HIDDEN BL_NORETURN void blRuntimeFailure(const char* fmt, ...) noexcept;

// ============================================================================
// [BLRuntime - Runtime Init]
// ============================================================================

BL_HIDDEN void blThreadRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blThreadPoolRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blZeroAllocatorRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blMatrix2DRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blArrayRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blStringRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blPathRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blRegionRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blImageRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blImageCodecRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blImageScalerRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blPatternRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blGradientRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blFontRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blFontManagerRtInit(BLRuntimeContext* rt) noexcept;
BL_HIDDEN void blContextRtInit(BLRuntimeContext* rt) noexcept;

#if !defined(BL_BUILD_NO_FIXED_PIPE)
BL_HIDDEN void blFixedPipeRtInit(BLRuntimeContext* rt) noexcept;
#endif

#if !defined(BL_BUILD_NO_JIT)
BL_HIDDEN void blPipeGenRtInit(BLRuntimeContext* rt) noexcept;
#endif

//! \}
//! \endcond

#endif // BLEND2D_RUNTIME_P_H
