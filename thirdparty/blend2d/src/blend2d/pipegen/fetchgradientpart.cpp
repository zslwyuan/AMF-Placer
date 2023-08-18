// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include "../api-build_p.h"
#if BL_TARGET_ARCH_X86 && !defined(BL_BUILD_NO_JIT)

#include "../pipegen/compoppart_p.h"
#include "../pipegen/fetchgradientpart_p.h"
#include "../pipegen/fetchutils_p.h"
#include "../pipegen/pipecompiler_p.h"

namespace BLPipeGen {

#define REL_GRADIENT(FIELD) BL_OFFSET_OF(BLPipeFetchData::Gradient, FIELD)

// ============================================================================
// [BLPipeGen::FetchGradientPart - Construction / Destruction]
// ============================================================================

FetchGradientPart::FetchGradientPart(PipeCompiler* pc, uint32_t fetchType, uint32_t fetchPayload, uint32_t format) noexcept
  : FetchPart(pc, fetchType, fetchPayload, format),
    _extend(0) {}

void FetchGradientPart::fetchGradientPixel1(Pixel& dst, uint32_t flags, const x86::Mem& src) noexcept {
  pc->xFetchPixel_1x(dst, flags, BL_FORMAT_PRGB32, src, 4);
}

// ============================================================================
// [BLPipeGen::FetchLinearGradientPart - Construction / Destruction]
// ============================================================================

FetchLinearGradientPart::FetchLinearGradientPart(PipeCompiler* pc, uint32_t fetchType, uint32_t fetchPayload, uint32_t format) noexcept
  : FetchGradientPart(pc, fetchType, fetchPayload, format),
    _isRoR(fetchType == BL_PIPE_FETCH_TYPE_GRADIENT_LINEAR_ROR) {

  _maxPixels = 8;
  _maxSimdWidthSupported = 16;

  _persistentRegs[x86::Reg::kGroupGp] = 1;
  _persistentRegs[x86::Reg::kGroupVec] = 2;
  _extend = uint8_t(fetchType - BL_PIPE_FETCH_TYPE_GRADIENT_LINEAR_PAD);

  JitUtils::resetVarStruct(&f, sizeof(f));
}

// ============================================================================
// [BLPipeGen::FetchLinearGradientPart - Init / Fini]
// ============================================================================

void FetchLinearGradientPart::_initPart(x86::Gp& x, x86::Gp& y) noexcept {
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  f->table          = cc->newIntPtr("f.table");       // Reg.
  f->pt             = cc->newXmm("f.pt");             // Reg.
  f->dt             = cc->newXmm("f.dt");             // Reg/Mem.
  f->dt2            = cc->newXmm("f.dt2");            // Reg/Mem.
  f->py             = cc->newXmm("f.py");             // Reg/Mem.
  f->dy             = cc->newXmm("f.dy");             // Reg/Mem.
  f->rep            = cc->newXmm("f.rep");            // Reg/Mem [RoR only].
  f->msk            = cc->newXmm("f.msk");            // Reg/Mem.
  f->vIdx           = cc->newXmm("f.vIdx");           // Reg/Tmp.
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  cc->mov(f->table, x86::ptr(pc->_fetchData, REL_GRADIENT(lut.data)));

  pc->s_mov_i32(f->py, y);
  pc->v_broadcast_u64(f->dy, x86::ptr(pc->_fetchData, REL_GRADIENT(linear.dy.u64)));
  pc->v_dupl_i64(f->py, f->py);

  pc->vMulU64xU32Lo(f->py, f->dy, f->py);
  pc->v_loadu_i128(f->pt, x86::ptr(pc->_fetchData, REL_GRADIENT(linear.pt)));
  pc->v_add_i64(f->py, f->py, f->pt);

  pc->v_broadcast_u64(f->dt, x86::ptr(pc->_fetchData, REL_GRADIENT(linear.dt.u64)));
  pc->v_broadcast_u64(f->dt2, x86::ptr(pc->_fetchData, REL_GRADIENT(linear.dt2.u64)));

  if (isRoR()) {
    pc->v_broadcast_u64(f->rep, x86::ptr(pc->_fetchData, REL_GRADIENT(linear.rep.u64)));
  }

  pc->v_broadcast_u32(f->msk, x86::ptr(pc->_fetchData, REL_GRADIENT(linear.msk.u)));

  // If we cannot use `packusdw`, which was introduced by SSE4.1 we subtract
  // 32768 from the pointer and use `packssdw` instead. However, if we do this,
  // we have to adjust everything else accordingly.
  if (isPad() && !pc->hasSSE4_1()) {
    pc->v_sub_i32(f->py, f->py, pc->constAsMem(blCommonTable.i128_0000080000000800));
    pc->v_sub_i16(f->msk, f->msk, pc->constAsMem(blCommonTable.i128_8000800080008000));
  }

  if (isRectFill()) {
    pc->s_mov_i32(f->pt, x);
    pc->v_dupl_i64(f->pt, f->pt);
    pc->vMulU64xU32Lo(f->pt, f->dt, f->pt);
    pc->v_add_i64(f->py, f->py, f->pt);
  }

  if (pixelGranularity() > 1)
    enterN();
}

void FetchLinearGradientPart::_finiPart() noexcept {}

// ============================================================================
// [BLPipeGen::FetchLinearGradientPart - Advance]
// ============================================================================

void FetchLinearGradientPart::advanceY() noexcept {
  pc->v_add_i64(f->py, f->py, f->dy);
}

void FetchLinearGradientPart::startAtX(x86::Gp& x) noexcept {
  pc->v_mov(f->pt, f->py);

  if (!isRectFill())
    advanceX(x, x);
}

void FetchLinearGradientPart::advanceX(x86::Gp& x, x86::Gp& diff) noexcept {
  BL_UNUSED(x);

  x86::Xmm delta = cc->newXmm("f.delta");
  pc->s_mov_i32(delta, diff);
  pc->v_dupl_i64(delta, delta);
  pc->vMulU64xU32Lo(delta, f->dt, delta);
  pc->v_add_i64(f->pt, f->pt, delta);
}

// ============================================================================
// [BLPipeGen::FetchLinearGradientPart - Fetch]
// ============================================================================

void FetchLinearGradientPart::prefetch1() noexcept {
  if (isPad()) {
    // Nothing...
  }
  else {
    pc->v_and(f->pt, f->pt, f->rep);
  }
}

void FetchLinearGradientPart::fetch1(Pixel& p, uint32_t flags) noexcept {
  x86::Gp gIdx = cc->newInt32("gIdx");
  x86::Xmm vTmp = cc->newXmm("vTmp");

  if (isPad()) {
    if (pc->hasSSE4_1()) {
      pc->v_packs_i32_u16_(vTmp, f->pt, f->pt);
      pc->v_min_u16(vTmp, vTmp, f->msk);
      pc->v_add_i64(f->pt, f->pt, f->dt);

      pc->v_extract_u16(gIdx, vTmp, 1);
      fetchGradientPixel1(p, flags, x86::ptr(f->table, gIdx, 2));
      pc->xSatisfyPixel(p, flags);
    }
    else {
      pc->v_packs_i32_i16(vTmp, f->pt, f->pt);
      pc->v_min_i16(vTmp, vTmp, f->msk);
      pc->v_add_i16(vTmp, vTmp, pc->constAsMem(blCommonTable.i128_8000800080008000));
      pc->v_add_i64(f->pt, f->pt, f->dt);

      pc->v_extract_u16(gIdx, vTmp, 1);
      fetchGradientPixel1(p, flags, x86::ptr(f->table, gIdx, 2));
      pc->xSatisfyPixel(p, flags);
    }
  }
  else {
    pc->v_xor(vTmp, f->pt, f->msk);
    pc->v_min_i16(vTmp, vTmp, f->pt);
    pc->v_add_i64(f->pt, f->pt, f->dt);

    pc->v_extract_u16(gIdx, vTmp, 2);
    fetchGradientPixel1(p, flags, x86::ptr(f->table, gIdx, 2));

    pc->v_and(f->pt, f->pt, f->rep);
    pc->xSatisfyPixel(p, flags);
  }
}

void FetchLinearGradientPart::enterN() noexcept {}
void FetchLinearGradientPart::leaveN() noexcept {}

void FetchLinearGradientPart::prefetchN() noexcept {
  x86::Xmm vIdx = f->vIdx;

  if (isPad()) {
    pc->v_mov(vIdx, f->pt);
    pc->v_add_i64(f->pt, f->pt, f->dt2);
    pc->v_shuffle_i32(vIdx, vIdx, f->pt, x86::Predicate::shuf(3, 1, 3, 1));
    pc->v_add_i64(f->pt, f->pt, f->dt2);
  }
  else {
    pc->v_and(vIdx, f->pt, f->rep);
    pc->v_add_i64(f->pt, f->pt, f->dt2);
    pc->v_and(f->pt, f->pt, f->rep);
    pc->v_shuffle_i32(vIdx, vIdx, f->pt, x86::Predicate::shuf(3, 1, 3, 1));
    pc->v_add_i64(f->pt, f->pt, f->dt2);
  }
}

void FetchLinearGradientPart::postfetchN() noexcept {
  pc->v_sub_i64(f->pt, f->pt, f->dt2);
  pc->v_sub_i64(f->pt, f->pt, f->dt2);
}

void FetchLinearGradientPart::fetch4(Pixel& p, uint32_t flags) noexcept {
  FetchContext fCtx(pc, &p, 4, format(), flags);
  IndexExtractor iExt(pc);

  x86::Xmm vIdx = f->vIdx;
  uint32_t srcShift = 2;

  if (isPad()) {
    const uint8_t srcIndexes[4] = { 0, 1, 2, 3 };

    if (pc->hasSSE4_1()) {
      pc->v_packs_i32_u16_(vIdx, vIdx, vIdx);
      pc->v_min_u16(vIdx, vIdx, f->msk);
    }
    else {
      pc->v_packs_i32_i16(vIdx, vIdx, vIdx);
      pc->v_min_i16(vIdx, vIdx, f->msk);
      pc->v_add_i16(vIdx, vIdx, pc->constAsMem(blCommonTable.i128_8000800080008000));
    }

    iExt.begin(IndexExtractor::kTypeUInt16, vIdx);
    pc->v_mov(vIdx, f->pt);
    pc->v_add_i64(f->pt, f->pt, f->dt2);

    fCtx.fetchAll(x86::ptr(f->table), srcShift, iExt, srcIndexes, [&](uint32_t step) {
      switch (step) {
        case 3: pc->v_shuffle_i32(vIdx, vIdx, f->pt, x86::Predicate::shuf(3, 1, 3, 1)); break;
      }
    });

    pc->v_add_i64(f->pt, f->pt, f->dt2);
    fCtx.end();
  }
  else {
    const uint8_t srcIndexes[4] = { 0, 2, 4, 6 };

    x86::Xmm vTmp = cc->newXmm("vTmp");
    pc->v_xor(vTmp, vIdx, f->msk);
    pc->v_min_i16(vTmp, vTmp, vIdx);
    pc->v_and(vIdx, f->pt, f->rep);

    iExt.begin(IndexExtractor::kTypeUInt16, vTmp);
    pc->v_add_i64(f->pt, f->pt, f->dt2);

    fCtx.fetchAll(x86::ptr(f->table), srcShift, iExt, srcIndexes, [&](uint32_t step) {
      switch (step) {
        case 2: pc->v_and(f->pt, f->pt, f->rep); break;
        case 3: pc->v_shuffle_i32(vIdx, vIdx, f->pt, x86::Predicate::shuf(3, 1, 3, 1)); break;
      }
    });

    pc->v_add_i64(f->pt, f->pt, f->dt2);
    fCtx.end();
  }

  pc->xSatisfyPixel(p, flags);
}

void FetchLinearGradientPart::fetch8(Pixel& p, uint32_t flags) noexcept {
  FetchContext fCtx(pc, &p, 8, format(), flags);
  IndexExtractor iExt(pc);

  x86::Xmm vIdx = f->vIdx;
  x86::Xmm vTmp = cc->newXmm("vTmp0");

  uint32_t srcShift = 2;
  const uint8_t srcIndexes[8] = { 4, 5, 6, 7, 0, 1, 2, 3 };

  if (isPad()) {
    pc->v_mov(vTmp, f->pt);
    pc->v_add_i64(f->pt, f->pt, f->dt2);
    pc->v_shuffle_i32(vTmp, vTmp, f->pt, x86::Predicate::shuf(3, 1, 3, 1));
    pc->v_add_i64(f->pt, f->pt, f->dt2);

    if (pc->hasSSE4_1()) {
      pc->v_packs_i32_u16_(vTmp, vTmp, vIdx);
      pc->v_mov(vIdx, f->pt);
      pc->v_min_u16(vTmp, vTmp, f->msk);
    }
    else {
      pc->v_packs_i32_i16(vTmp, vTmp, vIdx);
      pc->v_min_i16(vTmp, vTmp, f->msk);
      pc->v_add_i16(vTmp, vTmp, pc->constAsMem(blCommonTable.i128_8000800080008000));
      pc->v_mov(vIdx, f->pt);
    }

    iExt.begin(IndexExtractor::kTypeUInt16, vTmp);
    pc->v_add_i64(f->pt, f->pt, f->dt2);

    fCtx.fetchAll(x86::ptr(f->table), srcShift, iExt, srcIndexes, [&](uint32_t step) {
      switch (step) {
        case 7: pc->v_shuffle_i32(vIdx, vIdx, f->pt, x86::Predicate::shuf(3, 1, 3, 1)); break;
      }
    });

    pc->v_add_i64(f->pt, f->pt, f->dt2);
    fCtx.end();
  }
  else {
    pc->v_and(vTmp, f->pt, f->rep);
    pc->v_add_i64(f->pt, f->pt, f->dt2);
    pc->v_and(f->pt, f->pt, f->rep);
    pc->v_shuffle_i32(vTmp, vTmp, f->pt, x86::Predicate::shuf(3, 1, 3, 1));

    pc->v_packs_i32_i16(vTmp, vTmp, vIdx);
    pc->v_add_i64(f->pt, f->pt, f->dt2);

    pc->v_xor(vIdx, vTmp, f->msk);
    pc->v_min_i16(vTmp, vTmp, vIdx);
    iExt.begin(IndexExtractor::kTypeUInt16, vTmp);

    pc->v_and(vIdx, f->pt, f->rep);
    pc->v_add_i64(f->pt, f->pt, f->dt2);

    fCtx.fetchAll(x86::ptr(f->table), srcShift, iExt, srcIndexes, [&](uint32_t step) {
      switch (step) {
        case 0: pc->v_and(f->pt, f->pt, f->rep); break;
        case 7: pc->v_shuffle_i32(vIdx, vIdx, f->pt, x86::Predicate::shuf(3, 1, 3, 1)); break;
      }
    });

    pc->v_add_i64(f->pt, f->pt, f->dt2);
    fCtx.end();
  }

  pc->xSatisfyPixel(p, flags);
}

// ============================================================================
// [BLPipeGen::FetchRadialGradientPart - Construction / Destruction]
// ============================================================================

FetchRadialGradientPart::FetchRadialGradientPart(PipeCompiler* pc, uint32_t fetchType, uint32_t fetchPayload, uint32_t format) noexcept
  : FetchGradientPart(pc, fetchType, fetchPayload, format) {

  _maxPixels = 4;
  _maxSimdWidthSupported = 16;
  _isComplexFetch = true;
  _persistentRegs[x86::Reg::kGroupVec] = 3;
  _temporaryRegs[x86::Reg::kGroupVec] = 1;
  _extend = uint8_t(fetchType - BL_PIPE_FETCH_TYPE_GRADIENT_RADIAL_PAD);

  JitUtils::resetVarStruct(&f, sizeof(f));
}

// ============================================================================
// [BLPipeGen::FetchRadialGradientPart - Init / Fini]
// ============================================================================

void FetchRadialGradientPart::_initPart(x86::Gp& x, x86::Gp& y) noexcept {
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  f->table          = cc->newIntPtr("f.table");       // Reg.
  f->xx_xy          = cc->newXmmPd("f.xx_xy");        // Mem.
  f->yx_yy          = cc->newXmmPd("f.yx_yy");        // Mem.
  f->ax_ay          = cc->newXmmPd("f.ax_ay");        // Mem.
  f->fx_fy          = cc->newXmmPd("f.fx_fy");        // Mem.
  f->da_ba          = cc->newXmmPd("f.da_ba");        // Mem.

  f->d_b            = cc->newXmmPd("f.d_b");          // Reg.
  f->dd_bd          = cc->newXmmPd("f.dd_bd");        // Reg.
  f->ddx_ddy        = cc->newXmmPd("f.ddx_ddy");      // Mem.

  f->px_py          = cc->newXmmPd("f.px_py");        // Reg.
  f->scale          = cc->newXmmPs("f.scale");        // Mem.
  f->ddd            = cc->newXmmPd("f.ddd");          // Mem.
  f->value          = cc->newXmmPs("f.value");        // Reg/Tmp.

  f->maxi           = cc->newUInt32("f.maxi");        // Mem.
  f->vmaxi          = cc->newXmm("f.vmaxi");          // Mem.
  f->vmaxf          = cc->newXmmPd("f.vmaxf");        // Mem.

  f->d_b_prev       = cc->newXmmPd("f.d_b_prev");     // Mem.
  f->dd_bd_prev     = cc->newXmmPd("f.dd_bd_prev");   // Mem.

  x86::Xmm off      = cc->newXmmPd("f.off");          // Local.
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  cc->mov(f->table, x86::ptr(pc->_fetchData, REL_GRADIENT(lut.data)));

  pc->v_loadu_d128(f->ax_ay, x86::ptr(pc->_fetchData, REL_GRADIENT(radial.ax)));
  pc->v_loadu_d128(f->fx_fy, x86::ptr(pc->_fetchData, REL_GRADIENT(radial.fx)));

  pc->v_loadu_d128(f->da_ba  , x86::ptr(pc->_fetchData, REL_GRADIENT(radial.dd)));
  pc->v_loadu_d128(f->ddx_ddy, x86::ptr(pc->_fetchData, REL_GRADIENT(radial.ddx)));

  pc->v_zero_f(f->scale);
  pc->s_cvt_f64_f32(f->scale, f->scale, x86::ptr(pc->_fetchData, REL_GRADIENT(radial.scale)));

  pc->v_load_f64(f->ddd, x86::ptr(pc->_fetchData, REL_GRADIENT(radial.ddd)));
  pc->v_dupl_f64(f->ddd, f->ddd);
  pc->vexpandlps(f->scale, f->scale);

  pc->v_loadu_d128(f->xx_xy, x86::ptr(pc->_fetchData, REL_GRADIENT(radial.xx)));
  pc->v_loadu_d128(f->yx_yy, x86::ptr(pc->_fetchData, REL_GRADIENT(radial.yx)));

  pc->v_zero_d(f->px_py);
  pc->s_cvt_int_f64(f->px_py, f->px_py, y);
  pc->v_loadu_d128(off, x86::ptr(pc->_fetchData, REL_GRADIENT(radial.ox)));

  pc->v_dupl_f64(f->px_py, f->px_py);
  pc->v_mul_f64(f->px_py, f->px_py, f->yx_yy);
  pc->v_add_f64(f->px_py, f->px_py, off);

  pc->v_load_i32(f->vmaxi, x86::ptr(pc->_fetchData, REL_GRADIENT(radial.maxi)));
  pc->vexpandli32(f->vmaxi, f->vmaxi);
  pc->s_mov_i32(f->maxi, f->vmaxi);

  if (extend() == BL_EXTEND_MODE_PAD) {
    pc->v_cvt_i32_f32(f->vmaxf, f->vmaxi);
  }

  if (isRectFill()) {
    pc->v_zero_d(off);
    pc->s_cvt_int_f64(off, off, x);
    pc->v_dupl_f64(off, off);
    pc->v_mul_f64(off, off, f->xx_xy);
    pc->v_add_f64(f->px_py, f->px_py, off);
  }
}

void FetchRadialGradientPart::_finiPart() noexcept {}

// ============================================================================
// [BLPipeGen::FetchRadialGradientPart - Advance]
// ============================================================================

void FetchRadialGradientPart::advanceY() noexcept {
  pc->v_add_f64(f->px_py, f->px_py, f->yx_yy);
}

void FetchRadialGradientPart::startAtX(x86::Gp& x) noexcept {
  if (isRectFill()) {
    precalc(f->px_py);
  }
  else {
    x86::Xmm px_py = cc->newXmmPd("@px_py");

    pc->v_zero_d(px_py);
    pc->s_cvt_int_f64(px_py, px_py, x);
    pc->v_dupl_f64(px_py, px_py);
    pc->v_mul_f64(px_py, px_py, f->xx_xy);
    pc->v_add_f64(px_py, px_py, f->px_py);

    precalc(px_py);
  }
}

void FetchRadialGradientPart::advanceX(x86::Gp& x, x86::Gp& diff) noexcept {
  BL_UNUSED(diff);

  if (isRectFill()) {
    precalc(f->px_py);
  }
  else {
    x86::Xmm px_py = cc->newXmmPd("@px_py");

    // TODO: [PIPEGEN] Duplicated code :(
    pc->v_zero_d(px_py);
    pc->s_cvt_int_f64(px_py, px_py, x);
    pc->v_dupl_f64(px_py, px_py);
    pc->v_mul_f64(px_py, px_py, f->xx_xy);
    pc->v_add_f64(px_py, px_py, f->px_py);

    precalc(px_py);
  }
}

// ============================================================================
// [BLPipeGen::FetchRadialGradientPart - Fetch]
// ============================================================================

void FetchRadialGradientPart::prefetch1() noexcept {
  pc->v_cvt_f64_f32(f->value, f->d_b);
  pc->v_and_f32(f->value, f->value, pc->constAsMem(blCommonTable.f128_abs_lo));
  pc->s_sqrt_f32(f->value, f->value, f->value);
}

void FetchRadialGradientPart::fetch1(Pixel& p, uint32_t flags) noexcept {
  x86::Xmm x0 = cc->newXmmPs("@x0");
  x86::Gp gIdx = cc->newInt32("@gIdx");

  pc->v_swizzle_i32(x0, f->value, x86::Predicate::shuf(1, 1, 1, 1));
  pc->v_add_f64(f->d_b, f->d_b, f->dd_bd);

  pc->s_add_f32(x0, x0, f->value);
  pc->v_cvt_f64_f32(f->value, f->d_b);

  pc->s_mul_f32(x0, x0, f->scale);
  pc->v_and_f32(f->value, f->value, pc->constAsMem(blCommonTable.f128_abs_lo));

  if (extend() == BL_EXTEND_MODE_PAD) {
    pc->s_max_f32(x0, x0, pc->constAsXmm(blCommonTable.i128_0000000000000000));
    pc->s_min_f32(x0, x0, f->vmaxf);
  }

  pc->s_add_f64(f->dd_bd, f->dd_bd, f->ddd);
  pc->s_cvtt_f32_int(gIdx, x0);
  pc->s_sqrt_f32(f->value, f->value, f->value);

  if (extend() == BL_EXTEND_MODE_REPEAT) {
    cc->and_(gIdx, f->maxi);
  }

  if (extend() == BL_EXTEND_MODE_REFLECT) {
    x86::Gp t = cc->newGpd("f.t");

    cc->mov(t, f->maxi);
    cc->and_(gIdx, t);
    cc->sub(t, gIdx);

    // Select the lesser, which would be at [0...tableSize).
    cc->cmp(gIdx, t);
    cc->cmovge(gIdx, t);
  }

  fetchGradientPixel1(p, flags, x86::ptr(f->table, gIdx, 2));
  pc->xSatisfyPixel(p, flags);
}

void FetchRadialGradientPart::prefetchN() noexcept {
  x86::Xmm& d_b   = f->d_b;
  x86::Xmm& dd_bd = f->dd_bd;
  x86::Xmm& ddd   = f->ddd;
  x86::Xmm& value = f->value;

  x86::Xmm x0 = cc->newXmmSd("@x0");
  x86::Xmm x1 = cc->newXmmSd("@x1");
  x86::Xmm x2 = cc->newXmmSd("@x2");

  pc->vmovaps(f->d_b_prev, f->d_b);     // Save `d_b`.
  pc->vmovaps(f->dd_bd_prev, f->dd_bd); // Save `dd_bd`.

  pc->v_cvt_f64_f32(x0, d_b);
  pc->v_add_f64(d_b, d_b, dd_bd);
  pc->s_add_f64(dd_bd, dd_bd, ddd);

  pc->v_cvt_f64_f32(x1, d_b);
  pc->v_add_f64(d_b, d_b, dd_bd);
  pc->s_add_f64(dd_bd, dd_bd, ddd);
  pc->v_shuffle_f32(x0, x0, x1, x86::Predicate::shuf(1, 0, 1, 0));

  pc->v_cvt_f64_f32(x1, d_b);
  pc->v_add_f64(d_b, d_b, dd_bd);
  pc->s_add_f64(dd_bd, dd_bd, ddd);

  pc->v_cvt_f64_f32(x2, d_b);
  pc->v_add_f64(d_b, d_b, dd_bd);
  pc->s_add_f64(dd_bd, dd_bd, ddd);
  pc->v_shuffle_f32(x1, x1, x2, x86::Predicate::shuf(1, 0, 1, 0));

  pc->v_shuffle_f32(value, x0, x1, x86::Predicate::shuf(2, 0, 2, 0));
  pc->v_and_f32(value, value, pc->constAsMem(blCommonTable.f128_abs));
  pc->v_sqrt_f32(value, value);

  pc->v_shuffle_f32(x0, x0, x1, x86::Predicate::shuf(3, 1, 3, 1));
  pc->v_add_f32(value, value, x0);
}

void FetchRadialGradientPart::postfetchN() noexcept {
  pc->vmovaps(f->d_b, f->d_b_prev);     // Restore `d_b`.
  pc->vmovaps(f->dd_bd, f->dd_bd_prev); // Restore `dd_bd`.
}

void FetchRadialGradientPart::fetch4(Pixel& p, uint32_t flags) noexcept {
  x86::Xmm& d_b   = f->d_b;
  x86::Xmm& dd_bd = f->dd_bd;
  x86::Xmm& ddd   = f->ddd;
  x86::Xmm& value = f->value;

  x86::Xmm x0 = cc->newXmmSd("@x0");
  x86::Xmm x1 = cc->newXmmSd("@x1");
  x86::Xmm x2 = cc->newXmmSd("@x2");
  x86::Xmm x3 = cc->newXmmSd("@x3");

  FetchContext fCtx(pc, &p, 4, format(), flags);
  IndexExtractor iExt(pc);

  uint32_t srcShift = 2;
  const uint8_t srcIndexes[4] = { 0, 2, 4, 6 };

  pc->v_mul_f32(value, value, f->scale);
  pc->v_cvt_f64_f32(x0, d_b);

  pc->vmovaps(f->d_b_prev, d_b);     // Save `d_b_prev`.
  pc->vmovaps(f->dd_bd_prev, dd_bd); // Save `dd_bd_prev`.

  if (extend() == BL_EXTEND_MODE_PAD)
    pc->v_max_f32(value, value, pc->constAsXmm(blCommonTable.i128_0000000000000000));

  pc->v_add_f64(d_b, d_b, dd_bd);
  pc->s_add_f64(dd_bd, dd_bd, ddd);

  if (extend() == BL_EXTEND_MODE_PAD)
    pc->v_min_f32(value, value, f->vmaxf);

  pc->v_cvt_f64_f32(x1, d_b);
  pc->v_add_f64(d_b, d_b, dd_bd);

  pc->v_cvt_f32_i32(x3, value);
  pc->s_add_f64(dd_bd, dd_bd, ddd);

  if (extend() == BL_EXTEND_MODE_REPEAT) {
    pc->v_and(x3, x3, f->vmaxi);
  }

  if (extend() == BL_EXTEND_MODE_REFLECT) {
    x86::Xmm t = cc->newXmm("t");
    pc->vmovaps(t, f->vmaxi);

    pc->v_and(x3, x3, t);
    pc->v_sub_i32(t, t, x3);
    pc->v_min_i16(x3, x3, t);
  }

  pc->v_shuffle_f32(x0, x0, x1, x86::Predicate::shuf(1, 0, 1, 0));
  iExt.begin(IndexExtractor::kTypeUInt16, x3);

  pc->v_cvt_f64_f32(x1, d_b);
  pc->v_add_f64(d_b, d_b, dd_bd);

  fCtx.fetchAll(x86::ptr(f->table), srcShift, iExt, srcIndexes, [&](uint32_t step) {
    switch (step) {
      case 0:
        pc->vmovaps(value, x0);
        pc->v_cvt_f64_f32(x2, d_b);
        break;
      case 1:
        pc->s_add_f64(dd_bd, dd_bd, ddd);
        pc->v_shuffle_f32(x1, x1, x2, x86::Predicate::shuf(1, 0, 1, 0));
        break;
      case 2:
        pc->v_shuffle_f32(x0, x0, x1, x86::Predicate::shuf(2, 0, 2, 0));
        pc->v_and_f32(x0, x0, pc->constAsMem(blCommonTable.f128_abs));
        break;
      case 3:
        pc->v_sqrt_f32(x0, x0);
        pc->v_add_f64(d_b, d_b, dd_bd);
        break;
    }
  });

  pc->v_shuffle_f32(value, value, x1, x86::Predicate::shuf(3, 1, 3, 1));
  pc->s_add_f64(dd_bd, dd_bd, ddd);
  fCtx.end();

  pc->xSatisfyPixel(p, flags);
  pc->v_add_f32(value, value, x0);
}

void FetchRadialGradientPart::precalc(x86::Xmm& px_py) noexcept {
  x86::Xmm& d_b   = f->d_b;
  x86::Xmm& dd_bd = f->dd_bd;

  x86::Xmm x0 = cc->newXmmPd("@x0");
  x86::Xmm x1 = cc->newXmmPd("@x1");
  x86::Xmm x2 = cc->newXmmPd("@x2");

  pc->v_mul_f64(d_b, px_py, f->ax_ay);                   // [Ax.Px                             | Ay.Py         ]
  pc->v_mul_f64(x0, px_py, f->fx_fy);                    // [Fx.Px                             | Fy.Py         ]
  pc->v_mul_f64(x1, px_py, f->ddx_ddy);                  // [Ddx.Px                            | Ddy.Py        ]

  pc->v_mul_f64(d_b, d_b, px_py);                        // [Ax.Px^2                           | Ay.Py^2       ]
  pc->v_hadd_f64(d_b, d_b, x0);                          // [Ax.Px^2 + Ay.Py^2                 | Fx.Px + Fy.Py ]

  pc->v_swap_f64(x2, x0);
  pc->s_mul_f64(x2, x2, x0);                             // [Fx.Px.Fy.Py                       | ?             ]
  pc->s_add_f64(x2, x2, x2);                             // [2.Fx.Px.Fy.Py                     | ?             ]
  pc->s_add_f64(d_b, d_b, x2);                           // [Ax.Px^2 + Ay.Py^2 + 2.Fx.Px.Fy.Py | Fx.Px + Fy.Py ]
  pc->s_add_f64(dd_bd, f->da_ba, x1);                    // [Dd + Ddx.Px                       | Bd            ]

  pc->v_swap_f64(x1, x1);
  pc->s_add_f64(dd_bd, dd_bd, x1);                       // [Dd + Ddx.Px + Ddy.Py              | Bd            ]
}

// ============================================================================
// [BLPipeGen::FetchConicalGradientPart - Construction / Destruction]
// ============================================================================

FetchConicalGradientPart::FetchConicalGradientPart(PipeCompiler* pc, uint32_t fetchType, uint32_t fetchPayload, uint32_t format) noexcept
  : FetchGradientPart(pc, fetchType, fetchPayload, format) {

  _maxPixels = 4;
  _maxSimdWidthSupported = 16;

  _isComplexFetch = true;
  _persistentRegs[x86::Reg::kGroupGp] = 1;
  _persistentRegs[x86::Reg::kGroupVec] = 4;
  _temporaryRegs[x86::Reg::kGroupVec] = 6;

  JitUtils::resetVarStruct(&f, sizeof(f));
}

// ============================================================================
// [BLPipeGen::FetchConicalGradientPart - Init / Fini]
// ============================================================================

void FetchConicalGradientPart::_initPart(x86::Gp& x, x86::Gp& y) noexcept {
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  f->table          = cc->newIntPtr("f.table");       // Reg.
  f->xx_xy          = cc->newXmmPd("f.xx_xy");        // Mem.
  f->yx_yy          = cc->newXmmPd("f.yx_yy");        // Mem.
  f->hx_hy          = cc->newXmmPd("f.hx_hy");        // Reg. (TODO: Make spillable).
  f->px_py          = cc->newXmmPd("f.px_py");        // Reg.
  f->consts         = cc->newIntPtr("f.consts");      // Reg.

  f->maxi           = cc->newUInt32("f.maxi");        // Mem.
  f->vmaxi          = cc->newXmm("f.vmaxi");          // Mem.

  f->x0             = cc->newXmmPs("f.x0");           // Reg/Tmp.
  f->x1             = cc->newXmmPs("f.x1");           // Reg/Tmp.
  f->x2             = cc->newXmmPs("f.x2");           // Reg/Tmp.
  f->x3             = cc->newXmmPs("f.x3");           // Reg/Tmp.
  f->x4             = cc->newXmmPs("f.x4");           // Reg/Tmp.
  f->x5             = cc->newXmmPs("f.x5");           // Reg.

  x86::Xmm off      = cc->newXmmPd("f.off");          // Local.
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  cc->mov(f->table, x86::ptr(pc->_fetchData, REL_GRADIENT(lut.data)));

  pc->v_zero_d(f->hx_hy);
  pc->s_cvt_int_f64(f->hx_hy, f->hx_hy, y);

  pc->v_loadu_d128(f->xx_xy, x86::ptr(pc->_fetchData, REL_GRADIENT(conical.xx)));
  pc->v_loadu_d128(f->yx_yy, x86::ptr(pc->_fetchData, REL_GRADIENT(conical.yx)));
  pc->v_loadu_d128(off    , x86::ptr(pc->_fetchData, REL_GRADIENT(conical.ox)));

  pc->v_dupl_f64(f->hx_hy, f->hx_hy);
  pc->v_mul_f64(f->hx_hy, f->hx_hy, f->yx_yy);
  pc->v_add_f64(f->hx_hy, f->hx_hy, off);

  cc->mov(f->consts, x86::ptr(pc->_fetchData, REL_GRADIENT(conical.consts)));

  if (isRectFill()) {
    pc->v_zero_d(off);
    pc->s_cvt_int_f64(off, off, x);
    pc->v_dupl_f64(off, off);
    pc->v_mul_f64(off, off, f->xx_xy);
    pc->v_add_f64(f->hx_hy, f->hx_hy, off);
  }

  // Setup constants used by 4+ pixel fetches.
  if (maxPixels() > 1) {
    f->xx4_xy4 = cc->newXmmPd("f.xx4_xy4"); // Mem.
    f->xx_0123 = cc->newXmmPs("f.xx_0123"); // Mem.
    f->xy_0123 = cc->newXmmPs("f.xy_0123"); // Mem.

    pc->v_cvt_f64_f32(f->xy_0123, f->xx_xy);
    pc->v_mul_f64(f->xx4_xy4, f->xx_xy, pc->constAsMem(blCommonTable.d128_4));

    pc->v_swizzle_i32(f->xx_0123, f->xy_0123, x86::Predicate::shuf(0, 0, 0, 0));
    pc->v_swizzle_i32(f->xy_0123, f->xy_0123, x86::Predicate::shuf(1, 1, 1, 1));

    pc->v_mul_f32(f->xx_0123, f->xx_0123, pc->constAsMem(blCommonTable.f128_3_2_1_0));
    pc->v_mul_f32(f->xy_0123, f->xy_0123, pc->constAsMem(blCommonTable.f128_3_2_1_0));
  }

  pc->v_load_i32(f->vmaxi, x86::ptr(pc->_fetchData, REL_GRADIENT(conical.maxi)));
  pc->vexpandli32(f->vmaxi, f->vmaxi);
  pc->s_mov_i32(f->maxi, f->vmaxi);
}

void FetchConicalGradientPart::_finiPart() noexcept {}

// ============================================================================
// [BLPipeGen::FetchConicalGradientPart - Advance]
// ============================================================================

void FetchConicalGradientPart::advanceY() noexcept {
  pc->v_add_f64(f->hx_hy, f->hx_hy, f->yx_yy);
}

void FetchConicalGradientPart::startAtX(x86::Gp& x) noexcept {
  if (isRectFill()) {
    pc->vmovapd(f->px_py, f->hx_hy);
  }
  else {
    pc->v_zero_d(f->px_py);
    pc->s_cvt_int_f64(f->px_py, f->px_py, x);
    pc->v_dupl_f64(f->px_py, f->px_py);
    pc->v_mul_f64(f->px_py, f->px_py, f->xx_xy);
    pc->v_add_f64(f->px_py, f->px_py, f->hx_hy);
  }
}

void FetchConicalGradientPart::advanceX(x86::Gp& x, x86::Gp& diff) noexcept {
  BL_UNUSED(diff);

  x86::Xmm& hx_hy = f->hx_hy;
  x86::Xmm& px_py = f->px_py;

  if (isRectFill()) {
    pc->vmovapd(px_py, hx_hy);
  }
  else {
    pc->v_zero_d(px_py);
    pc->s_cvt_int_f64(px_py, px_py, x);
    pc->v_dupl_f64(px_py, px_py);
    pc->v_mul_f64(px_py, px_py, f->xx_xy);
    pc->v_add_f64(px_py, px_py, hx_hy);
  }
}

// ============================================================================
// [BLPipeGen::FetchConicalGradientPart - Fetch]
// ============================================================================

void FetchConicalGradientPart::fetch1(Pixel& p, uint32_t flags) noexcept {
  x86::Gp& consts = f->consts;
  x86::Xmm& px_py = f->px_py;
  x86::Xmm& x0 = f->x0;
  x86::Xmm& x1 = f->x1;
  x86::Xmm& x2 = f->x2;
  x86::Xmm& x3 = f->x3;
  x86::Xmm& x4 = f->x4;

  x86::Gp gIdx = cc->newInt32("@gIdx");

  pc->v_cvt_f64_f32(x0, px_py);
  pc->vmovaps(x1, pc->constAsMem(blCommonTable.f128_abs));
  pc->vmovaps(x2, pc->constAsMem(blCommonTable.f128_1e_m20));

  pc->v_and_f32(x1, x1, x0);
  pc->v_add_f64(px_py, px_py, f->xx_xy);

  pc->v_swizzle_i32(x3, x1, x86::Predicate::shuf(2, 3, 0, 1));
  pc->s_max_f32(x2, x2, x1);

  pc->s_max_f32(x2, x2, x3);
  pc->s_min_f32(x3, x3, x1);

  pc->s_cmp_f32(x1, x1, x3, x86::Predicate::kCmpEQ);
  pc->s_div_f32(x3, x3, x2);

  pc->v_sra_i32(x0, x0, 31);
  pc->v_and_f32(x1, x1, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, n_div_4)));

  pc->s_mul_f32(x2, x3, x3);
  pc->v_and_f32(x0, x0, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, n_extra)));

  pc->s_mul_f32(x4, x2, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q3)));
  pc->s_add_f32(x4, x4, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q2)));

  pc->s_mul_f32(x4, x4, x2);
  pc->s_add_f32(x4, x4, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q1)));

  pc->s_mul_f32(x2, x2, x4);
  pc->s_add_f32(x2, x2, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q0)));

  pc->s_mul_f32(x2, x2, x3);
  pc->s_sub_f32(x2, x2, x1);

  pc->v_swizzle_f32(x1, x0, x86::Predicate::shuf(2, 3, 0, 1));
  pc->v_and_f32(x2, x2, pc->constAsMem(blCommonTable.f128_abs));

  pc->s_sub_f32(x2, x2, x0);
  pc->v_and_f32(x2, x2, pc->constAsMem(blCommonTable.f128_abs));

  pc->s_sub_f32(x2, x2, x1);
  pc->v_and_f32(x2, x2, pc->constAsMem(blCommonTable.f128_abs));
  pc->s_cvtt_f32_int(gIdx, x2);
  cc->and_(gIdx.r32(), f->maxi.r32());

  fetchGradientPixel1(p, flags, x86::ptr(f->table, gIdx, 2));
  pc->xSatisfyPixel(p, flags);
}

void FetchConicalGradientPart::prefetchN() noexcept {
  x86::Gp& consts = f->consts;
  x86::Xmm& px_py = f->px_py;
  x86::Xmm& x0 = f->x0;
  x86::Xmm& x1 = f->x1;
  x86::Xmm& x2 = f->x2;
  x86::Xmm& x3 = f->x3;
  x86::Xmm& x4 = f->x4;
  x86::Xmm& x5 = f->x5;

  pc->v_cvt_f64_f32(x1, px_py);
  pc->vmovaps(x2, pc->constAsMem(blCommonTable.f128_abs));

  pc->v_swizzle_f32(x0, x1, x86::Predicate::shuf(0, 0, 0, 0));
  pc->v_swizzle_f32(x1, x1, x86::Predicate::shuf(1, 1, 1, 1));

  pc->v_add_f32(x0, x0, f->xx_0123);
  pc->v_add_f32(x1, x1, f->xy_0123);

  pc->vmovaps(x4, pc->constAsMem(blCommonTable.f128_1e_m20));
  pc->v_and_f32(x3, x2, x1);
  pc->v_and_f32(x2, x2, x0);

  pc->v_max_f32(x4, x4, x2);
  pc->v_max_f32(x4, x4, x3);
  pc->v_min_f32(x3, x3, x2);

  pc->v_cmp_f32(x2, x2, x3, x86::Predicate::kCmpEQ);
  pc->v_div_f32(x3, x3, x4);

  pc->v_sra_i32(x0, x0, 31);
  pc->v_and_f32(x2, x2, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, n_div_4)));

  pc->v_sra_i32(x1, x1, 31);
  pc->v_and_f32(x0, x0, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, n_div_2)));

  pc->v_mul_f32(x5, x3, x3);
  pc->v_and_f32(x1, x1, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, n_div_1)));

  pc->v_mul_f32(x4, x5, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q3)));
  pc->v_add_f32(x4, x4, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q2)));

  pc->v_mul_f32(x4, x4, x5);
  pc->v_add_f32(x4, x4, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q1)));

  pc->v_mul_f32(x5, x5, x4);
  pc->v_add_f32(x5, x5, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q0)));

  pc->v_mul_f32(x5, x5, x3);
  pc->v_sub_f32(x5, x5, x2);

  pc->v_and_f32(x5, x5, pc->constAsMem(blCommonTable.f128_abs));

  pc->v_sub_f32(x5, x5, x0);
  pc->v_and_f32(x5, x5, pc->constAsMem(blCommonTable.f128_abs));

  pc->v_sub_f32(x5, x5, x1);
  pc->v_and_f32(x5, x5, pc->constAsMem(blCommonTable.f128_abs));
}

void FetchConicalGradientPart::fetch4(Pixel& p, uint32_t flags) noexcept {
  x86::Gp& consts = f->consts;
  x86::Xmm& px_py = f->px_py;
  x86::Xmm& x0 = f->x0;
  x86::Xmm& x1 = f->x1;
  x86::Xmm& x2 = f->x2;
  x86::Xmm& x3 = f->x3;
  x86::Xmm& x4 = f->x4;
  x86::Xmm& x5 = f->x5;

  x86::Gp idx0 = cc->newInt32("@idx0");
  x86::Gp idx1 = cc->newInt32("@idx1");

  FetchContext fCtx(pc, &p, 4, format(), flags);
  IndexExtractor iExt(pc);

  pc->v_add_f64(px_py, px_py, f->xx4_xy4);
  pc->v_and_f32(x5, x5, pc->constAsMem(blCommonTable.f128_abs));

  pc->v_cvt_f64_f32(x1, px_py);
  pc->vmovaps(x2, pc->constAsMem(blCommonTable.f128_abs));

  pc->v_swizzle_f32(x0, x1, x86::Predicate::shuf(0, 0, 0, 0));
  pc->v_swizzle_f32(x1, x1, x86::Predicate::shuf(1, 1, 1, 1));

  pc->v_add_f32(x0, x0, f->xx_0123);
  pc->v_add_f32(x1, x1, f->xy_0123);

  pc->vmovaps(x4, pc->constAsMem(blCommonTable.f128_1e_m20));
  pc->v_and_f32(x3, x2, x1);
  pc->v_and_f32(x2, x2, x0);

  pc->v_max_f32(x4, x4, x2);
  pc->v_cvtt_f32_i32(x5, x5);

  pc->v_max_f32(x4, x4, x3);
  pc->v_min_f32(x3, x3, x2);

  pc->v_cmp_f32(x2, x2, x3, x86::Predicate::kCmpEQ);
  pc->v_and(x5, x5, f->vmaxi);
  pc->v_div_f32(x3, x3, x4);

  iExt.begin(IndexExtractor::kTypeUInt16, x5);
  pc->v_sra_i32(x0, x0, 31);
  pc->v_and_f32(x2, x2, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, n_div_4)));
  iExt.extract(idx0, 0);

  pc->v_sra_i32(x1, x1, 31);
  pc->v_and_f32(x0, x0, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, n_div_2)));
  iExt.extract(idx1, 2);

  fCtx.fetchPixel(x86::ptr(f->table, idx0, 2));
  iExt.extract(idx0, 4);
  pc->v_mul_f32(x4, x3, x3);

  fCtx.fetchPixel(x86::ptr(f->table, idx1, 2));
  iExt.extract(idx1, 6);

  pc->vmovaps(x5, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q3)));
  pc->v_mul_f32(x5, x5, x4);
  pc->v_and_f32(x1, x1, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, n_div_1)));
  pc->v_add_f32(x5, x5, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q2)));
  pc->v_mul_f32(x5, x5, x4);
  fCtx.fetchPixel(x86::ptr(f->table, idx0, 2));

  pc->v_add_f32(x5, x5, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q1)));
  pc->v_mul_f32(x5, x5, x4);
  pc->v_add_f32(x5, x5, x86::ptr(consts, BL_OFFSET_OF(BLCommonTable::Conical, q0)));
  pc->v_mul_f32(x5, x5, x3);
  fCtx.fetchPixel(x86::ptr(f->table, idx1, 2));

  pc->v_sub_f32(x5, x5, x2);
  pc->v_and_f32(x5, x5, pc->constAsMem(blCommonTable.f128_abs));
  pc->v_sub_f32(x5, x5, x0);

  fCtx.end();
  pc->v_and_f32(x5, x5, pc->constAsMem(blCommonTable.f128_abs));

  pc->xSatisfyPixel(p, flags);
  pc->v_sub_f32(x5, x5, x1);
}

} // {BLPipeGen}

#endif
