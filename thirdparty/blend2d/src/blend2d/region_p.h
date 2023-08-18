// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_REGION_P_H
#define BLEND2D_REGION_P_H

#include "./api-internal_p.h"
#include "./array_p.h"
#include "./region.h"

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

// ============================================================================
// [BLRegion - Internal]
// ============================================================================

//! Internal implementation that extends `BLRegionImpl`.
struct BLInternalRegionImpl : public BLRegionImpl {
  // Internal members can be placed here in the future.
};

template<>
struct BLInternalCastImpl<BLRegionImpl> { typedef BLInternalRegionImpl Type; };

BL_HIDDEN BLResult blRegionImplDelete(BLRegionImpl* impl) noexcept;

//! \}
//! \endcond

#endif // BLEND2D_REGION_P_H
