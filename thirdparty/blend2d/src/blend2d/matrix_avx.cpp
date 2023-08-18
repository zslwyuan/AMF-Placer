// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include "./api-build_p.h"
#ifdef BL_TARGET_OPT_AVX

#include "./geometry.h"
#include "./matrix_p.h"
#include "./runtime_p.h"
#include "./simd_p.h"
#include "./support_p.h"

// ============================================================================
// [BLMatrix2D - MapPointDArray [AVX]]
// ============================================================================

static BLResult BL_CDECL blMatrix2DMapPointDArrayIdentity_AVX(const BLMatrix2D* self, BLPoint* dst, const BLPoint* src, size_t size) noexcept {
  using namespace SIMD;

  BL_UNUSED(self);
  if (dst == src)
    return BL_SUCCESS;

  size_t i = size;
  while (i >= 8) {
    v_storeu_d256(dst + 0, v_loadu_d256(src + 0));
    v_storeu_d256(dst + 2, v_loadu_d256(src + 2));
    v_storeu_d256(dst + 4, v_loadu_d256(src + 4));
    v_storeu_d256(dst + 6, v_loadu_d256(src + 6));

    i -= 8;
    dst += 8;
    src += 8;
  }

  while (i >= 2) {
    v_storeu_d256(dst, v_loadu_d256(src));

    i -= 2;
    dst += 2;
    src += 2;
  }

  if (i)
    v_storeu_d128(dst, v_loadu_d128(src));

  return BL_SUCCESS;
}

static BLResult BL_CDECL blMatrix2DMapPointDArrayTranslate_AVX(const BLMatrix2D* self, BLPoint* dst, const BLPoint* src, size_t size) noexcept {
  using namespace SIMD;

  size_t i = size;
  Vec256D m20_m21 = v_broadcast_d256_128(&self->m20);

  while (i >= 8) {
    v_storeu_d256(dst + 0, v_add_f64(v_loadu_d256(src + 0), m20_m21));
    v_storeu_d256(dst + 2, v_add_f64(v_loadu_d256(src + 2), m20_m21));
    v_storeu_d256(dst + 4, v_add_f64(v_loadu_d256(src + 4), m20_m21));
    v_storeu_d256(dst + 6, v_add_f64(v_loadu_d256(src + 6), m20_m21));

    i -= 8;
    dst += 8;
    src += 8;
  }

  while (i >= 2) {
    v_storeu_d256(dst, v_add_f64(v_loadu_d256(src), m20_m21));

    i -= 2;
    dst += 2;
    src += 2;
  }

  if (i)
    v_storeu_d128(dst, v_add_f64(v_loadu_d128(src), v_cast<Vec128D>(m20_m21)));

  return BL_SUCCESS;
}

static BLResult BL_CDECL blMatrix2DMapPointDArrayScale_AVX(const BLMatrix2D* self, BLPoint* dst, const BLPoint* src, size_t size) noexcept {
  using namespace SIMD;

  size_t i = size;
  Vec256D m00_m11 = v_dupl_d128(v_fill_d128(self->m11, self->m00));
  Vec256D m20_m21 = v_broadcast_d256_128(&self->m20);

  while (i >= 8) {
    v_storeu_d256(dst + 0, v_add_f64(v_mul_f64(v_loadu_d256(src + 0), m00_m11), m20_m21));
    v_storeu_d256(dst + 2, v_add_f64(v_mul_f64(v_loadu_d256(src + 2), m00_m11), m20_m21));
    v_storeu_d256(dst + 4, v_add_f64(v_mul_f64(v_loadu_d256(src + 4), m00_m11), m20_m21));
    v_storeu_d256(dst + 6, v_add_f64(v_mul_f64(v_loadu_d256(src + 6), m00_m11), m20_m21));

    i -= 8;
    dst += 8;
    src += 8;
  }

  while (i >= 2) {
    v_storeu_d256(dst, v_add_f64(v_mul_f64(v_loadu_d256(src), m00_m11), m20_m21));

    i -= 2;
    dst += 2;
    src += 2;
  }

  if (i)
    v_storeu_d128(dst, v_add_f64(v_mul_f64(v_loadu_d128(src), v_cast<Vec128D>(m00_m11)), v_cast<Vec128D>(m20_m21)));

  return BL_SUCCESS;
}

static BLResult BL_CDECL blMatrix2DMapPointDArraySwap_AVX(const BLMatrix2D* self, BLPoint* dst, const BLPoint* src, size_t size) noexcept {
  using namespace SIMD;

  size_t i = size;
  Vec256D m01_m10 = v_dupl_d128(v_fill_d128(self->m01, self->m10));
  Vec256D m20_m21 = v_broadcast_d256_128(&self->m20);

  while (i >= 8) {
    v_storeu_d256(dst + 0, v_add_f64(v_mul_f64(v_swap_f64(v_loadu_d256(src + 0)), m01_m10), m20_m21));
    v_storeu_d256(dst + 2, v_add_f64(v_mul_f64(v_swap_f64(v_loadu_d256(src + 2)), m01_m10), m20_m21));
    v_storeu_d256(dst + 4, v_add_f64(v_mul_f64(v_swap_f64(v_loadu_d256(src + 4)), m01_m10), m20_m21));
    v_storeu_d256(dst + 6, v_add_f64(v_mul_f64(v_swap_f64(v_loadu_d256(src + 6)), m01_m10), m20_m21));

    i -= 8;
    dst += 8;
    src += 8;
  }

  while (i >= 2) {
    v_storeu_d256(dst, v_add_f64(v_mul_f64(v_swap_f64(v_loadu_d256(src)), m01_m10), m20_m21));

    i -= 2;
    dst += 2;
    src += 2;
  }

  if (i)
    v_storeu_d128(dst, v_add_f64(v_mul_f64(v_swap_f64(v_loadu_d128(src)), v_cast<Vec128D>(m01_m10)), v_cast<Vec128D>(m20_m21)));

  return BL_SUCCESS;
}

static BLResult BL_CDECL blMatrix2DMapPointDArrayAffine_AVX(const BLMatrix2D* self, BLPoint* dst, const BLPoint* src, size_t size) noexcept {
  using namespace SIMD;

  size_t i = size;
  Vec256D m00_m11 = v_dupl_d128(v_fill_d128(self->m11, self->m00));
  Vec256D m10_m01 = v_dupl_d128(v_fill_d128(self->m01, self->m10));
  Vec256D m20_m21 = v_broadcast_d256_128(&self->m20);

  while (i >= 8) {
    Vec256D s0 = v_loadu_d256(src + 0);
    Vec256D s1 = v_loadu_d256(src + 2);
    Vec256D s2 = v_loadu_d256(src + 4);
    Vec256D s3 = v_loadu_d256(src + 6);

    v_storeu_d256(dst + 0, v_add_f64(v_add_f64(v_mul_f64(s0, m00_m11), m20_m21), v_mul_f64(v_swap_f64(s0), m10_m01)));
    v_storeu_d256(dst + 2, v_add_f64(v_add_f64(v_mul_f64(s1, m00_m11), m20_m21), v_mul_f64(v_swap_f64(s1), m10_m01)));
    v_storeu_d256(dst + 4, v_add_f64(v_add_f64(v_mul_f64(s2, m00_m11), m20_m21), v_mul_f64(v_swap_f64(s2), m10_m01)));
    v_storeu_d256(dst + 6, v_add_f64(v_add_f64(v_mul_f64(s3, m00_m11), m20_m21), v_mul_f64(v_swap_f64(s3), m10_m01)));

    i -= 8;
    dst += 8;
    src += 8;
  }

  while (i >= 2) {
    Vec256D s0 = v_loadu_d256(src);
    v_storeu_d256(dst, v_add_f64(v_add_f64(v_mul_f64(s0, m00_m11), m20_m21), v_mul_f64(v_swap_f64(s0), m10_m01)));

    i -= 2;
    dst += 2;
    src += 2;
  }

  if (i) {
    Vec128D s0 = v_loadu_d128(src);
    v_storeu_d128(dst, v_add_f64(v_add_f64(v_mul_f64(s0, v_cast<Vec128D>(m00_m11)), v_cast<Vec128D>(m20_m21)), v_mul_f64(v_swap_f64(s0), v_cast<Vec128D>(m10_m01))));
  }

  return BL_SUCCESS;
}

// ============================================================================
// [BLMatrix2D - Runtime Init [AVX]]
// ============================================================================

void blMatrix2DRtInit_AVX(BLRuntimeContext* rt) noexcept {
  BL_UNUSED(rt);
  BLMapPointDArrayFunc* funcs = blMatrix2DMapPointDArrayFuncs;

  blAssignFunc(&funcs[BL_MATRIX2D_TYPE_IDENTITY ], blMatrix2DMapPointDArrayIdentity_AVX);
  blAssignFunc(&funcs[BL_MATRIX2D_TYPE_TRANSLATE], blMatrix2DMapPointDArrayTranslate_AVX);
  blAssignFunc(&funcs[BL_MATRIX2D_TYPE_SCALE    ], blMatrix2DMapPointDArrayScale_AVX);
  blAssignFunc(&funcs[BL_MATRIX2D_TYPE_SWAP     ], blMatrix2DMapPointDArraySwap_AVX);
  blAssignFunc(&funcs[BL_MATRIX2D_TYPE_AFFINE   ], blMatrix2DMapPointDArrayAffine_AVX);
  blAssignFunc(&funcs[BL_MATRIX2D_TYPE_INVALID  ], blMatrix2DMapPointDArrayAffine_AVX);
}

#endif
