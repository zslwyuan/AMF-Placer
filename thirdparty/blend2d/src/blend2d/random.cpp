// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include "./api-build_p.h"
#include "./random_p.h"
#include "./simd_p.h"

// ============================================================================
// [BLRandom - Core]
// ============================================================================

BLResult blRandomReset(BLRandom* self, uint64_t seed) noexcept {
  blRandomResetInline(self, seed);
  return BL_SUCCESS;
}

double blRandomNextDouble(BLRandom* self) noexcept {
  return blRandomNextDoubleInline(self);
}

uint32_t blRandomNextUInt32(BLRandom* self) noexcept {
  return blRandomNextUInt32Inline(self);
}

uint64_t blRandomNextUInt64(BLRandom* self) noexcept {
  return blRandomNextUInt64Inline(self);
}

// ============================================================================
// [BLRandom - Unit Tests]
// ============================================================================

#ifdef BL_TEST
UNIT(random) {
  // Number of iterations for tests that use loop.
  enum { kCount = 1000000 };

  // Test whether the SIMD implementation matches the C++ one.
  #if defined(BL_TARGET_OPT_SSE2)
  {
    BLRandom a(0);
    BLRandom b(0);

    SIMD::Vec128I bLo = blRandomNextUInt64AsI128Inline(&b);
    SIMD::Vec128I bHi = SIMD::v_swizzle_i32<2, 3, 0, 1>(bLo);

    uint64_t aVal = a.nextUInt64();
    uint64_t bVal = (uint64_t(SIMD::v_get_i32(bLo))      ) +
                    (uint64_t(SIMD::v_get_i32(bHi)) << 32) ;

    EXPECT(aVal == bVal);
  }
  #endif

  // Test whether the random 32-bit integer is the HI part of the 64-bit one.
  {
    BLRandom a(0);
    BLRandom b(0);
    EXPECT(uint32_t(a.nextUInt64() >> 32) == b.nextUInt32());
  }

  // Test whether returned doubles satisfy [0..1) condition.
  {
    // Supply a low-entropy seed on purpose.
    BLRandom rnd(3);

    uint32_t below = 0;
    uint32_t above = 0;

    for (uint32_t i = 0; i < kCount; i++) {
      double x = rnd.nextDouble();
      below += x <  0.5;
      above += x >= 0.5;
      EXPECT(x >= 0.0 && x < 1.0);
    }
    INFO("Random numbers at [0.0, 0.5): %u of %u", below, kCount);
    INFO("Random numbers at [0.5, 1.0): %u of %u", above, kCount);
  }
}
#endif
