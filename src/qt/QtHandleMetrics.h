// qt/QtHandleMetrics.h -- selection-handle metrics shared by the diagram canvases
// (ClassDiagram + SequenceDiagram) and CbPainter_QPainter that draws the handles.
// Defined in ONE place so the visible handle size and the (deliberately larger)
// grab tolerance stay consistent across the views and can't drift.
#pragma once

#include <QtGlobal>   // qMax, qRound

namespace QtHandle {

// Visible handle: a square drawn by CbPainter_QPainter::DrawSelectionHandle.
// Rather than a fixed device size (which dominates the shrinking shapes when you
// zoom out), the side eases with the apparent scale: it is kVisibleSizeModelUnits
// model units across, CAPPED at kVisibleSizeMaxDev so it stays a constant,
// grabbable square when zoomed in, and floored at kVisibleSizeMinDev so it never
// vanishes when zoomed far out. So the handle:shape ratio stays roughly steady.
constexpr double kVisibleSizeMaxDev    = 7.0;   // cap (zoomed in) -- the old fixed size
constexpr double kVisibleSizeMinDev    = 4.0;   // floor (zoomed far out)
constexpr double kVisibleSizeModelUnits = 14.0; // handle side in model units before clamping

// Visible handle side in DEVICE pixels for the current device-per-model scale
// (abs of the world transform's scale term). Clamped to [min, max].
inline double visibleSizeDev(double devicePerModel)
{
    const double sized = devicePerModel * kVisibleSizeModelUnits;
    return sized < kVisibleSizeMinDev ? kVisibleSizeMinDev
         : sized > kVisibleSizeMaxDev ? kVisibleSizeMaxDev
         : sized;
}

// Grab tolerance: a hit accepts a point within this many DEVICE pixels of the
// handle centre on each axis -- intentionally a bit larger than the visible
// handle so it's easy to grab. A model-unit floor keeps it usable when zoomed far
// out.
constexpr double kGrabRadiusDev      = 6.0;
constexpr int    kGrabRadiusModelMin = 3;

// Grab tolerance in MODEL units for a device-per-model scale (fitScale * zoom).
// Used by the canvas hover / hit-tests.
inline int grabToleranceModel(double devicePerModel)
{
    return devicePerModel > 1e-6
        ? qMax(kGrabRadiusModelMin, qRound(kGrabRadiusDev / devicePerModel))
        : qRound(kGrabRadiusDev);
}

} // namespace QtHandle
