// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_CONTEXT_H
#define BLEND2D_CONTEXT_H

#include "./font.h"
#include "./geometry.h"
#include "./image.h"
#include "./matrix.h"
#include "./path.h"
#include "./rgba.h"
#include "./region.h"
#include "./style.h"
#include "./variant.h"

//! \addtogroup blend2d_api_rendering
//! \{

// ============================================================================
// [Constants]
// ============================================================================

//! Rendering context type.
BL_DEFINE_ENUM(BLContextType) {
  //! No rendering context.
  BL_CONTEXT_TYPE_NONE = 0,
  //! Dummy rendering context.
  BL_CONTEXT_TYPE_DUMMY = 1,

  /*
  //! Proxy rendering context.
  BL_CONTEXT_TYPE_PROXY = 2,
  */

  //! Software-accelerated rendering context.
  BL_CONTEXT_TYPE_RASTER = 3,

  //! Count of rendering context types.
  BL_CONTEXT_TYPE_COUNT = 4
};

//! Rendering context hint.
BL_DEFINE_ENUM(BLContextHint) {
  //! Rendering quality.
  BL_CONTEXT_HINT_RENDERING_QUALITY = 0,
  //! Gradient quality.
  BL_CONTEXT_HINT_GRADIENT_QUALITY = 1,
  //! Pattern quality.
  BL_CONTEXT_HINT_PATTERN_QUALITY = 2,

  //! Count of rendering context hints.
  BL_CONTEXT_HINT_COUNT = 8
};

//! Describes a rendering operation type - fill or stroke.
//!
//! The rendering context allows to get and set fill & stroke options directly
//! or via "style" functions that take the rendering operation type (`opType`)
//! and dispatch the call to the right function.
BL_DEFINE_ENUM(BLContextOpType) {
  //! Fill operation type.
  BL_CONTEXT_OP_TYPE_FILL = 0,
  //! Stroke operation type.
  BL_CONTEXT_OP_TYPE_STROKE = 1,

  //! Count of rendering operations.
  BL_CONTEXT_OP_TYPE_COUNT = 2
};

//! Rendering context flush-flags, use with `BLContext::flush()`.
BL_DEFINE_ENUM(BLContextFlushFlags) {
  //! Flush the command queue and wait for its completion (will block).
  BL_CONTEXT_FLUSH_SYNC = 0x80000000u
};

//! Rendering context create-flags.
BL_DEFINE_ENUM(BLContextCreateFlags) {
  //! Fallbacks to a synchronous rendering in case that the rendering engine
  //! wasn't able to acquire threads. This flag only makes sense when the
  //! asynchronous mode was specified by having `threadCount` greater than 0.
  //! If the rendering context fails to acquire at least one thread it would
  //! fallback to synchronous mode with no worker threads.
  //!
  //! \note If this flag is specified with `threadCount == 1` it means to
  //! immedialy fallback to synchronous rendering. It's only practical to
  //! use this flag with 2 or more requested threads.
  BL_CONTEXT_CREATE_FLAG_FALLBACK_TO_SYNC = 0x00000008u,

  //! If this flag is specified and asynchronous rendering is enabled then
  //! the context would create its own isolated thread-pool, which is useful
  //! for debugging purposes.
  //!
  //! Do not use this flag in production as rendering contexts with isolated
  //! thread-pool have to create and destroy all threads they use. This flag
  //! is only useful for testing, debugging, and isolated benchmarking.
  BL_CONTEXT_CREATE_FLAG_ISOLATED_THREAD_POOL = 0x01000000u,

  //! If this flag is specified and JIT pipeline generation enabled then the
  //! rendering context would create its own isolated JIT runtime. which is
  //! useful for debugging purposes. This flag will be ignored if JIT pipeline
  //! generation is either not supported or was disabled by other flags.
  //!
  //! Do not use this flag in production as rendering contexts with isolated
  //! JIT runtime do not use global pipeline cache, that's it, after the
  //! rendering context is destroyed the JIT runtime is destroyed with it with
  //! all compiled pipelines. This flag is only useful for testing, debugging,
  //! and isolated benchmarking.
  BL_CONTEXT_CREATE_FLAG_ISOLATED_JIT = 0x02000000u,

  //! Override CPU features when creating isolated context.
  BL_CONTEXT_CREATE_FLAG_OVERRIDE_CPU_FEATURES = 0x04000000u
};

//! Specifies a rendering context property that can be specific to the rendering
//! context implementation and that doesn't have its own C and C++ API. Different
//! rendering context implementations may expose various properties that users
//! can query to get more details about the rendering context itself, rendering
//! details (like optimizations or possibly limitations), memory details, and
//! other information that was collected during the rendering.
//!
//! Properties are never part of the rendering context state - they are stateless
//! and are not subject to `save()` and `restore()`. Many properties are purely
//! informative, but some not, e.g. `BL_CONTEXT_PROPERTY_ACCUMULATED_ERROR_FLAGS`.
BL_DEFINE_ENUM(BLContextProperty) {
  //! Number of threads that the rendering context uses for rendering.
  BL_CONTEXT_PROPERTY_THREAD_COUNT = 0,

  //! Accumulated errors collected during the lifetime of the rendering context.
  BL_CONTEXT_PROPERTY_ACCUMULATED_ERROR_FLAGS = 10
};

//! Error flags that are accumulated during the rendering context lifetime and
//! that can be queried through `BLContext::queryAccumulatedErrorFlags()`. The
//! reason why these flags exist is that errors can happen during asynchronous
//! rendering, and there is no way the user can catch these errors.
BL_DEFINE_ENUM(BLContextErrorFlags) {
  //! The rendering context returned or encountered `BL_ERROR_INVALID_VALUE`,
  //! which is mostly related to function argument handling. It's very likely
  //! some argument was wrong when calling `BLContext` API.
  BL_CONTEXT_ERROR_FLAG_INVALID_VALUE = 0x00000001u,

  //! Invalid state describes something wrong, for example a pipeline compilation
  //! error.
  BL_CONTEXT_ERROR_FLAG_INVALID_STATE = 0x00000002u,

  //! The rendering context has encountered invalid geometry.
  BL_CONTEXT_ERROR_FLAG_INVALID_GEOMETRY = 0x00000004u,

  //! The rendering context has encountered invalid glyph.
  BL_CONTEXT_ERROR_FLAG_INVALID_GLYPH = 0x00000008u,

  //! The rendering context has encountered invalid or uninitialized font.
  BL_CONTEXT_ERROR_FLAG_INVALID_FONT = 0x00000010u,

  //! Thread pool was exhausted and couldn't acquire the requested number of threads.
  BL_CONTEXT_ERROR_FLAG_THREAD_POOL_EXHAUSTED = 0x20000000u,

  //! Out of memory condition.
  BL_CONTEXT_ERROR_FLAG_OUT_OF_MEMORY = 0x40000000u,

  //! Unknown error, which we don't have flag for.
  BL_CONTEXT_ERROR_FLAG_UNKNOWN_ERROR = 0x80000000u
};

//! Clip mode.
BL_DEFINE_ENUM(BLClipMode) {
  //! Clipping to a rectangle that is aligned to the pixel grid.
  BL_CLIP_MODE_ALIGNED_RECT = 0,
  //! Clipping to a rectangle that is not aligned to pixel grid.
  BL_CLIP_MODE_UNALIGNED_RECT = 1,
  //! Clipping to a non-rectangular area that is defined by using mask.
  BL_CLIP_MODE_MASK = 2,

  //! Count of clip modes.
  BL_CLIP_MODE_COUNT = 3
};

//! Composition & blending operator.
BL_DEFINE_ENUM(BLCompOp) {
  //! Source-over [default].
  BL_COMP_OP_SRC_OVER = 0,
  //! Source-copy.
  BL_COMP_OP_SRC_COPY = 1,
  //! Source-in.
  BL_COMP_OP_SRC_IN = 2,
  //! Source-out.
  BL_COMP_OP_SRC_OUT = 3,
  //! Source-atop.
  BL_COMP_OP_SRC_ATOP = 4,
  //! Destination-over.
  BL_COMP_OP_DST_OVER = 5,
  //! Destination-copy [nop].
  BL_COMP_OP_DST_COPY = 6,
  //! Destination-in.
  BL_COMP_OP_DST_IN = 7,
  //! Destination-out.
  BL_COMP_OP_DST_OUT = 8,
  //! Destination-atop.
  BL_COMP_OP_DST_ATOP = 9,
  //! Xor.
  BL_COMP_OP_XOR = 10,
  //! Clear.
  BL_COMP_OP_CLEAR = 11,
  //! Plus.
  BL_COMP_OP_PLUS = 12,
  //! Minus.
  BL_COMP_OP_MINUS = 13,
  //! Modulate.
  BL_COMP_OP_MODULATE = 14,
  //! Multiply.
  BL_COMP_OP_MULTIPLY = 15,
  //! Screen.
  BL_COMP_OP_SCREEN = 16,
  //! Overlay.
  BL_COMP_OP_OVERLAY = 17,
  //! Darken.
  BL_COMP_OP_DARKEN = 18,
  //! Lighten.
  BL_COMP_OP_LIGHTEN = 19,
  //! Color dodge.
  BL_COMP_OP_COLOR_DODGE = 20,
  //! Color burn.
  BL_COMP_OP_COLOR_BURN = 21,
  //! Linear burn.
  BL_COMP_OP_LINEAR_BURN = 22,
  //! Linear light.
  BL_COMP_OP_LINEAR_LIGHT = 23,
  //! Pin light.
  BL_COMP_OP_PIN_LIGHT = 24,
  //! Hard-light.
  BL_COMP_OP_HARD_LIGHT = 25,
  //! Soft-light.
  BL_COMP_OP_SOFT_LIGHT = 26,
  //! Difference.
  BL_COMP_OP_DIFFERENCE = 27,
  //! Exclusion.
  BL_COMP_OP_EXCLUSION = 28,

  //! Count of composition & blending operators.
  BL_COMP_OP_COUNT = 29
};

//! Gradient rendering quality.
BL_DEFINE_ENUM(BLGradientQuality) {
  //! Nearest neighbor.
  BL_GRADIENT_QUALITY_NEAREST = 0,

  //! Count of gradient quality options.
  BL_GRADIENT_QUALITY_COUNT = 1
};

//! Pattern quality.
BL_DEFINE_ENUM(BLPatternQuality) {
  //! Nearest neighbor.
  BL_PATTERN_QUALITY_NEAREST = 0,
  //! Bilinear.
  BL_PATTERN_QUALITY_BILINEAR = 1,

  //! Count of pattern quality options.
  BL_PATTERN_QUALITY_COUNT = 2
};

//! Rendering quality.
BL_DEFINE_ENUM(BLRenderingQuality) {
  //! Render using anti-aliasing.
  BL_RENDERING_QUALITY_ANTIALIAS = 0,

  //! Count of rendering quality options.
  BL_RENDERING_QUALITY_COUNT = 1
};

// ============================================================================
// [BLContext - CreateInfo]
// ============================================================================

//! Information that can be used to customize the rendering context.
struct BLContextCreateInfo {
  //! Create flags, see `BLContextCreateFlags`.
  uint32_t flags;

  //! Number of worker threads to use for asynchronous rendering, if non-zero.
  //!
  //! If `threadCount` is zero it means to initialize the context for synchronous
  //! rendering. This means that every operation will take effect immediately.
  //! If `threadCount` is `1` it means that the rendering will be asynchronous,
  //! but no thread would be acquired from a thread-pool, because the user thread
  //! will be used as a worker. And finally, if `threadCount` is greater than `1`
  //! then total of `threadCount - 1` threads will be acquired from thread-pool
  //! and used as additional workers.
  uint32_t threadCount;

  //! CPU features to use in isolated JIT runtime (if supported), only used
  //! when `flags` contains `BL_CONTEXT_CREATE_FLAG_OVERRIDE_CPU_FEATURES`.
  uint32_t cpuFeatures;

  //! Maximum number of commands to be queued.
  //!
  //! If this parameter is zero the queue size will be determined automatically.
  //!
  //! TODO: To be documented, has no effect at the moment.
  uint32_t commandQueueLimit;

  //! Reserved for future use, must be zero.
  uint32_t reserved[4];

  // --------------------------------------------------------------------------
  #ifdef __cplusplus
  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
  #endif
  // --------------------------------------------------------------------------
};

// ============================================================================
// [BLContext - Cookie]
// ============================================================================

//! Holds an arbitrary 128-bit value (cookie) that can be used to match other
//! cookies. Blend2D uses cookies in places where it allows to "lock" some
//! state that can only be unlocked by a matching cookie. Please don't confuse
//! cookies with a security of any kind, it's just an arbitrary data that must
//! match to proceed with a certain operation.
//!
//! Cookies can be used with `BLContext::save()` and `BLContext::restore()`
//! operations.
struct BLContextCookie {
  uint64_t data[2];

  // --------------------------------------------------------------------------
  #ifdef __cplusplus

  BL_INLINE bool operator==(const BLContextCookie& other) const noexcept { return  equals(other); }
  BL_INLINE bool operator!=(const BLContextCookie& other) const noexcept { return !equals(other); }

  BL_INLINE bool empty() const noexcept {
    return this->data[0] == 0 && this->data[1] == 0;
  }

  BL_INLINE void reset() noexcept { reset(0, 0); }
  BL_INLINE void reset(const BLContextCookie& other) noexcept { reset(other.data[0], other.data[1]); }

  BL_INLINE void reset(uint64_t data0, uint64_t data1) noexcept {
    this->data[0] = data0;
    this->data[1] = data1;
  }

  BL_INLINE bool equals(const BLContextCookie& other) const noexcept {
    return blEquals(this->data[0], other.data[0]) &
           blEquals(this->data[1], other.data[1]);
  }

  #endif
  // --------------------------------------------------------------------------
};

// ============================================================================
// [BLContext - Hints]
// ============================================================================

//! Rendering context hints.
struct BLContextHints {
  union {
    struct {
      uint8_t renderingQuality;
      uint8_t gradientQuality;
      uint8_t patternQuality;
    };

    uint8_t hints[BL_CONTEXT_HINT_COUNT];
  };

  // --------------------------------------------------------------------------
  #ifdef __cplusplus

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }

  #endif
  // --------------------------------------------------------------------------
};

// ============================================================================
// [BLContext - State]
// ============================================================================

//! Rendering context state.
//!
//! This state is not meant to be created by users, it's only provided for users
//! that want to introspect the rendering context state and for C++ API that can
//! access it directly for performance reasons.
struct BLContextState {
  //! Target image or image object with nullptr impl in case that the rendering
  //! context doesn't render to an image.
  BLImageCore* targetImage;
  //! Current size of the target in abstract units, pixels if rendering to `BLImage`.
  BLSize targetSize;

  //! Current context hints.
  BLContextHints hints;
  //! Current composition operator.
  uint8_t compOp;
  //! Current fill rule.
  uint8_t fillRule;
  //! Current type of a style for fill and stroke operations, see `BLContextOpType`
  //! that describes indexes and `BLStyleType` that describes styles.
  uint8_t styleType[2];
  //! Reserved for future use, must be zero.
  uint8_t reserved[4];

  //! Approximation options.
  BLApproximationOptions approximationOptions;

  //! Current global alpha value [0, 1].
  double globalAlpha;
  //! Current fill or stroke alpha, see `BLContextOpType`.
  double styleAlpha[2];

  //! Current stroke options.
  BL_TYPED_MEMBER(BLStrokeOptionsCore, BLStrokeOptions, strokeOptions);

  //! Current meta transformation matrix.
  BLMatrix2D metaMatrix;
  //! Current user transformation matrix.
  BLMatrix2D userMatrix;

  //! Count of saved states in the context.
  size_t savedStateCount;

  BL_HAS_TYPED_MEMBERS(BLContextState)
};

// ============================================================================
// [BLContext - Core]
// ============================================================================

//! Rendering context [C Interface - Virtual Function Table].
struct BLContextVirt {
  BLResult (BL_CDECL* destroy                )(BLContextImpl* impl) BL_NOEXCEPT;
  BLResult (BL_CDECL* flush                  )(BLContextImpl* impl, uint32_t flags) BL_NOEXCEPT;

  BLResult (BL_CDECL* queryProperty          )(const BLContextImpl* impl, uint32_t propertyId, void* valueOut) BL_NOEXCEPT;

  BLResult (BL_CDECL* save                   )(BLContextImpl* impl, BLContextCookie* cookie) BL_NOEXCEPT;
  BLResult (BL_CDECL* restore                )(BLContextImpl* impl, const BLContextCookie* cookie) BL_NOEXCEPT;

  BLResult (BL_CDECL* matrixOp               )(BLContextImpl* impl, uint32_t opType, const void* opData) BL_NOEXCEPT;
  BLResult (BL_CDECL* userToMeta             )(BLContextImpl* impl) BL_NOEXCEPT;

  BLResult (BL_CDECL* setHint                )(BLContextImpl* impl, uint32_t hintType, uint32_t value) BL_NOEXCEPT;
  BLResult (BL_CDECL* setHints               )(BLContextImpl* impl, const BLContextHints* hints) BL_NOEXCEPT;
  BLResult (BL_CDECL* setFlattenMode         )(BLContextImpl* impl, uint32_t mode) BL_NOEXCEPT;
  BLResult (BL_CDECL* setFlattenTolerance    )(BLContextImpl* impl, double tolerance) BL_NOEXCEPT;
  BLResult (BL_CDECL* setApproximationOptions)(BLContextImpl* impl, const BLApproximationOptions* options) BL_NOEXCEPT;

  BLResult (BL_CDECL* setCompOp              )(BLContextImpl* impl, uint32_t compOp) BL_NOEXCEPT;
  BLResult (BL_CDECL* setGlobalAlpha         )(BLContextImpl* impl, double alpha) BL_NOEXCEPT;

  // Allows to dispatch fill/stroke by `BLContextOpType`.
  BLResult (BL_CDECL* setStyleAlpha[2]       )(BLContextImpl* impl, double alpha) BL_NOEXCEPT;
  BLResult (BL_CDECL* getStyle[2]            )(const BLContextImpl* impl, BLStyleCore* out) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStyle[2]            )(BLContextImpl* impl, const BLStyleCore* style) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStyleRgba[2]        )(BLContextImpl* impl, const BLRgba* rgba) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStyleRgba32[2]      )(BLContextImpl* impl, uint32_t rgba32) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStyleRgba64[2]      )(BLContextImpl* impl, uint64_t rgba64) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStyleObject[2]      )(BLContextImpl* impl, const void* object) BL_NOEXCEPT;

  BLResult (BL_CDECL* setFillRule            )(BLContextImpl* impl, uint32_t fillRule) BL_NOEXCEPT;

  BLResult (BL_CDECL* setStrokeWidth         )(BLContextImpl* impl, double width) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStrokeMiterLimit    )(BLContextImpl* impl, double miterLimit) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStrokeCap           )(BLContextImpl* impl, uint32_t position, uint32_t strokeCap) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStrokeCaps          )(BLContextImpl* impl, uint32_t strokeCap) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStrokeJoin          )(BLContextImpl* impl, uint32_t strokeJoin) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStrokeDashOffset    )(BLContextImpl* impl, double dashOffset) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStrokeDashArray     )(BLContextImpl* impl, const BLArrayCore* dashArray) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStrokeTransformOrder)(BLContextImpl* impl, uint32_t transformOrder) BL_NOEXCEPT;
  BLResult (BL_CDECL* setStrokeOptions       )(BLContextImpl* impl, const BLStrokeOptionsCore* options) BL_NOEXCEPT;

  BLResult (BL_CDECL* clipToRectI            )(BLContextImpl* impl, const BLRectI* rect) BL_NOEXCEPT;
  BLResult (BL_CDECL* clipToRectD            )(BLContextImpl* impl, const BLRect* rect) BL_NOEXCEPT;
  BLResult (BL_CDECL* restoreClipping        )(BLContextImpl* impl) BL_NOEXCEPT;

  BLResult (BL_CDECL* clearAll               )(BLContextImpl* impl) BL_NOEXCEPT;
  BLResult (BL_CDECL* clearRectI             )(BLContextImpl* impl, const BLRectI* rect) BL_NOEXCEPT;
  BLResult (BL_CDECL* clearRectD             )(BLContextImpl* impl, const BLRect* rect) BL_NOEXCEPT;

  BLResult (BL_CDECL* fillAll                )(BLContextImpl* impl) BL_NOEXCEPT;
  BLResult (BL_CDECL* fillRectI              )(BLContextImpl* impl, const BLRectI* rect) BL_NOEXCEPT;
  BLResult (BL_CDECL* fillRectD              )(BLContextImpl* impl, const BLRect* rect) BL_NOEXCEPT;
  BLResult (BL_CDECL* fillPathD              )(BLContextImpl* impl, const BLPathCore* path) BL_NOEXCEPT;
  BLResult (BL_CDECL* fillGeometry           )(BLContextImpl* impl, uint32_t geometryType, const void* geometryData) BL_NOEXCEPT;
  BLResult (BL_CDECL* fillTextI              )(BLContextImpl* impl, const BLPointI* pt, const BLFontCore* font, const void* text, size_t size, uint32_t encoding) BL_NOEXCEPT;
  BLResult (BL_CDECL* fillTextD              )(BLContextImpl* impl, const BLPoint* pt, const BLFontCore* font, const void* text, size_t size, uint32_t encoding) BL_NOEXCEPT;
  BLResult (BL_CDECL* fillGlyphRunI          )(BLContextImpl* impl, const BLPointI* pt, const BLFontCore* font, const BLGlyphRun* glyphRun) BL_NOEXCEPT;
  BLResult (BL_CDECL* fillGlyphRunD          )(BLContextImpl* impl, const BLPoint* pt, const BLFontCore* font, const BLGlyphRun* glyphRun) BL_NOEXCEPT;

  BLResult (BL_CDECL* strokeRectI            )(BLContextImpl* impl, const BLRectI* rect) BL_NOEXCEPT;
  BLResult (BL_CDECL* strokeRectD            )(BLContextImpl* impl, const BLRect* rect) BL_NOEXCEPT;
  BLResult (BL_CDECL* strokePathD            )(BLContextImpl* impl, const BLPathCore* path) BL_NOEXCEPT;
  BLResult (BL_CDECL* strokeGeometry         )(BLContextImpl* impl, uint32_t geometryType, const void* geometryData) BL_NOEXCEPT;
  BLResult (BL_CDECL* strokeTextI            )(BLContextImpl* impl, const BLPointI* pt, const BLFontCore* font, const void* text, size_t size, uint32_t encoding) BL_NOEXCEPT;
  BLResult (BL_CDECL* strokeTextD            )(BLContextImpl* impl, const BLPoint* pt, const BLFontCore* font, const void* text, size_t size, uint32_t encoding) BL_NOEXCEPT;
  BLResult (BL_CDECL* strokeGlyphRunI        )(BLContextImpl* impl, const BLPointI* pt, const BLFontCore* font, const BLGlyphRun* glyphRun) BL_NOEXCEPT;
  BLResult (BL_CDECL* strokeGlyphRunD        )(BLContextImpl* impl, const BLPoint* pt, const BLFontCore* font, const BLGlyphRun* glyphRun) BL_NOEXCEPT;

  BLResult (BL_CDECL* blitImageI             )(BLContextImpl* impl, const BLPointI* pt, const BLImageCore* img, const BLRectI* imgArea) BL_NOEXCEPT;
  BLResult (BL_CDECL* blitImageD             )(BLContextImpl* impl, const BLPoint* pt, const BLImageCore* img, const BLRectI* imgArea) BL_NOEXCEPT;
  BLResult (BL_CDECL* blitScaledImageI       )(BLContextImpl* impl, const BLRectI* rect, const BLImageCore* img, const BLRectI* imgArea) BL_NOEXCEPT;
  BLResult (BL_CDECL* blitScaledImageD       )(BLContextImpl* impl, const BLRect* rect, const BLImageCore* img, const BLRectI* imgArea) BL_NOEXCEPT;
};

//! Rendering context [C Interface - Impl].
struct BLContextImpl {
  //! Virtual function table.
  const BLContextVirt* virt;

  //! Reference count.
  volatile size_t refCount;
  //! Impl type.
  uint8_t implType;
  //! Impl traits.
  uint8_t implTraits;
  //! Memory pool data.
  uint16_t memPoolData;
  //! Type of the context, see `BLContextType`.
  uint32_t contextType;

  //! Current state of the context.
  const BLContextState* state;
};

//! Rendering context [C Interface - Core].
struct BLContextCore {
  BLContextImpl* impl;
};

// ============================================================================
// [BLContext - C++]
// ============================================================================

#ifdef __cplusplus
//! Rendering context [C++ API].
class BLContext : public BLContextCore {
public:
  //! \cond INTERNAL
  static constexpr const uint32_t kImplType = BL_IMPL_TYPE_CONTEXT;

  // Only used by `BLContext` to make invocation of functions in `BLContextVirt`.
  enum OpType : uint32_t {
    kOpFill = BL_CONTEXT_OP_TYPE_FILL,
    kOpStroke = BL_CONTEXT_OP_TYPE_STROKE
  };
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  //! Creates a default constructed rendering context.
  //!
  //! Default constructed means that the instance is valid, but uninitialized,
  //! which means the rendering context does not have attached any target. Any
  //! attempt to use uninitialized context results in `BL_ERROR_NOT_INITIALIZED`
  //! error.
  BL_INLINE BLContext() noexcept { this->impl = none().impl; }

  //! Move constructor.
  //!
  //! Moves the `other` rendering context into this one and resets the `other`.
  BL_INLINE BLContext(BLContext&& other) noexcept { blVariantInitMove(this, &other); }

  //! Copy constructor.
  //!
  //! Creates a weak-copy of the `other` rendering context by increasing it's
  //! internal reference counter. This context and `other` would point to the
  //! same data and would be otherwise identical. Any change to `other` would
  //! also affect this context.
  //!
  //! This function is mostly provided for C++ users that may keep a global
  //! reference to the same rendering context, for example, otherwise sharing
  //! is not that useful as they also share a state.
  //!
  //! Two weak copies of the same rendering context cannot be used by different
  //! threads simultaneously.
  BL_INLINE BLContext(const BLContext& other) noexcept { blVariantInitWeak(this, &other); }

  //! Initializes this `BLContext` class with rendering context `impl`.
  //!
  //! Mostly for internal purposes and to keep the API consistent.
  BL_INLINE explicit BLContext(BLContextImpl* impl) noexcept { this->impl = impl; }

  //! Creates a new rendering context for rendering to the image `target`.
  BL_INLINE explicit BLContext(BLImage& target) noexcept { blContextInitAs(this, &target, nullptr); }
  //! Creates a new rendering context for rendering to the image `target`.
  //!
  //! This overload accepts create options that can be used to change the
  //! implementation of the rendering context.
  BL_INLINE BLContext(BLImage& target, const BLContextCreateInfo& createInfo) noexcept { blContextInitAs(this, &target, &createInfo); }
  //! \overload
  BL_INLINE BLContext(BLImage& target, const BLContextCreateInfo* createInfo) noexcept { blContextInitAs(this, &target, createInfo); }

  //! Destroys the rendering context.
  //!
  //! Waits for all operations, detaches the target from the rendering context
  //! and then destroys it. Does nothing if the context is not initialized.
  BL_INLINE ~BLContext() noexcept { blContextDestroy(this); }

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Returns true if the rendering context is initialized (has target attached).
  //!
  //! Provided for users that want to use bool idiom in C++ to check for the
  //! status of the object.
  //!
  //! ```
  //! if (ctx) {
  //!   // Rendering context is initialized.
  //! }
  //! ```
  BL_INLINE explicit operator bool() const noexcept { return !isNone(); }

  BL_INLINE BLContext& operator=(BLContext&& other) noexcept { blContextAssignMove(this, &other); return *this; }
  BL_INLINE BLContext& operator=(const BLContext& other) noexcept { blContextAssignWeak(this, &other); return *this; }

  //! Returns whether this and `other` point to the same rendering context.
  BL_INLINE bool operator==(const BLContext& other) const noexcept { return  equals(other); }
  //! Returns whether this and `other` are different rendering contexts.
  BL_INLINE bool operator!=(const BLContext& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Target Information
  //! \{

  //! Returns the target size in abstract units (pixels in case of `BLImage`).
  BL_INLINE BLSize targetSize() const noexcept { return impl->state->targetSize; }
  //! Returns the target width in abstract units (pixels in case of `BLImage`).
  BL_INLINE double targetWidth() const noexcept { return impl->state->targetSize.w; }
  //! Returns the target height in abstract units (pixels in case of `BLImage`).
  BL_INLINE double targetHeight() const noexcept { return impl->state->targetSize.h; }

  //! Returns the target image or null if there is no target image.
  //!
  //! \note The rendering context doesn't own the image, but it increases its
  //! writer count, which means that the image will not be destroyed even when
  //! user destroys it during the rendering (in such case it will be destroyed
  //! after the rendering ends when the writer count goes to zero). This means
  //! that the rendering context must hold the image and not the pointer to
  //! the `BLImage` passed to either the constructor or `begin()` function. So
  //! the returned pointer is not the same as the pointer passed to `begin()`,
  //! but it points to the same impl.
  BL_INLINE BLImage* targetImage() const noexcept { return blDownCast(impl->state->targetImage); }

  //! \}

  //! \name Context Lifetime and Others
  //! \{

  //! Returns the type of this context, see `BLContextType`.
  BL_INLINE uint32_t contextType() const noexcept { return impl->contextType; }

  //! Tests whether the context is a built-in null instance.
  BL_INLINE bool isNone() const noexcept { return (impl->implTraits & BL_IMPL_TRAIT_NULL) != 0; }

  //! Returns whether this and `other` point to the same rendering context.
  BL_INLINE bool equals(const BLContext& other) const noexcept { return this->impl == other.impl; }

  //! Resets this rendering context to the default constructed one.
  //!
  //! Similar behavior to the destructor, but the context will still be valid
  //! after `reset()` and would behave like a default constructed context.
  BL_INLINE BLResult reset() noexcept { return blContextReset(this); }

  BL_INLINE BLResult assign(BLContext&& other) noexcept { return blContextAssignMove(this, &other); }
  BL_INLINE BLResult assign(const BLContext& other) noexcept { return blContextAssignWeak(this, &other); }

  //! Begins rendering to the given `image`.
  //!
  //! If this operation succeeds then the rendering context will have exclusive
  //! access to the image data. This means that no other renderer can use it
  //! during rendering.
  BL_INLINE BLResult begin(BLImage& image) noexcept { return blContextBegin(this, &image, nullptr); }
  //! \overload
  BL_INLINE BLResult begin(BLImage& image, const BLContextCreateInfo& createInfo) noexcept { return blContextBegin(this, &image, &createInfo); }
  //! \overload
  BL_INLINE BLResult begin(BLImage& image, const BLContextCreateInfo* createInfo) noexcept { return blContextBegin(this, &image, createInfo); }

  //! Waits for completion of all render commands and detaches the rendering
  //! context from the rendering target. After `end()` completes the rendering
  //! context implementation would be released and replaced by a built-in null
  //! instance (no context).
  BL_INLINE BLResult end() noexcept { return blContextEnd(this); }

  //! Flushes the context, see `BLContextFlushFlags`.
  BL_INLINE BLResult flush(uint32_t flags) noexcept { return impl->virt->flush(impl, flags); }

  //! \}

  //! \name Context Properties
  //! \{

  BL_INLINE BLResult queryProperty(uint32_t propertyId, void* valueOut) const noexcept {
    return impl->virt->queryProperty(impl, propertyId, valueOut);
  }

  //! \cond INTERNAL
  template<typename T>
  BL_INLINE T _queryPropertyT(uint32_t propertyId) const noexcept {
    T result {};
    queryProperty(propertyId, &result);
    return result;
  }
  //! \endcond

  //! Queries the number of threads that the rendering context uses.
  //!
  //! If the returned value is zero it means that the rendering is synchronous,
  //! otherwise it describes the number of threads used for asynchronous rendering
  //! which include the user thread. For example if the returned value is `2` it
  //! means that the rendering context uses the user thread and one more worker.
  BL_INLINE uint32_t queryThreadCount() const noexcept {
    return _queryPropertyT<uint32_t>(BL_CONTEXT_PROPERTY_THREAD_COUNT);
  }

  //! Queries accumulated errors as flags, see \ref BLContextErrorFlags.
  //!
  //! Errors may accumulate during the lifetime of the rendering context.
  BL_INLINE uint32_t queryAccumulatedErrorFlags() const noexcept {
    return _queryPropertyT<uint32_t>(BL_CONTEXT_PROPERTY_ACCUMULATED_ERROR_FLAGS);
  }

  //! \}

  //! \name State Management
  //! \{

  //! Returns the number of saved states in the context (0 means no saved states).
  BL_INLINE size_t savedStateCount() const noexcept { return impl->state->savedStateCount; }

  //! Saves the current rendering context state.
  //!
  //! Blend2D uses optimizations that make `save()` a cheap operation. Only core
  //! values are actually saved in `save()`, others will only be saved if they
  //! are modified. This means that consecutive calls to `save()` and `restore()`
  //! do almost nothing.
  BL_INLINE BLResult save() noexcept { return impl->virt->save(impl, nullptr); }

  //! Saves the current rendering context state and creates a restoration `cookie`.
  //!
  //! If you use a `cookie` to save a state you have to use the same cookie to
  //! restore it otherwise the `restore()` would fail. Please note that cookies
  //! are not a means of security, they are provided for making it easier to
  //! guarantee that a code that you may not control won't break your context.
  BL_INLINE BLResult save(BLContextCookie& cookie) noexcept { return impl->virt->save(impl, &cookie); }

  //! Restores the top-most saved context-state.
  //!
  //! Possible return conditions:
  //!
  //!   * `BL_SUCCESS` - State was restored successfully.
  //!   * `BL_ERROR_NO_STATES_TO_RESTORE` - There are no saved states to restore.
  //!   * `BL_ERROR_NO_MATCHING_COOKIE` - Previous state was saved with cookie,
  //!     which was not provided. You would need the correct cookie to restore
  //!     such state.
  BL_INLINE BLResult restore() noexcept { return impl->virt->restore(impl, nullptr); }

  //! Restores to the point that matches the given `cookie`.
  //!
  //! More than one state can be restored in case that the `cookie` points to
  //! some previous state in the list.
  //!
  //! Possible return conditions:
  //!
  //!   * `BL_SUCCESS` - Matching state was restored successfully.
  //!   * `BL_ERROR_NO_STATES_TO_RESTORE` - There are no saved states to restore.
  //!   * `BL_ERROR_NO_MATCHING_COOKIE` - The cookie did't match any saved state.
  BL_INLINE BLResult restore(const BLContextCookie& cookie) noexcept { return impl->virt->restore(impl, &cookie); }

  //! \}

  //! \name Transformations
  //! \{

  //! Returns meta-matrix.
  //!
  //! Meta matrix is a core transformation matrix that is normally not changed
  //! by transformations applied to the context. Instead it acts as a secondary
  //! matrix used to create the final transformation matrix from meta and user
  //! matrices.
  //!
  //! Meta matrix can be used to scale the whole context for HI-DPI rendering
  //! or to change the orientation of the image being rendered, however, the
  //! number of use-cases is unlimited.
  //!
  //! To change the meta-matrix you must first change user-matrix and then call
  //! `userToMeta()`, which would update meta-matrix and clear user-matrix.
  //!
  //! See `userMatrix()` and `userToMeta()`.
  BL_INLINE const BLMatrix2D& metaMatrix() const noexcept { return impl->state->metaMatrix; }

  //! Returns user-matrix.
  //!
  //! User matrix contains all transformations that happened to the rendering
  //! context unless the context was restored or `userToMeta()` was called.
  BL_INLINE const BLMatrix2D& userMatrix() const noexcept { return impl->state->userMatrix; }

  //! Applies a matrix operation to the current transformation matrix (internal).
  BL_INLINE BLResult _applyMatrixOp(uint32_t opType, const void* opData) noexcept {
    return impl->virt->matrixOp(impl, opType, opData);
  }

  //! \cond INTERNAL
  //! Applies a matrix operation to the current transformation matrix (internal).
  template<typename... Args>
  BL_INLINE BLResult _applyMatrixOpV(uint32_t opType, Args&&... args) noexcept {
    double opData[] = { double(args)... };
    return impl->virt->matrixOp(impl, opType, opData);
  }
  //! \endcond

  //! Sets user matrix to `m`.
  BL_INLINE BLResult setMatrix(const BLMatrix2D& m) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_ASSIGN, &m); }
  //! Resets user matrix to identity.
  BL_INLINE BLResult resetMatrix() noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_RESET, nullptr); }

  BL_INLINE BLResult translate(double x, double y) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_TRANSLATE, x, y); }
  BL_INLINE BLResult translate(const BLPointI& p) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_TRANSLATE, p.x, p.y); }
  BL_INLINE BLResult translate(const BLPoint& p) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_TRANSLATE, &p); }
  BL_INLINE BLResult scale(double xy) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_SCALE, xy, xy); }
  BL_INLINE BLResult scale(double x, double y) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_SCALE, x, y); }
  BL_INLINE BLResult scale(const BLPointI& p) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_SCALE, p.x, p.y); }
  BL_INLINE BLResult scale(const BLPoint& p) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_SCALE, &p); }
  BL_INLINE BLResult skew(double x, double y) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_SKEW, x, y); }
  BL_INLINE BLResult skew(const BLPoint& p) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_SKEW, &p); }
  BL_INLINE BLResult rotate(double angle) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_ROTATE, &angle); }
  BL_INLINE BLResult rotate(double angle, double x, double y) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_ROTATE_PT, angle, x, y); }
  BL_INLINE BLResult rotate(double angle, const BLPoint& p) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_ROTATE_PT, angle, p.x, p.y); }
  BL_INLINE BLResult rotate(double angle, const BLPointI& p) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_ROTATE_PT, angle, p.x, p.y); }
  BL_INLINE BLResult transform(const BLMatrix2D& m) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_TRANSFORM, &m); }

  BL_INLINE BLResult postTranslate(double x, double y) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_POST_TRANSLATE, x, y); }
  BL_INLINE BLResult postTranslate(const BLPointI& p) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_POST_TRANSLATE, p.x, p.y); }
  BL_INLINE BLResult postTranslate(const BLPoint& p) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_POST_TRANSLATE, &p); }
  BL_INLINE BLResult postScale(double xy) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_POST_SCALE, xy, xy); }
  BL_INLINE BLResult postScale(double x, double y) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_POST_SCALE, x, y); }
  BL_INLINE BLResult postScale(const BLPointI& p) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_POST_SCALE, p.x, p.y); }
  BL_INLINE BLResult postScale(const BLPoint& p) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_POST_SCALE, &p); }
  BL_INLINE BLResult postSkew(double x, double y) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_POST_SKEW, x, y); }
  BL_INLINE BLResult postSkew(const BLPoint& p) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_POST_SKEW, &p); }
  BL_INLINE BLResult postRotate(double angle) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_POST_ROTATE, &angle); }
  BL_INLINE BLResult postRotate(double angle, double x, double y) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_POST_ROTATE_PT, angle, x, y); }
  BL_INLINE BLResult postRotate(double angle, const BLPoint& p) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_POST_ROTATE_PT, angle, p.x, p.y); }
  BL_INLINE BLResult postRotate(double angle, const BLPointI& p) noexcept { return _applyMatrixOpV(BL_MATRIX2D_OP_POST_ROTATE_PT, angle, p.x, p.y); }
  BL_INLINE BLResult postTransform(const BLMatrix2D& m) noexcept { return _applyMatrixOp(BL_MATRIX2D_OP_POST_TRANSFORM, &m); }

  //! Store the result of combining the current `MetaMatrix` and `UserMatrix`
  //! to `MetaMatrix` and reset `UserMatrix` to identity as shown below:
  //!
  //! ```
  //! MetaMatrix = MetaMatrix x UserMatrix
  //! UserMatrix = Identity
  //! ```
  //!
  //! Please note that this operation is irreversible. The only way to restore
  //! both matrices to the state before the call to `userToMeta()` is to use
  //! `save()` and `restore()` functions.
  BL_INLINE BLResult userToMeta() noexcept { return impl->virt->userToMeta(impl); }

  //! \}

  //! \name Rendering Hints
  //! \{

  //! Returns rendering hints.
  BL_INLINE const BLContextHints& hints() const noexcept { return impl->state->hints; }

  //! Sets the given rendering hint `hintType` to the `value`.
  BL_INLINE BLResult setHint(uint32_t hintType, uint32_t value) noexcept { return impl->virt->setHint(impl, hintType, value); }
  //! Sets all rendering hints of this context to the given `hints`.
  BL_INLINE BLResult setHints(const BLContextHints& hints) noexcept { return impl->virt->setHints(impl, &hints); }

  BL_INLINE BLResult setRenderingQuality(uint32_t value) noexcept { return setHint(BL_CONTEXT_HINT_RENDERING_QUALITY, value); }
  BL_INLINE BLResult setGradientQuality(uint32_t value) noexcept { return setHint(BL_CONTEXT_HINT_GRADIENT_QUALITY, value); }
  BL_INLINE BLResult setPatternQuality(uint32_t value) noexcept { return setHint(BL_CONTEXT_HINT_PATTERN_QUALITY, value); }

  //! \}

  //! \name Approximation Options
  //! \{

  //! Returns approximation options.
  BL_INLINE const BLApproximationOptions& approximationOptions() const noexcept { return impl->state->approximationOptions; }

  //! Returns flatten mode (how curves are flattened), see `BLFlattenMmode`.
  BL_INLINE uint32_t flattenMode() const noexcept { return impl->state->approximationOptions.flattenMode; }
  //! Sets flatten `mode` (how curves are flattened), see `BLFlattenMmode`.
  BL_INLINE BLResult setFlattenMode(uint32_t mode) noexcept { return impl->virt->setFlattenMode(impl, mode); }

  //! Returns tolerance used for curve flattening.
  BL_INLINE double flattenTolerance() const noexcept { return impl->state->approximationOptions.flattenTolerance; }
  //! Sets tolerance used for curve flattening.
  BL_INLINE BLResult setFlattenTolerance(double tolerance) noexcept { return impl->virt->setFlattenTolerance(impl, tolerance); }

  //! \}

  //! \name Composition Options
  //! \{

  //! Returns composition operator.
  BL_INLINE uint32_t compOp() const noexcept { return impl->state->compOp; }
  //! Sets composition operator to `compOp`, see `BLCompOp`.
  BL_INLINE BLResult setCompOp(uint32_t compOp) noexcept { return impl->virt->setCompOp(impl, compOp); }

  //! Returns global alpha value.
  BL_INLINE double globalAlpha() const noexcept { return impl->state->globalAlpha; }
  //! Sets global alpha value.
  BL_INLINE BLResult setGlobalAlpha(double alpha) noexcept { return impl->virt->setGlobalAlpha(impl, alpha); }

  //! \}

  //! \name Style Options
  //! \{

  BL_INLINE uint32_t styleType(uint32_t opType) const noexcept {
    return opType <= BL_CONTEXT_OP_TYPE_COUNT ? uint32_t(impl->state->styleType[opType]) : uint32_t(0);
  }

  BL_INLINE BLResult getStyle(uint32_t opType, BLStyle& styleOut) const noexcept {
    if (BL_UNLIKELY(opType >= BL_CONTEXT_OP_TYPE_COUNT))
      return blTraceError(BL_ERROR_INVALID_VALUE);
    return impl->virt->getStyle[opType](impl, &styleOut);
  }

  BL_INLINE BLResult setStyle(uint32_t opType, const BLStyle& style) noexcept {
    if (BL_UNLIKELY(opType >= BL_CONTEXT_OP_TYPE_COUNT))
      return blTraceError(BL_ERROR_INVALID_VALUE);
    return impl->virt->setStyle[opType](impl, &style);
  }

  BL_INLINE BLResult setStyle(uint32_t opType, const BLRgba& rgba) noexcept {
    if (BL_UNLIKELY(opType >= BL_CONTEXT_OP_TYPE_COUNT))
      return blTraceError(BL_ERROR_INVALID_VALUE);
    return impl->virt->setStyleRgba[opType](impl, &rgba);
  }

  BL_INLINE BLResult setStyle(uint32_t opType, const BLRgba32& rgba32) noexcept {
    if (BL_UNLIKELY(opType >= BL_CONTEXT_OP_TYPE_COUNT))
      return blTraceError(BL_ERROR_INVALID_VALUE);
    return impl->virt->setStyleRgba32[opType](impl, rgba32.value);
  }

  BL_INLINE BLResult setStyle(uint32_t opType, const BLRgba64& rgba64) noexcept {
    if (BL_UNLIKELY(opType >= BL_CONTEXT_OP_TYPE_COUNT))
      return blTraceError(BL_ERROR_INVALID_VALUE);
    return impl->virt->setStyleRgba64[opType](impl, rgba64.value);
  }

  BL_INLINE BLResult setStyle(uint32_t opType, const BLPattern& pattern) noexcept {
    if (BL_UNLIKELY(opType >= BL_CONTEXT_OP_TYPE_COUNT))
      return blTraceError(BL_ERROR_INVALID_VALUE);
    return impl->virt->setStyleObject[opType](impl, &pattern);
  }

  BL_INLINE BLResult setStyle(uint32_t opType, const BLGradient& gradient) noexcept {
    if (BL_UNLIKELY(opType >= BL_CONTEXT_OP_TYPE_COUNT))
      return blTraceError(BL_ERROR_INVALID_VALUE);
    return impl->virt->setStyleObject[opType](impl, &gradient);
  }

  BL_INLINE BLResult setStyle(uint32_t opType, const BLImage& image) noexcept {
    if (BL_UNLIKELY(opType >= BL_CONTEXT_OP_TYPE_COUNT))
      return blTraceError(BL_ERROR_INVALID_VALUE);
    return impl->virt->setStyleObject[opType](impl, &image);
  }

  //! Returns fill or alpha value dependeing on the rendering operation `opType`.
  //!
  //! The function behaves like `fillAlpha()` or `strokeAlpha()` depending on
  //! `opType` value, see `BLContextOpType`.
  BL_INLINE double styleAlpha(uint32_t opType) const noexcept {
    return opType < BL_CONTEXT_OP_TYPE_COUNT ? impl->state->styleAlpha[opType] : 0.0;
  }

  //! Set fill or stroke `alpha` value depending on the rendering operation `opType`.
  //!
  //! The function behaves like `setFillAlpha()` or `setStrokeAlpha()` depending
  //! on `opType` value, see `BLContextOpType`.
  BL_INLINE BLResult setStyleAlpha(uint32_t opType, double alpha) noexcept {
    if (BL_UNLIKELY(opType >= BL_CONTEXT_OP_TYPE_COUNT))
      return blTraceError(BL_ERROR_INVALID_VALUE);
    return impl->virt->setStyleAlpha[opType](impl, alpha);
  }

  //! \}

  //! \name Fill Style & Options
  //! \{

  BL_INLINE uint32_t fillStyleType() const noexcept { return impl->state->styleType[kOpFill]; }
  BL_INLINE BLResult getFillStyle(BLStyle& out) const noexcept { return impl->virt->getStyle[kOpFill](impl, &out); }

  BL_INLINE BLResult setFillStyle(const BLStyle& style) noexcept { return impl->virt->setStyle[kOpFill](impl, &style); }
  BL_INLINE BLResult setFillStyle(const BLRgba& rgba) noexcept { return impl->virt->setStyleRgba[kOpFill](impl, &rgba); }
  BL_INLINE BLResult setFillStyle(const BLRgba32& rgba32) noexcept { return impl->virt->setStyleRgba32[kOpFill](impl, rgba32.value); }
  BL_INLINE BLResult setFillStyle(const BLRgba64& rgba64) noexcept { return impl->virt->setStyleRgba64[kOpFill](impl, rgba64.value); }
  BL_INLINE BLResult setFillStyle(const BLPattern& pattern) noexcept { return impl->virt->setStyleObject[kOpFill](impl, &pattern); }
  BL_INLINE BLResult setFillStyle(const BLGradient& gradient) noexcept { return impl->virt->setStyleObject[kOpFill](impl, &gradient); }

  //! Returns fill alpha value.
  BL_INLINE double fillAlpha() const noexcept { return impl->state->styleAlpha[kOpFill]; }
  //! Sets fill `alpha` value.
  BL_INLINE BLResult setFillAlpha(double alpha) noexcept { return impl->virt->setStyleAlpha[kOpFill](impl, alpha); }

  //! Returns fill-rule, see `BLFillRule`.
  BL_INLINE uint32_t fillRule() const noexcept { return impl->state->fillRule; }
  //! Sets fill-rule, see `BLFillRule`.
  BL_INLINE BLResult setFillRule(uint32_t fillRule) noexcept { return impl->virt->setFillRule(impl, fillRule); }

  //! \}

  //! \name Stroke Style & Options
  //! \{

  BL_INLINE uint32_t strokeStyleType() const noexcept { return impl->state->styleType[kOpStroke]; }
  BL_INLINE BLResult getStrokeStyle(BLStyle& out) const noexcept { return impl->virt->getStyle[kOpStroke](impl, &out); }

  BL_INLINE BLResult setStrokeStyle(const BLStyle& style) noexcept { return impl->virt->setStyle[kOpStroke](impl, &style); }
  BL_INLINE BLResult setStrokeStyle(const BLRgba& rgba) noexcept { return impl->virt->setStyleRgba[kOpStroke](impl, &rgba); }
  BL_INLINE BLResult setStrokeStyle(const BLRgba32& rgba32) noexcept { return impl->virt->setStyleRgba32[kOpStroke](impl, rgba32.value); }
  BL_INLINE BLResult setStrokeStyle(const BLRgba64& rgba64) noexcept { return impl->virt->setStyleRgba64[kOpStroke](impl, rgba64.value); }
  BL_INLINE BLResult setStrokeStyle(const BLPattern& pattern) noexcept { return impl->virt->setStyleObject[kOpStroke](impl, &pattern); }
  BL_INLINE BLResult setStrokeStyle(const BLGradient& gradient) noexcept { return impl->virt->setStyleObject[kOpStroke](impl, &gradient); }

  //! Returns stroke width.
  BL_INLINE double strokeWidth() const noexcept { return impl->state->strokeOptions.width; }
  //! Returns stroke miter-limit.
  BL_INLINE double strokeMiterLimit() const noexcept { return impl->state->strokeOptions.miterLimit; }
  //! Returns stroke join, see `BLStrokeJoin`.
  BL_INLINE uint32_t strokeJoin() const noexcept { return impl->state->strokeOptions.join; }
  //! Returns stroke start-cap, see `BLStrokeCap`.
  BL_INLINE uint32_t strokeStartCap() const noexcept { return impl->state->strokeOptions.startCap; }
  //! Returns stroke end-cap, see `BLStrokeCap`.
  BL_INLINE uint32_t strokeEndCap() const noexcept { return impl->state->strokeOptions.endCap; }
  //! Returns stroke dash-offset.
  BL_INLINE double strokeDashOffset() const noexcept { return impl->state->strokeOptions.dashOffset; }
  //! Returns stroke dash-array.
  BL_INLINE const BLArray<double>& strokeDashArray() const noexcept { return impl->state->strokeOptions.dashArray; }
  //! Returns stroke transform order, see `BLStrokeTransformOrder`.
  BL_INLINE uint32_t strokeTransformOrder() const noexcept { return impl->state->strokeOptions.transformOrder; }
  //! Returns stroke options as a reference to `BLStrokeOptions`.
  BL_INLINE const BLStrokeOptions& strokeOptions() const noexcept { return impl->state->strokeOptions; }

  //! Sets stroke width to `width`.
  BL_INLINE BLResult setStrokeWidth(double width) noexcept { return impl->virt->setStrokeWidth(impl, width); }
  //! Sets miter limit to `miterLimit`.
  BL_INLINE BLResult setStrokeMiterLimit(double miterLimit) noexcept { return impl->virt->setStrokeMiterLimit(impl, miterLimit); }
  //! Sets stroke join to `strokeJoin`, see `BLStrokeJoin`.
  BL_INLINE BLResult setStrokeJoin(uint32_t strokeJoin) noexcept { return impl->virt->setStrokeJoin(impl, strokeJoin); }
  //! Sets stroke cap of the specified `type` to `strokeCap`, see `BLStrokeCap`.
  BL_INLINE BLResult setStrokeCap(uint32_t type, uint32_t strokeCap) noexcept { return impl->virt->setStrokeCap(impl, type, strokeCap); }
  //! Sets stroke start cap to `strokeCap`, see `BLStrokeCap`.
  BL_INLINE BLResult setStrokeStartCap(uint32_t strokeCap) noexcept { return setStrokeCap(BL_STROKE_CAP_POSITION_START, strokeCap); }
  //! Sets stroke end cap to `strokeCap`, see `BLStrokeCap`.
  BL_INLINE BLResult setStrokeEndCap(uint32_t strokeCap) noexcept { return setStrokeCap(BL_STROKE_CAP_POSITION_END, strokeCap); }
  //! Sets all stroke caps to `strokeCap`, see `BLStrokeCap`.
  BL_INLINE BLResult setStrokeCaps(uint32_t strokeCap) noexcept { return impl->virt->setStrokeCaps(impl, strokeCap); }
  //! Sets stroke dash-offset to `dashOffset`.
  BL_INLINE BLResult setStrokeDashOffset(double dashOffset) noexcept { return impl->virt->setStrokeDashOffset(impl, dashOffset); }
  //! Sets stroke dash-array to `dashArray`.
  BL_INLINE BLResult setStrokeDashArray(const BLArray<double>& dashArray) noexcept { return impl->virt->setStrokeDashArray(impl, &dashArray); }
  //! Sets stroke transformation order to `transformOrder`, see `BLStrokeTransformOrder`.
  BL_INLINE BLResult setStrokeTransformOrder(uint32_t transformOrder) noexcept { return impl->virt->setStrokeTransformOrder(impl, transformOrder); }
  //! Sets all stroke `options`.
  BL_INLINE BLResult setStrokeOptions(const BLStrokeOptions& options) noexcept { return impl->virt->setStrokeOptions(impl, &options); }

  //! Returns stroke alpha value.
  BL_INLINE double strokeAlpha() const noexcept { return impl->state->styleAlpha[kOpStroke]; }
  //! Sets stroke alpha value to `alpha`.
  BL_INLINE BLResult setStrokeAlpha(double alpha) noexcept { return impl->virt->setStyleAlpha[kOpStroke](impl, alpha); }

  //! \}

  //! \name Clip Operations
  //! \{

  //! Restores clipping to the last saved state or to the context default
  //! clipping if there is no saved state.
  //!
  //! If there are no saved states then it resets clipping completely to the
  //! initial state that was used when the rendering context was created.
  BL_INLINE BLResult restoreClipping() noexcept { return impl->virt->restoreClipping(impl); }

  BL_INLINE BLResult clipToRect(const BLRectI& rect) noexcept { return impl->virt->clipToRectI(impl, &rect); }
  BL_INLINE BLResult clipToRect(const BLRect& rect) noexcept { return impl->virt->clipToRectD(impl, &rect); }
  BL_INLINE BLResult clipToRect(double x, double y, double w, double h) noexcept { return clipToRect(BLRect(x, y, w, h)); }

  //! \}

  //! \name Clear Operations
  //! \{

  //! Clear everything.
  BL_INLINE BLResult clearAll() noexcept { return impl->virt->clearAll(impl); }

  //! Clears a rectangle `rect`.
  BL_INLINE BLResult clearRect(const BLRectI& rect) noexcept { return impl->virt->clearRectI(impl, &rect); }
  //! Clears a rectangle `rect`.
  BL_INLINE BLResult clearRect(const BLRect& rect) noexcept { return impl->virt->clearRectD(impl, &rect); }
  //! \overload
  BL_INLINE BLResult clearRect(double x, double y, double w, double h) noexcept { return clearRect(BLRect(x, y, w, h)); }

  //! \}

  //! \name Fill Operations
  //! \{

  //! Fills the passed geometry specified by `geometryType` and `geometryData` [Internal].
  BL_INLINE BLResult fillGeometry(uint32_t geometryType, const void* geometryData) noexcept { return impl->virt->fillGeometry(impl, geometryType, geometryData); }

  //! Fills everything.
  BL_INLINE BLResult fillAll() noexcept { return impl->virt->fillAll(impl); }

  //! Fills a box.
  BL_INLINE BLResult fillBox(const BLBox& box) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_BOXD, &box); }
  // \overload
  BL_INLINE BLResult fillBox(const BLBoxI& box) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_BOXI, &box); }
  // \overload
  BL_INLINE BLResult fillBox(double x0, double y0, double x1, double y1) noexcept { return fillBox(BLBox(x0, y0, x1, y1)); }

  //! Fills a rectangle `rect`.
  BL_INLINE BLResult fillRect(const BLRectI& rect) noexcept { return impl->virt->fillRectI(impl, &rect); }
  //! Fills a rectangle `rect`.
  BL_INLINE BLResult fillRect(const BLRect& rect) noexcept { return impl->virt->fillRectD(impl, &rect); }
  //! \overload
  BL_INLINE BLResult fillRect(double x, double y, double w, double h) noexcept { return fillRect(BLRect(x, y, w, h)); }

  //! Fills a circle.
  BL_INLINE BLResult fillCircle(const BLCircle& circle) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_CIRCLE, &circle); }
  //! \overload
  BL_INLINE BLResult fillCircle(double cx, double cy, double r) noexcept { return fillCircle(BLCircle(cx, cy, r)); }

  //! Fills an ellipse.
  BL_INLINE BLResult fillEllipse(const BLEllipse& ellipse) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_ELLIPSE, &ellipse); }
  //! \overload
  BL_INLINE BLResult fillEllipse(double cx, double cy, double rx, double ry) noexcept { return fillEllipse(BLEllipse(cx, cy, rx, ry)); }

  //! Fills a rounded rectangle.
  BL_INLINE BLResult fillRoundRect(const BLRoundRect& rr) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_ROUND_RECT, &rr); }
  //! \overload
  BL_INLINE BLResult fillRoundRect(const BLRect& rect, double r) noexcept { return fillRoundRect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, r)); }
  //! \overload
  BL_INLINE BLResult fillRoundRect(const BLRect& rect, double rx, double ry) noexcept { return fillRoundRect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, rx, ry)); }
  //! \overload
  BL_INLINE BLResult fillRoundRect(double x, double y, double w, double h, double r) noexcept { return fillRoundRect(BLRoundRect(x, y, w, h, r)); }
  //! \overload
  BL_INLINE BLResult fillRoundRect(double x, double y, double w, double h, double rx, double ry) noexcept { return fillRoundRect(BLRoundRect(x, y, w, h, rx, ry)); }

  //! Fills a chord.
  BL_INLINE BLResult fillChord(const BLArc& chord) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_CHORD, &chord); }
  //! \overload
  BL_INLINE BLResult fillChord(double cx, double cy, double r, double start, double sweep) noexcept { return fillChord(BLArc(cx, cy, r, r, start, sweep)); }
  //! \overload
  BL_INLINE BLResult fillChord(double cx, double cy, double rx, double ry, double start, double sweep) noexcept { return fillChord(BLArc(cx, cy, rx, ry, start, sweep)); }

  //! Fills a pie.
  BL_INLINE BLResult fillPie(const BLArc& pie) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_PIE, &pie); }
  //! \overload
  BL_INLINE BLResult fillPie(double cx, double cy, double r, double start, double sweep) noexcept { return fillPie(BLArc(cx, cy, r, r, start, sweep)); }
  //! \overload
  BL_INLINE BLResult fillPie(double cx, double cy, double rx, double ry, double start, double sweep) noexcept { return fillPie(BLArc(cx, cy, rx, ry, start, sweep)); }

  //! Fills a triangle.
  BL_INLINE BLResult fillTriangle(const BLTriangle& triangle) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_TRIANGLE, &triangle); }
  //! \overload
  BL_INLINE BLResult fillTriangle(double x0, double y0, double x1, double y1, double x2, double y2) noexcept { return fillTriangle(BLTriangle(x0, y0, x1, y1, x2, y2)); }

  //! Fills a polygon.
  BL_INLINE BLResult fillPolygon(const BLArrayView<BLPoint>& poly) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_POLYGOND, &poly); }
  //! \overload
  BL_INLINE BLResult fillPolygon(const BLPoint* poly, size_t n) noexcept { return fillPolygon(BLArrayView<BLPoint>{poly, n}); }

  //! Fills a polygon.
  BL_INLINE BLResult fillPolygon(const BLArrayView<BLPointI>& poly) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_POLYGONI, &poly); }
  //! \overload
  BL_INLINE BLResult fillPolygon(const BLPointI* poly, size_t n) noexcept { return fillPolygon(BLArrayView<BLPointI>{poly, n}); }

  //! Fills an array of boxes.
  BL_INLINE BLResult fillBoxArray(const BLArrayView<BLBox>& array) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD, &array); }
  //! \overload
  BL_INLINE BLResult fillBoxArray(const BLBox* data, size_t n) noexcept { return fillBoxArray(BLArrayView<BLBox>{data, n}); }

  //! Fills an array of boxes.
  BL_INLINE BLResult fillBoxArray(const BLArrayView<BLBoxI>& array) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI, &array); }
  //! \overload
  BL_INLINE BLResult fillBoxArray(const BLBoxI* data, size_t n) noexcept { return fillBoxArray(BLArrayView<BLBoxI>{data, n}); }

  //! Fills an array of rectangles.
  BL_INLINE BLResult fillRectArray(const BLArrayView<BLRect>& array) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD, &array); }
  //! \overload
  BL_INLINE BLResult fillRectArray(const BLRect* data, size_t n) noexcept { return fillRectArray(BLArrayView<BLRect>{data, n}); }

  //! Fills an array of rectangles.
  BL_INLINE BLResult fillRectArray(const BLArrayView<BLRectI>& array) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI, &array); }
  //! \overload
  BL_INLINE BLResult fillRectArray(const BLRectI* data, size_t n) noexcept { return fillRectArray(BLArrayView<BLRectI>{data, n}); }

  //! Fills the given `region`.
  BL_INLINE BLResult fillRegion(const BLRegion& region) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_REGION, &region); }

  //! Fills the given `path`.
  BL_INLINE BLResult fillPath(const BLPath& path) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_PATH, &path); }

  //! Fills the passed UTF-8 text by using the given `font`.
  BL_INLINE BLResult fillUtf8Text(const BLPointI& dst, const BLFont& font, const char* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->fillTextI(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF8);
  }

  //! Fills the passed UTF-8 text by using the given `font`.
  BL_INLINE BLResult fillUtf8Text(const BLPoint& dst, const BLFont& font, const char* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->fillTextD(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF8);
  }

  //! Fills the passed UTF-16 text by using the given `font`.
  BL_INLINE BLResult fillUtf16Text(const BLPointI& dst, const BLFont& font, const uint16_t* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->fillTextI(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF16);
  }

  //! Fills the passed UTF-16 text by using the given `font`.
  BL_INLINE BLResult fillUtf16Text(const BLPoint& dst, const BLFont& font, const uint16_t* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->fillTextD(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF16);
  }

  //! Fills the passed UTF-32 text by using the given `font`.
  BL_INLINE BLResult fillUtf32Text(const BLPointI& dst, const BLFont& font, const uint32_t* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->fillTextI(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF32);
  }

  //! Fills the passed UTF-32 text by using the given `font`.
  BL_INLINE BLResult fillUtf32Text(const BLPoint& dst, const BLFont& font, const uint32_t* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->fillTextD(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF32);
  }

  //! Fills the passed `glyphRun` by using the given `font`.
  BL_INLINE BLResult fillGlyphRun(const BLPointI& dst, const BLFont& font, const BLGlyphRun& glyphRun) noexcept {
    return impl->virt->fillGlyphRunI(impl, &dst, &font, &glyphRun);
  }

  //! Fills the passed `glyphRun` by using the given `font`.
  BL_INLINE BLResult fillGlyphRun(const BLPoint& dst, const BLFont& font, const BLGlyphRun& glyphRun) noexcept {
    return impl->virt->fillGlyphRunD(impl, &dst, &font, &glyphRun);
  }

  //! \}

  //! \name Stroke Operations
  //! \{

  //! Strokes the passed geometry specified by `geometryType` and `geometryData` [Internal].
  BL_INLINE BLResult strokeGeometry(uint32_t geometryType, const void* geometryData) noexcept { return impl->virt->strokeGeometry(impl, geometryType, geometryData); }

  //! Strokes a box.
  BL_INLINE BLResult strokeBox(const BLBox& box) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_BOXD, &box); }
  // \overload
  BL_INLINE BLResult strokeBox(const BLBoxI& box) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_BOXI, &box); }
  // \overload
  BL_INLINE BLResult strokeBox(double x0, double y0, double x1, double y1) noexcept { return strokeBox(BLBox(x0, y0, x1, y1)); }

  //! Strokes a rectangle.
  BL_INLINE BLResult strokeRect(const BLRect& rect) noexcept { return impl->virt->strokeRectD(impl, &rect); }
  //! \overload
  BL_INLINE BLResult strokeRect(const BLRectI& rect) noexcept { return impl->virt->strokeRectI(impl, &rect); }
  //! \overload
  BL_INLINE BLResult strokeRect(double x, double y, double w, double h) noexcept { return strokeRect(BLRect(x, y, w, h)); }

  //! Strokes a line.
  BL_INLINE BLResult strokeLine(const BLLine& line) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_LINE, &line); }
  //! \overload
  BL_INLINE BLResult strokeLine(const BLPoint& p0, const BLPoint& p1) noexcept { return strokeLine(BLLine(p0.x, p0.y, p1.x, p1.y)); }
  //! \overload
  BL_INLINE BLResult strokeLine(double x0, double y0, double x1, double y1) noexcept { return strokeLine(BLLine(x0, y0, x1, y1)); }

  //! Strokes a circle.
  BL_INLINE BLResult strokeCircle(const BLCircle& circle) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_CIRCLE, &circle); }
  //! \overload
  BL_INLINE BLResult strokeCircle(double cx, double cy, double r) noexcept { return strokeCircle(BLCircle(cx, cy, r)); }

  //! Strokes an ellipse.
  BL_INLINE BLResult strokeEllipse(const BLEllipse& ellipse) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_ELLIPSE, &ellipse); }
  //! \overload
  BL_INLINE BLResult strokeEllipse(double cx, double cy, double rx, double ry) noexcept { return strokeEllipse(BLEllipse(cx, cy, rx, ry)); }

  //! Strokes a round.
  BL_INLINE BLResult strokeRoundRect(const BLRoundRect& rr) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_ROUND_RECT, &rr); }
  //! \overload
  BL_INLINE BLResult strokeRoundRect(const BLRect& rect, double r) noexcept { return strokeRoundRect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, r)); }
  //! \overload
  BL_INLINE BLResult strokeRoundRect(const BLRect& rect, double rx, double ry) noexcept { return strokeRoundRect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, rx, ry)); }
  //! \overload
  BL_INLINE BLResult strokeRoundRect(double x, double y, double w, double h, double r) noexcept { return strokeRoundRect(BLRoundRect(x, y, w, h, r)); }
  //! \overload
  BL_INLINE BLResult strokeRoundRect(double x, double y, double w, double h, double rx, double ry) noexcept { return strokeRoundRect(BLRoundRect(x, y, w, h, rx, ry)); }

  //! Strokes an arc.
  BL_INLINE BLResult strokeArc(const BLArc& arc) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_ARC, &arc); }
  //! \overload
  BL_INLINE BLResult strokeArc(double cx, double cy, double r, double start, double sweep) noexcept { return strokeArc(BLArc(cx, cy, r, r, start, sweep)); }
  //! \overload
  BL_INLINE BLResult strokeArc(double cx, double cy, double rx, double ry, double start, double sweep) noexcept { return strokeArc(BLArc(cx, cy, rx, ry, start, sweep)); }

  //! Strokes a chord.
  BL_INLINE BLResult strokeChord(const BLArc& chord) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_CHORD, &chord); }
  //! \overload
  BL_INLINE BLResult strokeChord(double cx, double cy, double r, double start, double sweep) noexcept { return strokeChord(BLArc(cx, cy, r, r, start, sweep)); }
  //! \overload
  BL_INLINE BLResult strokeChord(double cx, double cy, double rx, double ry, double start, double sweep) noexcept { return strokeChord(BLArc(cx, cy, rx, ry, start, sweep)); }

  //! Strokes a pie.
  BL_INLINE BLResult strokePie(const BLArc& pie) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_PIE, &pie); }
  //! \overload
  BL_INLINE BLResult strokePie(double cx, double cy, double r, double start, double sweep) noexcept { return strokePie(BLArc(cx, cy, r, r, start, sweep)); }
  //! \overload
  BL_INLINE BLResult strokePie(double cx, double cy, double rx, double ry, double start, double sweep) noexcept { return strokePie(BLArc(cx, cy, rx, ry, start, sweep)); }

  //! Strokes a triangle.
  BL_INLINE BLResult strokeTriangle(const BLTriangle& triangle) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_TRIANGLE, &triangle); }
  //! \overload
  BL_INLINE BLResult strokeTriangle(double x0, double y0, double x1, double y1, double x2, double y2) noexcept { return strokeTriangle(BLTriangle(x0, y0, x1, y1, x2, y2)); }

  //! Strokes a polyline.
  BL_INLINE BLResult strokePolyline(const BLArrayView<BLPoint>& poly) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_POLYLINED, &poly); }
  //! \overload
  BL_INLINE BLResult strokePolyline(const BLPoint* poly, size_t n) noexcept { return strokePolyline(BLArrayView<BLPoint>{poly, n}); }

  //! Strokes a polyline.
  BL_INLINE BLResult strokePolyline(const BLArrayView<BLPointI>& poly) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_POLYLINED, &poly); }
  //! \overload
  BL_INLINE BLResult strokePolyline(const BLPointI* poly, size_t n) noexcept { return strokePolyline(BLArrayView<BLPointI>{poly, n}); }

  //! Strokes a polygon.
  BL_INLINE BLResult strokePolygon(const BLArrayView<BLPoint>& poly) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_POLYGOND, &poly); }
  //! \overload
  BL_INLINE BLResult strokePolygon(const BLPoint* poly, size_t n) noexcept { return strokePolygon(BLArrayView<BLPoint>{poly, n}); }

  //! Strokes a polygon.
  BL_INLINE BLResult strokePolygon(const BLArrayView<BLPointI>& poly) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_POLYGONI, &poly); }
  //! \overload
  BL_INLINE BLResult strokePolygon(const BLPointI* poly, size_t n) noexcept { return strokePolygon(BLArrayView<BLPointI>{poly, n}); }

  //! Strokes an array of boxes.
  BL_INLINE BLResult strokeBoxArray(const BLArrayView<BLBox>& array) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXD, &array); }
  //! \overload
  BL_INLINE BLResult strokeBoxArray(const BLBox* data, size_t n) noexcept { return strokeBoxArray(BLArrayView<BLBox>{data, n}); }

  //! Strokes an array of boxes.
  BL_INLINE BLResult strokeBoxArray(const BLArrayView<BLBoxI>& array) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_BOXI, &array); }
  //! \overload
  BL_INLINE BLResult strokeBoxArray(const BLBoxI* data, size_t n) noexcept { return strokeBoxArray(BLArrayView<BLBoxI>{data, n}); }

  //! Strokes an array of rectangles.
  BL_INLINE BLResult strokeRectArray(const BLArrayView<BLRect>& array) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTD, &array); }
  //! \overload
  BL_INLINE BLResult strokeRectArray(const BLRect* data, size_t n) noexcept { return strokeRectArray(BLArrayView<BLRect>{data, n}); }

  //! Strokes an array of rectangles.
  BL_INLINE BLResult strokeRectArray(const BLArrayView<BLRectI>& array) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_ARRAY_VIEW_RECTI, &array); }
  //! \overload
  BL_INLINE BLResult strokeRectArray(const BLRectI* data, size_t n) noexcept { return strokeRectArray(BLArrayView<BLRectI>{data, n}); }

  //! Strokes a path.
  BL_INLINE BLResult strokePath(const BLPath& path) noexcept { return strokeGeometry(BL_GEOMETRY_TYPE_PATH, &path); }

  //! Strokes the passed UTF-8 text by using the given `font`.
  BL_INLINE BLResult strokeUtf8Text(const BLPointI& dst, const BLFont& font, const char* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->strokeTextI(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF8);
  }

  //! Strokes the passed UTF-8 text by using the given `font`.
  BL_INLINE BLResult strokeUtf8Text(const BLPoint& dst, const BLFont& font, const char* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->strokeTextD(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF8);
  }

  //! Strokes the passed UTF-16 text by using the given `font`.
  BL_INLINE BLResult strokeUtf16Text(const BLPointI& dst, const BLFont& font, const uint16_t* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->strokeTextI(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF16);
  }

  //! Strokes the passed UTF-16 text by using the given `font`.
  BL_INLINE BLResult strokeUtf16Text(const BLPoint& dst, const BLFont& font, const uint16_t* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->strokeTextD(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF16);
  }

  //! Strokes the passed UTF-32 text by using the given `font`.
  BL_INLINE BLResult strokeUtf32Text(const BLPointI& dst, const BLFont& font, const uint32_t* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->strokeTextI(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF32);
  }

  //! Strokes the passed UTF-32 text by using the given `font`.
  BL_INLINE BLResult strokeUtf32Text(const BLPoint& dst, const BLFont& font, const uint32_t* text, size_t size = SIZE_MAX) noexcept {
    return impl->virt->strokeTextD(impl, &dst, &font, text, size, BL_TEXT_ENCODING_UTF32);
  }

  //! Strokes the passed `glyphRun` by using the given `font`.
  BL_INLINE BLResult strokeGlyphRun(const BLPointI& dst, const BLFont& font, const BLGlyphRun& glyphRun) noexcept {
    return impl->virt->strokeGlyphRunI(impl, &dst, &font, &glyphRun);
  }

  //! Strokes the passed `glyphRun` by using the given `font`.
  BL_INLINE BLResult strokeGlyphRun(const BLPoint& dst, const BLFont& font, const BLGlyphRun& glyphRun) noexcept {
    return impl->virt->strokeGlyphRunD(impl, &dst, &font, &glyphRun);
  }

  //! \}

  //! \name Image Blitting
  //! \{

  //! Blits source image `src` at coordinates specified by `dst`..
  BL_INLINE BLResult blitImage(const BLPoint& dst, const BLImage& src) noexcept {
    return impl->virt->blitImageD(impl, &dst, &src, nullptr);
  }

  //! Blits an area of source image `src` specified by `srcArea` at coordinates specified by `dst`.
  BL_INLINE BLResult blitImage(const BLPoint& dst, const BLImage& src, const BLRectI& srcArea) noexcept {
    return impl->virt->blitImageD(impl, &dst, &src, &srcArea);
  }

  //! Blits source image `src` at coordinates specified by `dst`. (int coordinates).
  BL_INLINE BLResult blitImage(const BLPointI& dst, const BLImage& src) noexcept {
    return impl->virt->blitImageI(impl, &dst, &src, nullptr);
  }

  //! Blits an area in source image `src` specified by `srcArea` at coordinates specified by `dst`. (int coordinates).
  BL_INLINE BLResult blitImage(const BLPointI& dst, const BLImage& src, const BLRectI& srcArea) noexcept {
    return impl->virt->blitImageI(impl, &dst, &src, &srcArea);
  }

  //! Blits a source image `src` scaled to fit into `dst` rectangle.
  BL_INLINE BLResult blitImage(const BLRect& dst, const BLImage& src) noexcept {
    return impl->virt->blitScaledImageD(impl, &dst, &src, nullptr);
  }

  //! Blits an area of source image `src` specified by `srcArea` scaled to fit into `dst` rectangle.
  BL_INLINE BLResult blitImage(const BLRect& dst, const BLImage& src, const BLRectI& srcArea) noexcept {
    return impl->virt->blitScaledImageD(impl, &dst, &src, &srcArea);
  }

  //! Blits a source image `src` scaled to fit into `dst` rectangle (int coordinates).
  BL_INLINE BLResult blitImage(const BLRectI& dst, const BLImage& src) noexcept {
    return impl->virt->blitScaledImageI(impl, &dst, &src, nullptr);
  }

  //! Blits an area of source image `src` specified by `srcArea` scaled to fit into `dst` rectangle (int coordinates).
  BL_INLINE BLResult blitImage(const BLRectI& dst, const BLImage& src, const BLRectI& srcArea) noexcept {
    return impl->virt->blitScaledImageI(impl, &dst, &src, &srcArea);
  }

  //! \}

  static BL_INLINE const BLContext& none() noexcept {
    return reinterpret_cast<const BLContext*>(blNone)[kImplType];
  }
};
#endif

//! \}

#endif // BLEND2D_CONTEXT_H
