// qt/QtDiagramZoom.h -- route the main-frame Zoom toolbar buttons to the
// most-recently-activated Qt diagram view (CD or SD).
//
// The Zoom In/Out/Full buttons historically zoomed the MFC main tree (icon
// size). The tree zoom is dropped (gimmick); the buttons now forward to the
// active Qt diagram canvas, which already has real zoom (Ctrl+wheel / Ctrl+=
// / Ctrl+-). The Qt views are top-level owned popups outside MFC's command
// routing, so the MFC side cannot reach them directly: each diagram view
// registers its canvas here when its window activates, and CMainFrame's
// handlers call Qt_DiagramZoom on whatever registered last.
//
// MFC-includable: no Qt headers (QWidget is forward-declared only).
#pragma once

class QWidget;

// Apply a zoom operation to `canvas`: +1 step in, -1 step out, 0 reset to
// fit-to-window. The thunk gives the registry one call signature over both
// canvas classes without a shared base.
typedef void (*DiagramZoomApplyFn)(QWidget* canvas, int op);

// Called by ClassDiagramQtView / SequenceDiagramQtView on WindowActivate.
// The registry holds the canvas weakly (QPointer) -- a closed view simply
// drops out, no unregister call needed.
void Qt_SetActiveDiagramZoom(QWidget* canvas, DiagramZoomApplyFn apply);

// True while a registered diagram canvas is alive (drives the toolbar
// buttons' enable state).
bool Qt_DiagramZoomAvailable();

// Forward op (+1 / -1 / 0, see above) to the registered canvas. No-op when
// none is alive.
void Qt_DiagramZoom(int op);
