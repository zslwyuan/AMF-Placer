// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include "./api-build_p.h"
#include "./rgba_p.h"

// ============================================================================
// [BLRgba - Unit Tests]
// ============================================================================

#ifdef BL_TEST
UNIT(rgba, -7) {
  BLRgba32 c32(0x01, 0x02, 0x03, 0xFF);
  BLRgba64 c64(0x100, 0x200, 0x300, 0xFFFF);

  EXPECT(c32.value == 0xFF010203u);
  EXPECT(c64.value == 0xFFFF010002000300u);

  EXPECT(BLRgba64(c32).value == 0xFFFF010102020303u);
  EXPECT(BLRgba32(c64).value == 0xFF010203u);
}
#endif
