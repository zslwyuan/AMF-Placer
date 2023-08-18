// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_PIPEGEN_FETCHPIXELPTRPART_P_H
#define BLEND2D_PIPEGEN_FETCHPIXELPTRPART_P_H

#include "../pipegen/fetchpart_p.h"

//! \cond INTERNAL
//! \addtogroup blend2d_internal_pipegen
//! \{

namespace BLPipeGen {

// ============================================================================
// [BLPipeGen::FetchPixelPtrPart]
// ============================================================================

//! Pipeline fetch pixel-pointer part.
class FetchPixelPtrPart : public FetchPart {
public:
  BL_NONCOPYABLE(FetchPixelPtrPart)

  //! Pixel pointer.
  x86::Gp _ptr;
  //! Pixel pointer alignment (updated by FillPart|CompOpPart).
  uint8_t _ptrAlignment;

  FetchPixelPtrPart(PipeCompiler* pc, uint32_t fetchType, uint32_t fetchPayload, uint32_t format) noexcept;

  //! Initializes the pixel pointer to `p`.
  inline void initPtr(const x86::Gp& p) noexcept { _ptr = p; }
  //! Returns the pixel-pointer.
  inline x86::Gp& ptr() noexcept { return _ptr; }

  //! Returns the pixel-pointer alignment.
  inline uint32_t ptrAlignment() const noexcept { return _ptrAlignment; }
  //! Sets the pixel-pointer alignment.
  inline void setPtrAlignment(uint32_t alignment) noexcept { _ptrAlignment = uint8_t(alignment); }

  void fetch1(Pixel& p, uint32_t flags) noexcept override;
  void fetch4(Pixel& p, uint32_t flags) noexcept override;
  void fetch8(Pixel& p, uint32_t flags) noexcept override;
};

} // {BLPipeGen}

//! \}
//! \endcond

#endif // BLEND2D_PIPEGEN_FETCHPIXELPTRPART_P_H
