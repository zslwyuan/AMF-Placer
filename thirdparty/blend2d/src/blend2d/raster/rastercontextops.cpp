// [Blend2D]
// 2D Vector Graphics Powered by a JIT Compiler.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#include "../api-build_p.h"
#include "../path_p.h"
#include "../pathstroke_p.h"
#include "../raster/edgebuilder_p.h"
#include "../raster/rastercontextops_p.h"
#include "../raster/rasterworkdata_p.h"

// ============================================================================
// [BLRasterContext - Edge Building Utilities]
// ============================================================================

template<typename PointType>
static BL_INLINE BLResult blRasterContextBuildPolyEdgesT(
  BLRasterWorkData* workData,
  const PointType* pts, size_t size, const BLMatrix2D& m, uint32_t mType) noexcept {

  BLResult result = workData->edgeBuilder.initFromPoly(pts, size, m, mType);
  if (BL_LIKELY(result == BL_SUCCESS))
    return result;

  workData->revertEdgeBuilder();
  return workData->accumulateError(result);
}

BLResult blRasterContextBuildPolyEdges(BLRasterWorkData* workData, const BLPointI* pts, size_t size, const BLMatrix2D& m, uint32_t mType) noexcept {
  return blRasterContextBuildPolyEdgesT(workData, pts, size, m, mType);
}

BLResult blRasterContextBuildPolyEdges(BLRasterWorkData* workData, const BLPoint* pts, size_t size, const BLMatrix2D& m, uint32_t mType) noexcept {
  return blRasterContextBuildPolyEdgesT(workData, pts, size, m, mType);
}

BLResult blRasterContextBuildPathEdges(BLRasterWorkData* workData, const BLPathView& pathView, const BLMatrix2D& m, uint32_t mType) noexcept {
  BLResult result = workData->edgeBuilder.initFromPath(pathView, true, m, mType);
  if (BL_LIKELY(result == BL_SUCCESS))
    return result;

  workData->revertEdgeBuilder();
  return workData->accumulateError(result);
}

// ============================================================================
// [BLRasterContext - Sinks & Sink Utilities]
// ============================================================================

BLResult blRasterContextFillGlyphRunSinkFunc(BLPathCore* path, const void* info, void* closure_) noexcept {
  BL_UNUSED(info);

  BLRasterContextEdgeBuilderSink* sink = static_cast<BLRasterContextEdgeBuilderSink*>(closure_);
  BLEdgeBuilder<int>* edgeBuilder = sink->edgeBuilder;

  BL_PROPAGATE(edgeBuilder->addPath(path->impl->view, true, blMatrix2DIdentity, BL_MATRIX2D_TYPE_IDENTITY));
  return blDownCast(path)->clear();
}

BLResult blRasterContextStrokeGeometrySinkFunc(BLPath* a, BLPath* b, BLPath* c, void* closure_) noexcept {
  BLRasterContextStrokeSink* self = static_cast<BLRasterContextStrokeSink*>(closure_);
  BLEdgeBuilder<int>* edgeBuilder = self->edgeBuilder;

  BL_PROPAGATE(edgeBuilder->addPath(a->view(), false, *self->matrix, self->matrixType));
  BL_PROPAGATE(edgeBuilder->flipSign());
  BL_PROPAGATE(edgeBuilder->addPath(b->view(), false, *self->matrix, self->matrixType));
  BL_PROPAGATE(edgeBuilder->flipSign());

  if (!c->empty())
    BL_PROPAGATE(edgeBuilder->addPath(c->view(), false, *self->matrix, self->matrixType));

  return a->clear();
}

BLResult blRasterContextStrokeGlyphRunSinkFunc(BLPathCore* path, const void* info, void* closure_) noexcept {
  BL_UNUSED(info);

  BLRasterContextStrokeGlyphRunSink* sink = static_cast<BLRasterContextStrokeGlyphRunSink*>(closure_);
  BLPath* a = &sink->paths[0];
  BLPath* b = &sink->paths[1];
  BLPath* c = &sink->paths[2];

  a->clear();
  BLResult localResult = blPathStrokeInternal(
    blDownCast(path)->view(),
    *sink->strokeOptions,
    *sink->approximationOptions,
    a, b, c,
    blRasterContextStrokeGeometrySinkFunc, sink);

  // We must clear the input path, because glyph outlines are appended to it
  // and we just just consumed its content. If we haven't cleared it then th
  // next time we would process the same data that we have already processed.
  blPathClear(path);

  return localResult;
}
