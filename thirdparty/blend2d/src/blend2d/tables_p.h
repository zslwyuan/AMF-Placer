// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_TABLES_P_H
#define BLEND2D_TABLES_P_H

#include "./lookuptable_p.h"

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

// ============================================================================
// [BLBitCountOfByteTable]
// ============================================================================

BL_HIDDEN extern const BLLookupTable<uint8_t, 256> blBitCountOfByteTable;

// ============================================================================
// [BLModuloTable]
// ============================================================================

//! Table that contains precomputed `{1..16} % N`.
struct alignas(16) BLModuloTable {
  uint8_t x1_16[16];
};

BL_HIDDEN extern const BLModuloTable blModuloTable[18];

// ============================================================================
// [BLCommonTable]
// ============================================================================

//! Common table that contains constants used across Blend2D library, but most
//! importantly in pipelines (either static or dynamic. The advantage of this
//! table is that it contains all constants that SIMD code (or also a generic
//! code) requires so only one register (pointer) is required to address all
//! of them in either static or generated pipelines.
struct alignas(32) BLCommonTable {
  struct LoadStoreM32 {
    uint32_t data[8];

    template<typename T>
    BL_INLINE const T& as() const noexcept { return *reinterpret_cast<const T*>(data); }
  };

  // --------------------------------------------------------------------------
  // [Vec128I Constants]
  // --------------------------------------------------------------------------

  uint64_t i128_0000000000000000[2];     // 8xU16(0x0000).

  uint64_t i128_007F007F007F007F[2];     // 8xU16(0x007F).
  uint64_t i128_0080008000800080[2];     // 8xU16(0x0080).
  uint64_t i128_00FF00FF00FF00FF[2];     // 8xU16(0x00FF).
  uint64_t i128_0100010001000100[2];     // 8xU16(0x0100).
  uint64_t i128_0101010101010101[2];     // 8xU16(0x0101).
  uint64_t i128_01FF01FF01FF01FF[2];     // 8xU16(0x01FF).
  uint64_t i128_0200020002000200[2];     // 8xU16(0x0200).
  uint64_t i128_8000800080008000[2];     // 8xU16(0x8000).
  uint64_t i128_FFFFFFFFFFFFFFFF[2];     // 8xU16(0xFFFF).

  uint64_t i128_000000FF000000FF[2];     // 4xU32(0x000000FF).
  uint64_t i128_0000010000000100[2];     // 4xU32(0x00000100).
  uint64_t i128_000001FF000001FF[2];     // 4xU32(0x000001FF).
  uint64_t i128_0000020000000200[2];     // 4xU32(0x00000200).
  uint64_t i128_0000FFFF0000FFFF[2];     // 4xU32(0x0000FFFF).
  uint64_t i128_0002000000020000[2];     // 4xU32(256u <<  9).
  uint64_t i128_00FFFFFF00FFFFFF[2];     // 4xU32(0x00FFFFFF).
  uint64_t i128_0101000001010000[2];     // 4xU32(0x01010000).
  uint64_t i128_FF000000FF000000[2];     // 4xU32(0xFF000000).
  uint64_t i128_FFFF0000FFFF0000[2];     // 4xU32(0xFFFF0000).

  uint64_t i128_000000FF00FF00FF[2];     // 2xU64(0x000000FF00FF00FF).
  uint64_t i128_0000010001000100[2];     // 2xU64(0x0000010001000100).
  uint64_t i128_0000080000000800[2];     // 2xU64(0x0000080000000800).
  uint64_t i128_0000FFFFFFFFFFFF[2];     // 2xU64(0x0000FFFFFFFFFFFF).
  uint64_t i128_00FF000000000000[2];     // 2xU64(0x00FF000000000000).
  uint64_t i128_0100000000000000[2];     // 2xU64(0x0100000000000000).
  uint64_t i128_0101010100000000[2];     // 2xU64(0x0101010100000000).
  uint64_t i128_FFFF000000000000[2];     // 2xU64(0xFFFF000000000000).
  uint64_t i128_FFFFFFFF00000000[2];     // 2xU64(0xFFFFFFFF00000000).

  // TODO: These constants need some renaming...

  //! uint32_t(0), uint32_t(0xFFFFFFFF), uint32_t(0xFFFFFFFF), uint32_t(0xFFFFFFFF).
  uint32_t i128_FFFFFFFF_FFFFFFFF_FFFFFFFF_0[4];

  uint32_t xmm_u32_0_1_2_3[4];           // uint32_t(0), uint32_t(1), uint32_t(2), uint32_t(3).
  uint32_t xmm_u32_4[4];                 // uint32_t(4).

  // --------------------------------------------------------------------------
  // [Vec128F Constants]
  // --------------------------------------------------------------------------

  uint64_t f128_sgn[2];                  // Mask of all `float` bits containing a sign.
  uint64_t f128_abs[2];                  // Mask of all `float` bits without a sign.
  uint64_t f128_abs_lo[2];               // Mask of all LO `float` bits without a sign.
  uint64_t f128_abs_hi[2];               // Mask of all HI `float` bits without a sign.
  float f128_round_max[4];               // Maximum float value to round (8388608).
  float f128_round_magic[4];             // Magic float used by round (12582912).

  float f128_1[4];                       // Vector of `1.0f`.
  float f128_4[4];                       // Vector of `4.0f`.
  float f128_255[4];                     // Vector of `255.0f`.
  float f128_1e_m3[4];                   // Vector of `1e-3`.
  float f128_1e_m20[4];                  // Vector of `1e-20`.
  float f128_1div255[4];                 // Vector of `1.0f / 255.0f`.
  float f128_3_2_1_0[4];                 // Vector of `[3f, 2f, 1f, 0f]`.

  // --------------------------------------------------------------------------
  // [Vec128D Constants]
  // --------------------------------------------------------------------------

  uint64_t d128_sgn[2];                  // Mask of all `double` bits containing a sign.
  uint64_t d128_abs[2];                  // Mask of all `double` bits without a sign.
  uint64_t d128_abs_lo[2];               // Mask of LO `double` bits without a sign.
  uint64_t d128_abs_hi[2];               // Mask of HI `double` bits without a sign.
  double d128_round_max[4];              // Maximum double value to round (4503599627370496).
  double d128_round_magic[4];            // Magic double used by round (6755399441055744).

  double d128_1[2];                      // Vector of `1.0`.
  double d128_1e_m20[2];                 // Vector of `1e-20`.
  double d128_4[2];                      // Vector of `4.0`.
  double d128_m1[2];                     // Vector of `-1.0`.

  // --------------------------------------------------------------------------
  // [PSHUFB Constants]
  // --------------------------------------------------------------------------

  #if BL_TARGET_ARCH_X86

  // PSHUFB predicates for performing shuffles.
  uint8_t i128_pshufb_u32_to_u8_lo[16];
  uint8_t i128_pshufb_u32_to_u16_lo[16];

  // PSHUFB predicates for unpacking ARGB32 into A8 components.
  uint8_t i128_pshufb_argb32_to_a8_packed[16];
  uint8_t i128_pshufb_packed_argb32_2x_lo_to_unpacked_a8[16];
  uint8_t i128_pshufb_packed_argb32_2x_hi_to_unpacked_a8[16];

  #endif

  // --------------------------------------------------------------------------
  // [Alignment]
  // --------------------------------------------------------------------------

  uint8_t alignment[32];

  // --------------------------------------------------------------------------
  // [Vec256I Constants]
  // --------------------------------------------------------------------------

  uint64_t i256_007F007F007F007F[4];     // 16xU16(0x007F).
  uint64_t i256_0080008000800080[4];     // 16xU16(0x0080).
  uint64_t i256_00FF00FF00FF00FF[4];     // 16xU16(0x00FF).
  uint64_t i256_0100010001000100[4];     // 16xU16(0x0100).
  uint64_t i256_0101010101010101[4];     // 16xU16(0x0101).
  uint64_t i256_01FF01FF01FF01FF[4];     // 16xU16(0x01FF).
  uint64_t i256_0200020002000200[4];     // 16xU16(0x0200).
  uint64_t i256_8000800080008000[4];     // 16xU16(0x8000).
  uint64_t i256_FFFFFFFFFFFFFFFF[4];     // 16xU16(0xFFFF).

  // --------------------------------------------------------------------------
  // [Vec256I Load/Store Masks]
  // --------------------------------------------------------------------------

  LoadStoreM32 load_store_m32[9]; // Index 0..9.

  // --------------------------------------------------------------------------
  // [Conical]
  // --------------------------------------------------------------------------

  enum TableId {
    kTable256   = 0,
    kTable512   = 1,
    kTable1024  = 2,
    kTable2048  = 3,
    kTable4096  = 4,
    kTableCount = 5
  };

  struct Conical {
    float n_div_1[4];
    float n_div_2[4];
    float n_div_4[4];
    float n_extra[4];

    // Polynomial to approximate `atan(x) * N / 2PI`:
    //   `x * (Q0 + x*x * (Q1 + x*x * (Q2 + x*x * Q3)))`
    // Where:
    //   `x >= 0 && x <= 1`
    float q0[4];
    float q1[4];
    float q2[4];
    float q3[4];
  } xmm_f_con[kTableCount];

  // --------------------------------------------------------------------------
  // [UnpremultiplyData]
  // --------------------------------------------------------------------------

  //! Table, which can be used to turn integer division into multiplication and
  //! shift that is used by PRGB to ARGB (unpremultiply) pixel conversion. It
  //! supports division by 0 (multiplies by zero) up to 255 using 24 bits of
  //! precision. The multiplied product has to be shifted to the right by 16
  //! bits to receive the final result.
  //!
  //! The unpremultiply function:
  //!
  //!   `if (b) ? (a * 255) / b : 0`
  //!
  //! can be rewritten as
  //!
  //!   `(a * unpremultiplyRcp[b] + 0x8000u) >> 16`
  uint32_t unpremultiplyRcp[256];

  // These tables are designed to take advantage of PMADDWD instruction (or its
  // VPMADDWD AVX variant) so we only use them on X86 targets. The approach
  // with `unpremultiplyRcp[]` table is not possible in baseline SSE2 and it's
  // not so good even on SSE4.1 capable hardware that has PMULLD instruction,
  // which has double the latency compared to other multiplication instructions
  // including PMADDWD.
  //
  // The downside of this approach is that we need two extra tables, as the
  // multiplication is not precise enough so we need variable a "rounding"
  // correction precalculated in the second table.
  #if BL_TARGET_ARCH_X86

  uint32_t unpremultiplyPmaddwdRcp[256];
  uint32_t unpremultiplyPmaddwdRnd[256];

  #endif

  // --------------------------------------------------------------------------
  // [Epilog]
  // --------------------------------------------------------------------------

  //! Dummy constant to have something at the end.
  uint8_t epilog[32];
};

BL_HIDDEN extern const BLCommonTable blCommonTable;

//! \}
//! \endcond

#endif // BLEND2D_TABLES_P_H
