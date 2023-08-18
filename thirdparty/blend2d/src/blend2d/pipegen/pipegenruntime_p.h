// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_PIPEGEN_PIPEGENRUNTIME_P_H
#define BLEND2D_PIPEGEN_PIPEGENRUNTIME_P_H

#include "../piperuntime_p.h"
#include "../zoneallocator_p.h"
#include "../zonehash_p.h"
#include "../pipegen/pipegencore_p.h"
#include "../threading/mutex_p.h"

//! \cond INTERNAL
//! \addtogroup blend2d_internal_pipegen
//! \{

// ============================================================================
// [BLPipeFunctionCache]
// ============================================================================

//! PipeGen function cache.
//!
//! \note No locking is preformed implicitly as `BLPipeGenRuntime` synchronizes
//! the access on its own.
class BLPipeFunctionCache {
public:
  struct FuncEntry : public BLZoneHashNode {
    //! Function pointer.
    void* _func;

    BL_INLINE FuncEntry(uint32_t signature, void* func) noexcept
      : BLZoneHashNode(signature) {
      _customData = signature;
      _func = func;
    }

    BL_INLINE void* func() const noexcept { return _func; }
    BL_INLINE uint32_t signature() const noexcept { return _customData; }
  };

  struct FuncMatcher {
    uint32_t _signature;

    BL_INLINE explicit FuncMatcher(uint32_t signature) noexcept
      : _signature(signature) {}

    BL_INLINE uint32_t hashCode() const noexcept { return _signature; }
    BL_INLINE bool matches(const FuncEntry* node) const noexcept { return node->signature() == _signature; }
  };

  BLZoneAllocator _zone;
  BLZoneHashMap<FuncEntry> _funcMap;

  BLPipeFunctionCache() noexcept;
  ~BLPipeFunctionCache() noexcept;

  BL_INLINE void* get(uint32_t signature) const noexcept {
    const FuncEntry* node = _funcMap.get(FuncMatcher(signature));
    return node ? node->func() : nullptr;
  }

  BLResult put(uint32_t signature, void* func) noexcept;
};

// ============================================================================
// [BLPipeGenRuntime]
// ============================================================================

//! PipeGen runtime.
class BLPipeGenRuntime : public BLPipeRuntime {
public:
  //! JIT runtime (stores JIT functions).
  asmjit::JitRuntime _jitRuntime;
  //! Read/write lock used to read/write function cache.
  BLSharedMutex _mutex;
  //! Function cache (caches JIT functions).
  BLPipeFunctionCache _functionCache;
  //! Count of cached pipelines.
  std::atomic<size_t> _pipelineCount;

  //! CPU features to use (either detected or restricted by the user).
  asmjit::BaseFeatures _cpuFeatures;
  //! Maximum pixels at a time, 0 if no limit (debug).
  uint32_t _maxPixels;

  //! Whether to turn on asmjit's logging feature.
  bool _enableLogger;
  //! Whether to emit correct stack frames to make debugging easier. Disabled
  //! by default, because it consumes one GP register, which is always useful.
  bool _emitStackFrames;

  #ifndef ASMJIT_NO_LOGGING
  asmjit::FileLogger _logger;
  #endif

  explicit BLPipeGenRuntime(uint32_t runtimeFlags = 0) noexcept;
  ~BLPipeGenRuntime() noexcept;

  //! Restricts CPU features not provided in the given mask. This function
  //! is only used by isolated runtimes to setup the runtime. It should never
  //! be used after the runtime is in use.
  void _restrictFeatures(uint32_t mask) noexcept;

  BL_INLINE uint32_t maxPixels() const noexcept { return _maxPixels; }
  BL_INLINE void setMaxPixelStep(uint32_t value) noexcept { _maxPixels = value; }

  BLPipeFillFunc _compileFillFunc(uint32_t signature) noexcept;

  static BLWrap<BLPipeGenRuntime> _global;
};

//! \}
//! \endcond

#endif // BLEND2D_PIPEGEN_PIPEGENRUNTIME_P_H
