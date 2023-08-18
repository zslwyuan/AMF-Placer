// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_PIPEDEFS_P_H
#define BLEND2D_PIPEDEFS_P_H

#include "./api-internal_p.h"
#include "./format_p.h"
#include "./gradient_p.h"
#include "./matrix_p.h"
#include "./pattern_p.h"
#include "./runtime_p.h"
#include "./support_p.h"
#include "./tables_p.h"
#include "./simd_p.h"

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct BLPipeContextData;
struct BLPipeFillData;
struct BLPipeFetchData;
struct BLPipeSignature;

// ============================================================================
// [Constants]
// ============================================================================

//! Global constants used by pipeline and affecting also rasterizers.
enum BLPipeGlobalConsts : uint32_t {
  //! How many pixels are represented by a single bit of a `BLBitWord`.
  //!
  //! This is a hardcoded value as it's required by both rasterizer and compositor.
  //! Before establishing `4` the values [4, 8, 16, 32] were tested. Candidates
  //! were `4` and `8` where `8` sometimes surpassed `4` in specific workloads,
  //! but `4` was stable across all tests.
  //!
  //! In general increasing `BL_PIPE_PIXELS_PER_ONE_BIT` would result in less
  //! memory consumed by bit vectors, but would increase the work compositors
  //! have to do to process cells produced by analytic rasterizer.
  BL_PIPE_PIXELS_PER_ONE_BIT = 4
};

//! 8-bit alpha constants used by pipelines and affecting also rasterizers.
enum BLPipeA8Consts : uint32_t {
  BL_PIPE_A8_SHIFT = 8,                     // 8.
  BL_PIPE_A8_SCALE = 1 << BL_PIPE_A8_SHIFT, // 256.
  BL_PIPE_A8_MASK  = BL_PIPE_A8_SCALE - 1   // 255.
};

//! Pipeline extend modes (non-combined).
//!
//! Pipeline sees extend modes a bit differently in most cases.
enum BLPipeExtendMode : uint32_t {
  BL_PIPE_EXTEND_MODE_PAD         = 0,         //!< Pad, same as `BL_EXTEND_MODE_PAD`.
  BL_PIPE_EXTEND_MODE_REPEAT      = 1,         //!< Repeat, same as `BL_EXTEND_MODE_REPEAT`.
  BL_PIPE_EXTEND_MODE_REFLECT     = 2,         //!< Reflect, same as `BL_EXTEND_MODE_REFLECT`.
  BL_PIPE_EXTEND_MODE_ROR         = 3,         //!< Repeat-or-reflect (the same code-path for both cases).

  BL_PIPE_EXTEND_MODE_COUNT       = 4          //! Count of pipeline-specific extend modes.
};

//! Pipeline fill-type.
//!
//! A unique id describing how a mask of each filled pixel is calculated.
enum BLPipeFillType : uint32_t {
  BL_PIPE_FILL_TYPE_NONE          = 0,         //!< None or uninitialized.
  BL_PIPE_FILL_TYPE_BOX_A         = 1,         //!< Fill axis-aligned box.
  BL_PIPE_FILL_TYPE_BOX_U         = 2,         //!< Fill axis-unaligned box.
  BL_PIPE_FILL_TYPE_ANALYTIC      = 3,         //!< Fill analytic non-zero/even-odd.

  BL_PIPE_FILL_TYPE_COUNT         = 4          //!< Count of fill types.
};

//! Fill rule mask used during composition of mask produced by analytic-rasterizer.
//!
//! See blfillpart.cpp how this is used. What you see in these values is
//! mask shifted left by one bit as we expect such values in the pipeline.
enum BLPipeFillRuleMask : uint32_t {
  BL_PIPE_FILL_RULE_MASK_NON_ZERO = uint32_t(0xFFFFFFFFu << 1),
  BL_PIPE_FILL_RULE_MASK_EVEN_ODD = uint32_t(0x000001FFu << 1)
};

//! Pipeline fetch-type.
//!
//! A unique id describing how pixels are fetched - supported fetchers include
//! solid pixels, patterns (sometimes referred as blits), and gradients.
//!
//! \note RoR is a shurtcut for repeat-or-reflect - a universal fetcher for both.
enum BLPipeFetchType : uint32_t {
  BL_PIPE_FETCH_TYPE_SOLID  = 0,               //!< Solid fetch.

  BL_PIPE_FETCH_TYPE_PATTERN_AA_BLIT,          //!< Pattern {aligned} (blit) [Base].
  BL_PIPE_FETCH_TYPE_PATTERN_AA_PAD,           //!< Pattern {aligned} (pad-x) [Base].
  BL_PIPE_FETCH_TYPE_PATTERN_AA_REPEAT,        //!< Pattern {aligned} (repeat-large-x) [Optimized].
  BL_PIPE_FETCH_TYPE_PATTERN_AA_ROR,           //!< Pattern {aligned} (ror-x) [Base].
  BL_PIPE_FETCH_TYPE_PATTERN_FX_PAD,           //!< Pattern {frac-x} (pad-x) [Optimized].
  BL_PIPE_FETCH_TYPE_PATTERN_FX_ROR,           //!< Pattern {frac-x} (ror-x) [Optimized].
  BL_PIPE_FETCH_TYPE_PATTERN_FY_PAD,           //!< Pattern {frac-y} (pad-x) [Optimized].
  BL_PIPE_FETCH_TYPE_PATTERN_FY_ROR,           //!< Pattern {frac-x} (ror-x) [Optimized].
  BL_PIPE_FETCH_TYPE_PATTERN_FX_FY_PAD,        //!< Pattern {frac-xy} (pad-x) [Base].
  BL_PIPE_FETCH_TYPE_PATTERN_FX_FY_ROR,        //!< Pattern {frac-xy} (ror-x) [Base].
  BL_PIPE_FETCH_TYPE_PATTERN_AFFINE_NN_ANY,    //!< Pattern {affine-nearest}  (any) [Base].
  BL_PIPE_FETCH_TYPE_PATTERN_AFFINE_NN_OPT,    //!< Pattern {affine-nearest}  (any) [Optimized].
  BL_PIPE_FETCH_TYPE_PATTERN_AFFINE_BI_ANY,    //!< Pattern {affine-bilinear} (any) [Base].
  BL_PIPE_FETCH_TYPE_PATTERN_AFFINE_BI_OPT,    //!< Pattern {affine-bilinear} (any) [Optimized].

  BL_PIPE_FETCH_TYPE_GRADIENT_LINEAR_PAD,      //!< Linear gradient (pad) [Base].
  BL_PIPE_FETCH_TYPE_GRADIENT_LINEAR_ROR,      //!< Linear gradient (ror) [Base].
  BL_PIPE_FETCH_TYPE_GRADIENT_RADIAL_PAD,      //!< Radial gradient (pad) [Base].
  BL_PIPE_FETCH_TYPE_GRADIENT_RADIAL_REPEAT,   //!< Radial gradient (repeat) [Base].
  BL_PIPE_FETCH_TYPE_GRADIENT_RADIAL_REFLECT,  //!< Radial gradient (reflect) [Base].
  BL_PIPE_FETCH_TYPE_GRADIENT_CONICAL,         //!< Conical gradient (any) [Base].

  BL_PIPE_FETCH_TYPE_COUNT,                    //!< Number of fetch types.

  BL_PIPE_FETCH_TYPE_PIXEL_PTR = 0xFF,         //!< Pixel pointer (special value, not a valid fetch type).
  BL_PIPE_FETCH_TYPE_FAILURE = 0xFFFFFFFFu,    //!< Invalid fetch type (special value, signalizes error).

  BL_PIPE_FETCH_TYPE_PATTERN_ANY_FIRST         = BL_PIPE_FETCH_TYPE_PATTERN_AA_BLIT,
  BL_PIPE_FETCH_TYPE_PATTERN_ANY_LAST          = BL_PIPE_FETCH_TYPE_PATTERN_AFFINE_BI_OPT,

  BL_PIPE_FETCH_TYPE_PATTERN_AA_FIRST          = BL_PIPE_FETCH_TYPE_PATTERN_AA_BLIT,
  BL_PIPE_FETCH_TYPE_PATTERN_AA_LAST           = BL_PIPE_FETCH_TYPE_PATTERN_AA_ROR,

  BL_PIPE_FETCH_TYPE_PATTERN_AU_FIRST          = BL_PIPE_FETCH_TYPE_PATTERN_FX_PAD,
  BL_PIPE_FETCH_TYPE_PATTERN_AU_LAST           = BL_PIPE_FETCH_TYPE_PATTERN_FX_FY_ROR,

  BL_PIPE_FETCH_TYPE_PATTERN_FX_FIRST          = BL_PIPE_FETCH_TYPE_PATTERN_FX_PAD,
  BL_PIPE_FETCH_TYPE_PATTERN_FX_LAST           = BL_PIPE_FETCH_TYPE_PATTERN_FX_ROR,

  BL_PIPE_FETCH_TYPE_PATTERN_FY_FIRST          = BL_PIPE_FETCH_TYPE_PATTERN_FY_PAD,
  BL_PIPE_FETCH_TYPE_PATTERN_FY_LAST           = BL_PIPE_FETCH_TYPE_PATTERN_FY_ROR,

  BL_PIPE_FETCH_TYPE_PATTERN_FX_FY_FIRST       = BL_PIPE_FETCH_TYPE_PATTERN_FX_FY_PAD,
  BL_PIPE_FETCH_TYPE_PATTERN_FX_FY_LAST        = BL_PIPE_FETCH_TYPE_PATTERN_FX_FY_ROR,

  BL_PIPE_FETCH_TYPE_PATTERN_SIMPLE_FIRST      = BL_PIPE_FETCH_TYPE_PATTERN_AA_BLIT,
  BL_PIPE_FETCH_TYPE_PATTERN_SIMPLE_LAST       = BL_PIPE_FETCH_TYPE_PATTERN_FX_FY_ROR,

  BL_PIPE_FETCH_TYPE_PATTERN_AFFINE_FIRST      = BL_PIPE_FETCH_TYPE_PATTERN_AFFINE_NN_ANY,
  BL_PIPE_FETCH_TYPE_PATTERN_AFFINE_LAST       = BL_PIPE_FETCH_TYPE_PATTERN_AFFINE_BI_OPT,

  BL_PIPE_FETCH_TYPE_GRADIENT_ANY_FIRST        = BL_PIPE_FETCH_TYPE_GRADIENT_LINEAR_PAD,
  BL_PIPE_FETCH_TYPE_GRADIENT_ANY_LAST         = BL_PIPE_FETCH_TYPE_GRADIENT_CONICAL,

  BL_PIPE_FETCH_TYPE_GRADIENT_LINEAR_FIRST     = BL_PIPE_FETCH_TYPE_GRADIENT_LINEAR_PAD,
  BL_PIPE_FETCH_TYPE_GRADIENT_LINEAR_LAST      = BL_PIPE_FETCH_TYPE_GRADIENT_LINEAR_ROR,

  BL_PIPE_FETCH_TYPE_GRADIENT_RADIAL_FIRST     = BL_PIPE_FETCH_TYPE_GRADIENT_RADIAL_PAD,
  BL_PIPE_FETCH_TYPE_GRADIENT_RADIAL_LAST      = BL_PIPE_FETCH_TYPE_GRADIENT_RADIAL_REFLECT,

  BL_PIPE_FETCH_TYPE_GRADIENT_CONICAL_FIRST    = BL_PIPE_FETCH_TYPE_GRADIENT_CONICAL,
  BL_PIPE_FETCH_TYPE_GRADIENT_CONICAL_LAST     = BL_PIPE_FETCH_TYPE_GRADIENT_CONICAL
};

//! Masks used by `BLPipeSignature`.
//!
//! Each mask represents one value in a signature. Each value describes a part
//! in a signature like format, composition operator, etc. All parts packed
//! together form a 32-bit integer that can be used to uniquely describe the
//! whole pipeline and can act as a key or hash-code in pipeline function caches.
enum BLPipeSignatureMasks : uint32_t {
  BL_PIPE_SIGNATURE_DST_FORMAT    = 0x0000000Fu <<  0, // [00..03] {16 values}
  BL_PIPE_SIGNATURE_SRC_FORMAT    = 0x0000000Fu <<  4, // [04..07] {16 values}
  BL_PIPE_SIGNATURE_COMP_OP       = 0x0000003Fu <<  8, // [08..13] {64 values}
  BL_PIPE_SIGNATURE_FILL_TYPE     = 0x00000003u << 14, // [14..15] {4 values}
  BL_PIPE_SIGNATURE_FETCH_TYPE    = 0x0000001Fu << 16, // [16..20] {32 values}
  BL_PIPE_SIGNATURE_FETCH_PAYLOAD = 0x000007FFu << 21  // [21..31] {2048 values}
};

// ============================================================================
// [Typedefs]
// ============================================================================

typedef void (BL_CDECL* BLPipeFillFunc)(void* ctxData, const void* fillData, const void* fetchData) BL_NOEXCEPT;

// ============================================================================
// [BLPipeValue32]
// ============================================================================

union BLPipeValue32 {
  uint32_t u;
  int32_t i;
  float f;
};

// ============================================================================
// [BLPipeValue64]
// ============================================================================

union BLPipeValue64 {
  uint64_t u64;
  int64_t i64;
  double d;

  int32_t i32[2];
  uint32_t u32[2];

  int16_t i16[4];
  uint16_t u16[4];

#if BL_BYTE_ORDER == 1234 // LITTLE ENDIAN
  struct { int32_t  i32Lo, i32Hi; };
  struct { uint32_t u32Lo, u32Hi; };
#else
  struct { int32_t  i32Hi, i32Lo; };
  struct { uint32_t u32Hi, u32Lo; };
#endif

  BL_INLINE void expandLoToHi() noexcept { u32Hi = u32Lo; }
};

// ============================================================================
// [BLPipeContextData]
// ============================================================================

struct BLPipeContextData {
  BLImageData dst;

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
};

// ============================================================================
// [BLPipeFillData]
// ============================================================================

struct BLPipeFillData {
  struct Common {
    //! Rectangle to fill.
    BLBoxI box;
    //! Alpha value (range depends on target pixel format).
    BLPipeValue32 alpha;
  };

  //! Rectangle (axis-aligned).
  struct BoxA {
    //! Rectangle to fill.
    BLBoxI box;
    //! Alpha value (range depends on target pixel format).
    BLPipeValue32 alpha;
  };

  //! Rectangle (axis-unaligned).
  struct BoxU {
    //! Rectangle to fill.
    BLBoxI box;
    //! Alpha value (range depends on target pixel format).
    BLPipeValue32 alpha;

    //! Masks of top, middle and bottom part of the rect.
    //!
    //! \note The last value `masks[3]` must be zero as it's a sentinel for the pipeline.
    uint32_t masks[4];
    //! Height of the middle (1) and last (2) masks.
    uint32_t heights[2];
    //! Start width (from 1 to 3).
    uint32_t startWidth;
    //! Inner width (from 0 to width).
    uint32_t innerWidth;
  };

  struct Analytic {
    //! Fill boundary.
    BLBoxI box;
    //! Alpha value (range depends on format).
    BLPipeValue32 alpha;
    //! All ones if NonZero or 0x01FF if EvenOdd.
    uint32_t fillRuleMask;

    //! Shadow bit-buffer (marks a group of cells which are non-zero).
    BLBitWord* bitTopPtr;
    //! Bit-buffer stride (in bytes).
    size_t bitStride;

    //! Cell buffer.
    uint32_t* cellTopPtr;
    //! Cell stride (in bytes).
    size_t cellStride;
  };

  union {
    Common common;
    BoxA boxAA;
    BoxU boxAU;
    Analytic analytic;
  };

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }

  // --------------------------------------------------------------------------
  // [Init]
  // --------------------------------------------------------------------------

  BL_INLINE bool initBoxA8bpc(uint32_t alpha, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1) noexcept {
    // The rendering engine should never pass out-of-range alpha.
    BL_ASSERT(alpha <= 255);

    // The rendering engine should never pass invalid box to the pipeline.
    BL_ASSERT(x0 < x1);
    BL_ASSERT(y0 < y1);

    boxAA.alpha.u = alpha;
    boxAA.box.reset(int(x0), int(y0), int(x1), int(y1));

    return true;
  }

  template<typename T>
  BL_INLINE bool initBoxU8bpcT(uint32_t alpha, T x0, T y0, T x1, T y1) noexcept {
    return initBoxU8bpc24x8(alpha, uint32_t(blTruncToInt(x0 * T(256))),
                                   uint32_t(blTruncToInt(y0 * T(256))),
                                   uint32_t(blTruncToInt(x1 * T(256))),
                                   uint32_t(blTruncToInt(y1 * T(256))));
  }

  bool initBoxU8bpc24x8(uint32_t alpha, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1) noexcept {
    // The rendering engine should never pass out-of-range alpha.
    BL_ASSERT(alpha <= 255);

    // The rendering engine should never pass invalid box to the pipeline.
    BL_ASSERT(x0 < x1);
    BL_ASSERT(y0 < y1);

    uint32_t ax0 = x0 >> 8;
    uint32_t ay0 = y0 >> 8;
    uint32_t ax1 = x1 >> 8;
    uint32_t ay1 = y1 >> 8;

    boxAU.alpha.u = alpha;
    boxAU.box.reset(int(ax0), int(ay0), int(ax1), int(ay1));

    uint32_t fx0 = x0 & 0xFFu;
    uint32_t fy0 = y0 & 0xFFu;
    uint32_t fx1 = x1 & 0xFFu;
    uint32_t fy1 = y1 & 0xFFu;

    boxAU.box.x1 += fx1 != 0;
    boxAU.box.y1 += fy1 != 0;

    if (!fx1) fx1 = 256;
    if (!fy1) fy1 = 256;

    if (((x0 ^ x1) >> 8) == 0) { fx0 = fx1 - fx0; fx1 = 0; } else { fx0 = 256 - fx0; }
    if (((y0 ^ y1) >> 8) == 0) { fy0 = fy1 - fy0; fy1 = 0; } else { fy0 = 256 - fy0; }

    uint32_t fy0_a = fy0 * alpha;
    uint32_t fy1_a = fy1 * alpha;

    uint32_t m0 = (fx1 * fy0_a) >> 16;
    uint32_t m1 = (fx1 * alpha) >>  8;
    uint32_t m2 = (fx1 * fy0_a) >> 16;

    uint32_t iw = uint32_t(boxAU.box.x1 - boxAU.box.x0);
    if (iw > 2) {
      m0 = (m0 << 8) | (fy0_a >> 8);
      m1 = (m1 << 8) | alpha;
      m2 = (m2 << 8) | (fy1_a >> 8);
    }

    if (iw > 1) {
      m0 = (m0 << 8) | ((fx0 * fy0_a) >> 16);
      m1 = (m1 << 8) | ((fx0 * alpha) >>  8);
      m2 = (m2 << 8) | ((fx0 * fy1_a) >> 16);
    }

    if (!m1)
      return false;

    // Border case - if alpha is too low it can cause `m0` or `m2` to be zero,
    // which would then confuse the pipeline as it would think to stop instead
    // of jumping to 'CMask' loop. So we patch `m0`
    if (!m0) {
      m0 = m1;
      boxAU.box.y0++;
      if (boxAU.box.y0 == boxAU.box.y1)
        return false;
    }

    uint32_t ih = uint32_t(boxAU.box.y1 - boxAU.box.y0);

    boxAU.masks[0] = m0;
    boxAU.masks[1] = m1;
    boxAU.masks[2] = m2;
    boxAU.masks[3] = 0;
    boxAU.heights[0] = ih - 2;
    boxAU.heights[1] = 1;

    // There is no middle layer (m1) if the height is 2 pixels or less.
    if (ih <= 2) {
      boxAU.masks[1] = boxAU.masks[2];
      boxAU.masks[2] = 0;
      boxAU.heights[0] = ih - 1;
      boxAU.heights[1] = 0;
    }

    if (ih <= 1) {
      boxAU.masks[1] = 0;
      boxAU.heights[0] = 0;
    }

    if (iw > 3) {
      boxAU.startWidth = 1;
      boxAU.innerWidth = iw - 2;
    }
    else {
      boxAU.startWidth = iw;
      boxAU.innerWidth = 0;
    }

    return true;
  }

  BL_INLINE bool initAnalytic(uint32_t alpha, uint32_t fillRule, BLBitWord* bitTopPtr, size_t bitStride, uint32_t* cellTopPtr, size_t cellStride) noexcept {
    analytic.alpha.u = alpha;
    analytic.fillRuleMask =
      (fillRule == BL_FILL_RULE_NON_ZERO)
        ? BL_PIPE_FILL_RULE_MASK_NON_ZERO
        : BL_PIPE_FILL_RULE_MASK_EVEN_ODD;
    analytic.bitTopPtr = bitTopPtr;
    analytic.bitStride = bitStride;
    analytic.cellTopPtr = cellTopPtr;
    analytic.cellStride = cellStride;

    return true;
  }
};

// ============================================================================
// [BLPipeFetchData]
// ============================================================================

//! Blend2D pipeline fetch data.
struct alignas(16) BLPipeFetchData {
  //! Solid fetch data.
  struct Solid {
    union {
      struct {
        //! 32-bit ARGB, premultiplied.
        uint32_t prgb32;
        //! Reserved in case 32-bit data is used.
        uint32_t reserved32;
      };
      //! 64-bit ARGB, premultiplied.
      uint64_t prgb64;
    };

    BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
  };

  //! Pattern fetch data.
  struct alignas(16) Pattern {
    //! Source image data.
    struct SourceData {
      const uint8_t* pixelData;
      intptr_t stride;
      BLSizeI size;
    };

    //! Simple pattern data (only identity or translation matrix).
    struct alignas(16) Simple {
      //! Translate by x/y (inverted).
      int32_t tx, ty;
      //! Repeat/Reflect w/h.
      int32_t rx, ry;
      //! Safe X increments by 1..16 (fetchN).
      BLModuloTable ix;
      //! 9-bit or 17-bit weight at [0, 0] (A).
      uint32_t wa;
      //! 9-bit or 17-bit weight at [1, 0] (B).
      uint32_t wb;
      //! 9-bit or 17-bit weight at [0, 1] (C).
      uint32_t wc;
      //! 9-bit or 17-bit weight at [1, 1] (D).
      uint32_t wd;
    };

    //! Affine pattern data.
    struct alignas(16) Affine {
      //! Single X/Y step in X direction.
      BLPipeValue64 xx, xy;
      //! Single X/Y step in Y direction.
      BLPipeValue64 yx, yy;
      //! Pattern offset at [0, 0].
      BLPipeValue64 tx, ty;
      //! Pattern overflow check.
      BLPipeValue64 ox, oy;
      //! Pattern overflow correction (repeat/reflect).
      BLPipeValue64 rx, ry;
      //! Two X/Y steps in X direction, used by `fetch4()`.
      BLPipeValue64 xx2, xy2;
      //! Pattern padding minimum (0 for PAD, INT32_MIN for other modes).
      int32_t minX, minY;
      //! Pattern padding maximum (width-1 and height-1).
      int32_t maxX, maxY;
      //! Correction X/Y values in case that maxX/maxY was exceeded (PAD, BILINEAR)
      int32_t corX, corY;
      //! Repeated tile width/height (doubled if reflected).
      double tw, th;

      //! 32-bit value to be used by [V]PMADDWD instruction to calculate address from Y/X pairs.
      int16_t addrMul[2];
    };

    //! Source image data.
    SourceData src;

    union {
      //! Simple pattern data.
      Simple simple;
      //! Affine pattern data.
      Affine affine;
    };

    BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
  };

  //! Gradient fetch data.
  struct alignas(16) Gradient {
    //! Precomputed lookup table, used by all gradient fetchers.
    struct LUT {
      //! Pixel data, array of either 32-bit or 64-bit pixels.
      const void* data;
      //! Number of pixels stored in `data`, must be a power of 2.
      uint32_t size;
    };

    //! Linear gradient data.
    struct alignas(16) Linear {
      //! Gradient offset of the pixel at [0, 0].
      BLPipeValue64 pt[2];
      //! One Y step.
      BLPipeValue64 dy;
      //! One X step.
      BLPipeValue64 dt;
      //! Two X steps.
      BLPipeValue64 dt2;
      //! Reflect/Repeat mask (repeated/reflected size - 1).
      BLPipeValue64 rep;
      //! Size mask (gradient size - 1).
      BLPipeValue32 msk;
    };

    //! Radial gradient data.
    struct alignas(16) Radial {
      //! Gradient X/Y increments (horizontal).
      double xx, xy;
      //! Gradient X/Y increments (vertical).
      double yx, yy;
      //! Gradient X/Y offsets of the pixel at [0, 0].
      double ox, oy;

      double ax, ay;
      double fx, fy;

      double dd, bd;
      double ddx, ddy;
      double ddd, scale;

      int maxi;
    };

    //! Conical gradient data.
    struct alignas(16) Conical {
      //! Gradient X/Y increments (horizontal).
      double xx, xy;
      //! Gradient X/Y increments (vertical).
      double yx, yy;
      //! Gradient X/Y offsets of the pixel at [0, 0].
      double ox, oy;
      //! Atan2 approximation constants.
      const BLCommonTable::Conical* consts;

      int maxi;
    };

    //! Precomputed lookup table.
    LUT lut;
    //! Union of all possible gradient data types.
    union {
      //! Linear gradient specific data.
      Linear linear;
      //! Radial gradient specific data.
      Radial radial;
      //! Conical gradient specific data.
      Conical conical;
    };

    BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
  };

  //! Union of all possible fetch data types.
  union {
    //! Solid fetch data.
    Solid solid;
    //! Pattern fetch data.
    Pattern pattern;
    //! Gradient fetch data.
    Gradient gradient;
  };

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }

  BL_INLINE void initPatternSource(const uint8_t* pixelData, intptr_t stride, int w, int h) noexcept {
    pattern.src.pixelData = pixelData;
    pattern.src.stride = stride;
    pattern.src.size.reset(w, h);
  }

  BL_INLINE uint32_t initPatternBlit(int x, int y) noexcept {
    pattern.simple.tx = x;
    pattern.simple.ty = y;
    pattern.simple.rx = 0;
    pattern.simple.ry = 0;
    return BL_PIPE_FETCH_TYPE_PATTERN_AA_BLIT;
  }

  BL_HIDDEN uint32_t initPatternAxAy(
    uint32_t extendMode,
    int x, int y) noexcept;

  BL_HIDDEN uint32_t initPatternFxFy(
    uint32_t extendMode,
    uint32_t filter,
    uint32_t bytesPerPixel,
    int64_t tx64, int64_t ty64) noexcept;

  BL_HIDDEN uint32_t initPatternAffine(
    uint32_t extendMode,
    uint32_t filter,
    uint32_t bytesPerPixel,
    const BLMatrix2D& m) noexcept;

  BL_HIDDEN uint32_t initGradient(
    uint32_t gradientType,
    const void* values,
    uint32_t extendMode,
    const BLGradientLUT* lut,
    const BLMatrix2D& m) noexcept;
};

// ============================================================================
// [BLPipeSignature]
// ============================================================================

//! Pipeline signature packed to a single `uint32_t` value.
//!
//! Can be used to build signatures as well as it offers the required functionality.
struct BLPipeSignature {
  //! Signature as a 32-bit value.
  uint32_t value;

  BL_INLINE BLPipeSignature() noexcept = default;
  BL_INLINE constexpr BLPipeSignature(const BLPipeSignature&) noexcept = default;
  BL_INLINE explicit constexpr BLPipeSignature(uint32_t value) : value(value) {}

  BL_INLINE bool operator==(const BLPipeSignature& other) const noexcept { return value == other.value; }
  BL_INLINE bool operator!=(const BLPipeSignature& other) const noexcept { return value != other.value; }

  BL_INLINE uint32_t _get(uint32_t mask) const noexcept {
    return (this->value & mask) >> blBitShiftOf(mask);
  }

  BL_INLINE void _set(uint32_t mask, uint32_t v) noexcept {
    BL_ASSERT(v <= (mask >> blBitShiftOf(mask)));
    this->value = (this->value & ~mask) | (v << blBitShiftOf(mask));
  }

  BL_INLINE void _add(uint32_t mask, uint32_t v) noexcept {
    BL_ASSERT(v <= (mask >> blBitShiftOf(mask)));
    this->value |= (v << blBitShiftOf(mask));
  }

  //! Reset all values to zero.
  BL_INLINE void reset() noexcept { this->value = 0; }
  //! Reset all values to `v`.
  BL_INLINE void reset(uint32_t v) noexcept { this->value = v; }
  //! Reset all values to the `other` signature.
  BL_INLINE void reset(const BLPipeSignature& other) noexcept { this->value = other.value; }

  //! Set the signature from a packed 32-bit integer.
  BL_INLINE void setValue(uint32_t v) noexcept { this->value = v; }
  //! Set the signature from another `BLPipeSignature`.
  BL_INLINE void setValue(const BLPipeSignature& other) noexcept { this->value = other.value; }

  //! Extracts destination pixel format from the signature.
  BL_INLINE uint32_t dstFormat() const noexcept { return _get(BL_PIPE_SIGNATURE_DST_FORMAT); }
  //! Extracts source pixel format from the signature.
  BL_INLINE uint32_t srcFormat() const noexcept { return _get(BL_PIPE_SIGNATURE_SRC_FORMAT); }
  //! Extracts composition operator from the signature.
  BL_INLINE uint32_t compOp() const noexcept { return _get(BL_PIPE_SIGNATURE_COMP_OP); }
  //! Extracts sweep type from the signature.
  BL_INLINE uint32_t fillType() const noexcept { return _get(BL_PIPE_SIGNATURE_FILL_TYPE); }
  //! Extracts fetch type from the signature.
  BL_INLINE uint32_t fetchType() const noexcept { return _get(BL_PIPE_SIGNATURE_FETCH_TYPE); }
  //! Extracts fetch data from the signature.
  BL_INLINE uint32_t fetchPayload() const noexcept { return _get(BL_PIPE_SIGNATURE_FETCH_PAYLOAD); }

  //! Add destination pixel format.
  BL_INLINE void setDstFormat(uint32_t v) noexcept { _set(BL_PIPE_SIGNATURE_DST_FORMAT, v); }
  //! Add source pixel format.
  BL_INLINE void setSrcFormat(uint32_t v) noexcept { _set(BL_PIPE_SIGNATURE_SRC_FORMAT, v); }
  //! Add clip mode.
  BL_INLINE void setCompOp(uint32_t v) noexcept { _set(BL_PIPE_SIGNATURE_COMP_OP, v); }
  //! Add sweep type.
  BL_INLINE void setFillType(uint32_t v) noexcept { _set(BL_PIPE_SIGNATURE_FILL_TYPE, v); }
  //! Add fetch type.
  BL_INLINE void setFetchType(uint32_t v) noexcept { _set(BL_PIPE_SIGNATURE_FETCH_TYPE, v); }
  //! Add fetch data.
  BL_INLINE void setFetchPayload(uint32_t v) noexcept { _set(BL_PIPE_SIGNATURE_FETCH_PAYLOAD, v); }

  // The following methods are used to build the signature. They use '|' operator
  // which doesn't clear the previous value, each function is expected to be called
  // only once when building a new signature.

  //! Combine with other signature.
  BL_INLINE void add(uint32_t v) noexcept { this->value |= v; }
  //! Combine with other signature.
  BL_INLINE void add(const BLPipeSignature& other) noexcept { this->value |= other.value; }

  //! Add destination pixel format.
  BL_INLINE void addDstFormat(uint32_t v) noexcept { _add(BL_PIPE_SIGNATURE_DST_FORMAT, v); }
  //! Add source pixel format.
  BL_INLINE void addSrcFormat(uint32_t v) noexcept { _add(BL_PIPE_SIGNATURE_SRC_FORMAT, v); }
  //! Add clip mode.
  BL_INLINE void addCompOp(uint32_t v) noexcept { _add(BL_PIPE_SIGNATURE_COMP_OP, v); }
  //! Add sweep type.
  BL_INLINE void addFillType(uint32_t v) noexcept { _add(BL_PIPE_SIGNATURE_FILL_TYPE, v); }
  //! Add fetch type.
  BL_INLINE void addFetchType(uint32_t v) noexcept { _add(BL_PIPE_SIGNATURE_FETCH_TYPE, v); }
  //! Add fetch data.
  BL_INLINE void addFetchPayload(uint32_t v) noexcept { _add(BL_PIPE_SIGNATURE_FETCH_PAYLOAD, v); }
};

//! \}
//! \endcond

#endif // BLEND2D_PIPEDEFS_P_H
