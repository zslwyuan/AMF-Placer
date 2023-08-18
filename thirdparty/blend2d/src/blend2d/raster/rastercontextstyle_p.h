// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_RASTER_RASTERCONTEXTSTYLE_P_H
#define BLEND2D_RASTER_RASTERCONTEXTSTYLE_P_H

#include "../raster/rasterdefs_p.h"

//! \cond INTERNAL
//! \addtogroup blend2d_internal_raster
//! \{

// ============================================================================
// [BLRasterContextStyleSource]
// ============================================================================

union BLRasterContextStyleSource {
  //! Solid data.
  BLPipeFetchData::Solid solid;
  //! Fetch data.
  BLRasterFetchData* fetchData;

  //! Reset all data to zero.
  BL_INLINE void reset() noexcept { solid.prgb64 = 0; }
};

// ============================================================================
// [BLRasterContextStyleData]
// ============================================================================

//! Style data holds a copy of user-provided style with additional members that
//! allow to create a `BLRasterFetchData` from it. When a style is assigned to
//! the rendering context it has to calculate the style transformation matrix
//! and a few other things that could degrade the style into a solid fill.
struct BLRasterContextStyleData {
  union {
    uint32_t packed;
    struct {
      uint8_t cmdFlags;
      //! Style type.
      uint8_t styleType;
      //! Style pixel format.
      uint8_t styleFormat;
      //! Gradient/Pattern filter.
      uint8_t quality : 4;
      //! Adjusted matrix type.
      uint8_t adjustedMatrixType : 4;
    };
  };

  //! Alpha value (0..255 or 0..65535).
  uint32_t alphaI;
  //! Source data - either solid data or pointer to `BLRasterFetchData`.
  BLRasterContextStyleSource source;

  union {
    //! Solid color as non-premultiplied RGBA (float components).
    BLRgba rgba;
    //! Solid color as non-premultiplied RGBA32 (integer components).
    BLRgba32 rgba32;
    //! Solid color as non-premultiplied RGBA64 (integer components).
    BLRgba64 rgba64;
    //! Image area in case this data wraps a `BLPattern` object.
    BLRectI imageArea;
    //! Structure used to tag which data is used, whether `rgba` or `rgba32`.
    struct {
      uint32_t u32[3];
      uint32_t tag;
    } tagging;
  };

  //! Adjusted matrix.
  BLMatrix2D adjustedMatrix;

  BL_INLINE bool hasFetchData() const noexcept {
    return (cmdFlags & BL_RASTER_COMMAND_FLAG_FETCH_DATA) != 0;
  }
};

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RASTERCONTEXTSTYLE_P_H
