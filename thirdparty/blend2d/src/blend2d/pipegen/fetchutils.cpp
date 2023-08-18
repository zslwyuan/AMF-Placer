// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include "../api-build_p.h"
#if BL_TARGET_ARCH_X86 && !defined(BL_BUILD_NO_JIT)

#include "../pipegen/fetchutils_p.h"

namespace BLPipeGen {

// ============================================================================
// [BLPipeGen::IndexExtractor]
// ============================================================================

IndexExtractor::IndexExtractor(PipeCompiler* pc) noexcept
  : _pc(pc),
    _mem(),
    _type(kTypeNone),
    _indexSize(0),
    _memSize(0) {}

void IndexExtractor::begin(uint32_t type, const x86::Vec& vec) noexcept {
  BL_ASSERT(type != kTypeNone);
  BL_ASSERT(type < kTypeCount);

  uint32_t vecSize = vec.size();
  x86::Mem mem = _pc->tmpStack(vecSize);

  if (vecSize <= 16)
    _pc->v_storea_i128(mem, vec);
  else
    _pc->v_storeu_i256(mem, vec);

  begin(type, mem, vec.size());
}

void IndexExtractor::begin(uint32_t type, const x86::Mem& mem, uint32_t memSize) noexcept {
  BL_ASSERT(type != kTypeNone);
  BL_ASSERT(type < kTypeCount);

  _type = type;
  _mem = mem;
  _memSize = uint16_t(memSize);

  switch (_type) {
    case kTypeInt16:
    case kTypeUInt16:
      _indexSize = 2;
      break;

    case kTypeInt32:
    case kTypeUInt32:
      _indexSize = 4;
      break;

    default:
      BL_NOT_REACHED();
  }
}

void IndexExtractor::extract(const x86::Gp& dst, uint32_t index) noexcept {
  BL_ASSERT(dst.size() >= 4);
  BL_ASSERT(_type != kTypeNone);

  uint32_t offset = index * _indexSize;
  BL_ASSERT(offset + _indexSize <= _memSize);

  x86::Mem m = _mem;
  x86::Compiler* cc = _pc->cc;

  m.setSize(int(_indexSize));
  m.addOffset(int(index * _indexSize));

  switch (_type) {
    case kTypeInt16: {
      cc->movsx(dst, m);
      break;
    }

    case kTypeUInt16: {
      cc->movzx(dst.r32(), m);
      break;
    }

    case kTypeInt32: {
      if (dst.size() == 8)
        cc->movsxd(dst, m);
      else
        cc->mov(dst, m);
      break;
    }

    case kTypeUInt32: {
      cc->mov(dst.r32(), m);
      break;
    }

    default:
      BL_NOT_REACHED();
  }
}

// ============================================================================
// [BLPipeGen::FetchContext]
// ============================================================================

void FetchContext::_init(uint32_t n) noexcept {
  BL_ASSERT(n == 4 || n == 8);

  _pixel->setCount(n);
  _fetchDone = false;

  // The strategy for fetching alpha pixels is a bit different than fetching
  // RGBA pixels. In general we prefer to fetch into a GP accumulator and then
  // convert it to XMM|YMM at the end.
  _a8FetchMode = _fetchFormat == BL_FORMAT_A8 || _pixel->isAlpha();

  x86::Compiler* cc = _pc->cc;
  switch (_pixel->type()) {
    case Pixel::kTypeRGBA:
      if (!_pc->hasSSE4_1() && !_a8FetchMode) {
        // We need some temporaries if the CPU doesn't support `SSE4.1`.
        pTmp0 = cc->newXmm("@pTmp0");
        pTmp1 = cc->newXmm("@pTmp1");
      }

      if (_fetchFlags & Pixel::kPC) {
        _pc->newXmmArray(_pixel->pc, (n + 3) / 4, "pc");
        aTmp = _pixel->pc[0].as<x86::Xmm>();
      }
      else {
        _pc->newXmmArray(_pixel->uc, (n + 1) / 2, "uc");
        aTmp = _pixel->uc[0].as<x86::Xmm>();
      }
      break;

    case Pixel::kTypeAlpha:
      if (_fetchFlags & Pixel::kPA) {
        _pc->newXmmArray(_pixel->pa, 1, "pa");
        aTmp = _pixel->pa[0].as<x86::Xmm>();
      }
      else {
        _pc->newXmmArray(_pixel->ua, 1, "ua");
        aTmp = _pixel->ua[0].as<x86::Xmm>();
      }
      break;
  }

  if (_a8FetchMode) {
    if (cc->is64Bit() && n > 4) {
      aAcc = cc->newUInt64("@aAcc");
      _a8FetchShift = 8;
    }
    else if (cc->is64Bit() && (_fetchFlags & (Pixel::kUA | Pixel::kUC))) {
      aAcc = cc->newUInt64("@aAcc");
      _a8FetchShift = 16;
    }
    else {
      aAcc = cc->newUInt32("@aAcc");
      _a8FetchShift = 8;
    }
  }
}

void FetchContext::fetchPixel(const x86::Mem& src) noexcept {
  BL_ASSERT(_fetchIndex < _pixel->count());
  x86::Compiler* cc = _pc->cc;

  if (_a8FetchMode) {
    x86::Mem m(src);
    m.setSize(1);

    if (_fetchFormat == BL_FORMAT_PRGB32)
      m.addOffset(3);

    bool clearAcc = _fetchIndex == 0 || (_fetchIndex == 4 && aAcc.size() == 4);
    bool finalize = _fetchIndex == _pixel->count() - 1;

    if (clearAcc)
      cc->movzx(aAcc.r32(), m);
    else
      cc->mov(aAcc.r8(), m);
    cc->ror(aAcc, _a8FetchShift);

    if (finalize) {
      // The last pixel -> Convert to XMM.
      if (aAcc.size() == 8) {
        _pc->s_mov_i64(aTmp, aAcc);
      }
      else if (_fetchIndex == 7) {
        if (_pc->hasSSE4_1()) {
          _pc->v_insert_u32_(aTmp, aTmp, aAcc, 1);
        }
        else {
          x86::Xmm aHi = cc->newXmm("@aHi");
          _pc->s_mov_i32(aHi, aAcc);
          _pc->v_interleave_lo_i32(aTmp, aTmp, aHi);
        }
      }
      else {
        _pc->s_mov_i32(aTmp, aAcc);
      }

      if (_a8FetchShift == 8 && !(_fetchFlags & (Pixel::kPA | Pixel::kPC)))
        _pc->vmovu8u16(aTmp, aTmp);
    }
    else if (_fetchIndex == 3 && aAcc.size() == 4) {
      // Not the last pixel, but we have to convert to XMM as we have no more
      // space in the GP accumulator. This should only happen in 32-bit mode.
      _pc->s_mov_i32(aTmp, aAcc);
    }
  }
  else if (_pixel->isRGBA()) {
    bool isPC = (_fetchFlags & Pixel::kPC) != 0;
    VecArray& uc = _pixel->uc;

    x86::Vec p0 = isPC ? _pixel->pc[0] : uc[0];
    x86::Vec p1;

    if (_pixel->count() > 4)
      p1 = isPC ? _pixel->pc[1] : uc[2];

    if (!_pc->hasSSE4_1()) {
      switch (_fetchIndex) {
        case 0:
          _pc->v_load_i32(p0, src);
          break;

        case 1:
          _pc->v_load_i32(pTmp0, src);
          break;

        case 2:
          _pc->v_interleave_lo_i32(p0, p0, pTmp0);
          if (isPC)
            _pc->v_load_i32(pTmp0, src);
          else
            _pc->v_load_i32(uc[1], src);
          break;

        case 3:
          _pc->v_load_i32(pTmp1, src);
          break;

        case 4:
          if (isPC) {
            _pc->v_interleave_lo_i32(pTmp0, pTmp0, pTmp1);
            _pc->v_interleave_lo_i64(p0, p0, pTmp0);
          }
          else {
            _pc->v_interleave_lo_i32(uc[1], uc[1], pTmp1);
          }

          _pc->v_load_i32(p1, src);
          break;

        case 5:
          _pc->v_load_i32(pTmp0, src);
          break;

        case 6:
          _pc->v_interleave_lo_i32(p1, p1, pTmp0);
          if (isPC)
            _pc->v_load_i32(pTmp0, src);
          else
            _pc->v_load_i32(uc[3], src);
          break;

        case 7:
          _pc->v_load_i32(pTmp1, src);
          break;
      }
    }
    else {
      switch (_fetchIndex) {
        case 0:
          _pc->v_load_i32(p0, src);
          break;

        case 1:
          _pc->v_insert_u32_(p0, p0, src, 1);
          break;

        case 2:
          if (isPC)
            _pc->v_insert_u32_(p0, p0, src, 2);
          else
            _pc->v_load_i32(uc[1], src);
          break;

        case 3:
          if (isPC)
            _pc->v_insert_u32_(p0, p0, src, 3);
          else
            _pc->v_insert_u32_(uc[1], uc[1], src, 1);
          break;

        case 4:
          _pc->v_load_i32(p1, src);
          break;

        case 5:
          _pc->v_insert_u32_(p1, p1, src, 1);
          break;

        case 6:
          if (isPC)
            _pc->v_insert_u32_(p1, p1, src, 2);
          else
            _pc->v_load_i32(uc[3], src);
          break;

        case 7:
          if (isPC)
            _pc->v_insert_u32_(p1, p1, src, 3);
          else
            _pc->v_insert_u32_(uc[3], uc[3], src, 1);
          break;
      }
    }
  }

  _fetchIndex++;
}

void FetchContext::_fetchAll(const x86::Mem& src, uint32_t srcShift, IndexExtractor& extractor, const uint8_t* indexes, InterleaveCallback cb, void* cbData) noexcept {
  BL_ASSERT(_fetchIndex == 0);

  x86::Compiler* cc = _pc->cc;

  x86::Gp idx0 = cc->newIntPtr("@idx0");
  x86::Gp idx1 = cc->newIntPtr("@idx1");

  x86::Mem src0 = src;
  x86::Mem src1 = src;

  src0.setIndex(idx0, srcShift);
  src1.setIndex(idx1, srcShift);

  switch (_pixel->count()) {
    case 2:
      extractor.extract(idx0, indexes[0]);
      extractor.extract(idx1, indexes[1]);

      cb(0, cbData);
      fetchPixel(src0);

      cb(1, cbData);
      fetchPixel(src1);
      break;

    case 4:
      extractor.extract(idx0, indexes[0]);
      extractor.extract(idx1, indexes[1]);

      cb(0, cbData);
      fetchPixel(src0);
      extractor.extract(idx0, indexes[2]);

      cb(1, cbData);
      fetchPixel(src1);
      extractor.extract(idx1, indexes[3]);

      cb(2, cbData);
      fetchPixel(src0);

      cb(3, cbData);
      fetchPixel(src1);
      break;

    case 8:
      if (_pc->hasSSE4_1() && (_fetchFlags & Pixel::kPC) && blFormatInfo[_fetchFormat].depth == 32) {
        x86::Vec& pc0 = _pixel->pc[0];
        x86::Vec& pc1 = _pixel->pc[1];

        extractor.extract(idx0, indexes[0]);
        extractor.extract(idx1, indexes[4]);

        cb(0, cbData);
        _pc->v_load_i32(pc0, src0);
        extractor.extract(idx0, indexes[1]);

        cb(1, cbData);
        _pc->v_load_i32(pc1, src1);
        extractor.extract(idx1, indexes[5]);

        cb(2, cbData);
        _pc->v_insert_u32_(pc0, pc0, src0, 1);
        extractor.extract(idx0, indexes[2]);

        cb(3, cbData);
        _pc->v_insert_u32_(pc1, pc1, src1, 1);
        extractor.extract(idx1, indexes[6]);

        cb(4, cbData);
        _pc->v_insert_u32_(pc0, pc0, src0, 2);
        extractor.extract(idx0, indexes[3]);

        cb(5, cbData);
        _pc->v_insert_u32_(pc1, pc1, src1, 2);
        extractor.extract(idx1, indexes[7]);

        cb(6, cbData);
        _pc->v_insert_u32_(pc0, pc0, src0, 3);

        cb(7, cbData);
        _pc->v_insert_u32_(pc1, pc1, src1, 3);

        _fetchIndex = 8;
        _fetchDone = true;
      }
      else {
        extractor.extract(idx0, indexes[0]);
        extractor.extract(idx1, indexes[1]);

        cb(0, cbData);
        fetchPixel(src0);
        extractor.extract(idx0, indexes[2]);

        cb(1, cbData);
        fetchPixel(src1);
        extractor.extract(idx1, indexes[3]);

        cb(2, cbData);
        fetchPixel(src0);
        extractor.extract(idx0, indexes[4]);

        cb(3, cbData);
        fetchPixel(src1);
        extractor.extract(idx1, indexes[5]);

        cb(4, cbData);
        fetchPixel(src0);
        extractor.extract(idx0, indexes[6]);

        cb(5, cbData);
        fetchPixel(src1);
        extractor.extract(idx1, indexes[7]);

        cb(6, cbData);
        fetchPixel(src0);

        cb(7, cbData);
        fetchPixel(src1);
      }
      break;

    default:
      BL_NOT_REACHED();
  }
}

void FetchContext::end() noexcept {
  uint32_t n = _pixel->count();

  BL_ASSERT(n != 0);
  BL_ASSERT(n == _fetchIndex);

  if (_fetchDone)
    return;

  if (_a8FetchMode) {
    if (_pixel->isRGBA()) {
      if (_fetchFlags & Pixel::kPC) {
        switch (n) {
          case 4: {
            x86::Vec& a0 = _pixel->pc[0];

            _pc->v_interleave_lo_i8(a0, a0, a0);
            _pc->v_interleave_lo_i16(a0, a0, a0);
            break;
          }

          case 8: {
            x86::Vec& a0 = _pixel->pc[0];
            x86::Vec& a1 = _pixel->pc[1];

            _pc->v_interleave_hi_i8(a1, a0, a0);
            _pc->v_interleave_lo_i8(a0, a0, a0);
            _pc->v_interleave_hi_i16(a1, a1, a1);
            _pc->v_interleave_lo_i16(a0, a0, a0);
            break;
          }

          default:
            BL_NOT_REACHED();
        }
      }
      else {
        switch (n) {
          case 4: {
            x86::Vec& a0 = _pixel->uc[0];
            x86::Vec& a1 = _pixel->uc[1];

            _pc->v_interleave_lo_i16(a0, a0, a0);

            _pc->v_swizzle_i32(a1, a0, x86::Predicate::shuf(3, 3, 2, 2));
            _pc->v_swizzle_i32(a0, a0, x86::Predicate::shuf(1, 1, 0, 0));
            break;
          }

          case 8: {
            x86::Vec& a0 = _pixel->uc[0];
            x86::Vec& a1 = _pixel->uc[1];
            x86::Vec& a2 = _pixel->uc[2];
            x86::Vec& a3 = _pixel->uc[3];

            _pc->v_interleave_hi_i16(a2, a0, a0);
            _pc->v_interleave_lo_i16(a0, a0, a0);

            _pc->v_swizzle_i32(a3, a2, x86::Predicate::shuf(3, 3, 2, 2));
            _pc->v_swizzle_i32(a1, a0, x86::Predicate::shuf(3, 3, 2, 2));
            _pc->v_swizzle_i32(a2, a2, x86::Predicate::shuf(1, 1, 0, 0));
            _pc->v_swizzle_i32(a0, a0, x86::Predicate::shuf(1, 1, 0, 0));
            break;
          }

          default:
            BL_NOT_REACHED();
        }
      }
    }
    else {
      // Nothing...
    }
  }
  else {
    if (!_pc->hasSSE4_1()) {
      if (_fetchFlags & Pixel::kPC) {
        const x86::Vec& pcLast = _pixel->pc[_pixel->pc.size() - 1];
        _pc->v_interleave_lo_i32(pTmp0, pTmp0, pTmp1);
        _pc->v_interleave_lo_i64(pcLast, pcLast, pTmp0);
      }
      else {
        const x86::Vec& ucLast = _pixel->uc[_pixel->uc.size() - 1];
        _pc->v_interleave_lo_i32(ucLast, ucLast, pTmp1);
      }
    }

    if (_fetchFlags & Pixel::kPC) {
      // Nothing...
    }
    else {
      _pc->vmovu8u16(_pixel->uc, _pixel->uc);
    }
  }

  _fetchDone = true;
}

} // {BLPipeGen}

#endif
