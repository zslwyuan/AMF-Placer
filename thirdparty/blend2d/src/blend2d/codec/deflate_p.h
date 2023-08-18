// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef BLEND2D_CODEC_DEFLATE_P_H
#define BLEND2D_CODEC_DEFLATE_P_H

#include "../api-internal_p.h"
#include "../array.h"

//! \cond INTERNAL

// ============================================================================
// [BLDeflate]
// ============================================================================

struct Deflate {
  //! Callback that is used to read a chunk of data to be consumed by the
  //! decoder. It was introduced for PNG support, which can divide the data
  //! stream into multiple `"IDAT"` chunks, thus the stream is not continuous.
  //!
  //! The logic has been simplified in a way that `ReadFunc` reads the first
  //! and all consecutive chunks. There is no other way to be consumed by the
  //! decoder.
  typedef bool (BL_CDECL* ReadFunc)(void* readCtx, const uint8_t** pData, const uint8_t** pEnd) BL_NOEXCEPT;

  //! Deflate data retrieved by `ReadFunc` into `dst` buffer.
  static BLResult deflate(BLArray<uint8_t>& dst, void* readCtx, ReadFunc readFunc, bool hasHeader) noexcept;
};

//! \endcond

#endif // BLEND2D_CODEC_DEFLATE_P_H
