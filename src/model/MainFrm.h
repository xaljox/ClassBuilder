// MainFrm.h : MFC-free facade of the old CMainFrame class.
//
// The MFC CMDIFrameWnd main frame is gone (Qt shell switch, 2026-06-09); the
// Qt main window is qt/QtShellWindow. This header keeps the CMainFrame NAME
// alive because model translation units include it for two things:
//
//   - CMainFrame::s_loadInProgress -- the re-entrant "document is mid-load"
//     guard checked before CheckUpdates-style model walks.
//   - the static status-bar coordinate setters (SetCoordinate / SetRect /
//     SetWidth / ...) the diagram views once fed. They forward to a hook the
//     Qt shell may install (status-bar panes); without a hook they are no-ops.
//
// No windows, no message map, nothing to construct.

#pragma once

#include "CbGeometry.h"

class CMainFrame
{
public:
	// Optional hook into the Qt shell's status bar: pane 0/1 = width/height,
	// pane 2/3 = X/Y. Null entries mean "no display" (the default).
	typedef void (*StatusTextFn)(int pane, const char* text);
	static StatusTextFn s_statusTextFn;

	static void SetCoordinateX(int x);
	static void SetCoordinateX(const CbPoint& point) { SetCoordinateX(point.x); }

	static void SetCoordinateY(int y);
	static void SetCoordinateY(const CbPoint& point) { SetCoordinateY(point.y); }

	static void SetCoordinate(int x, int y);
	static void SetCoordinate(const CbPoint& point) { SetCoordinate(point.x, point.y); }

	static void ResetCoordinate();

	static void SetWidth(int width);
	static void ResetWidth();

	static void SetHeight(int height);
	static void ResetHeight();

	static void SetRect(const CbRect& rect)
	{
		SetCoordinate(rect.left, rect.bottom);
		SetWidth(rect.Width());
		SetHeight(rect.Height());
	}
	static void ResetRect()
	{
		ResetCoordinate();
		ResetWidth();
		ResetHeight();
	}

	// Re-entrant guard. Incremented by CClassBuilderDoc around code paths
	// that mutate doc state under the message pump (file open, new doc).
	// The app-activate CheckUpdates walk skips entirely while > 0 — a
	// half-loaded doc has un-wired DataModel / partially-restored relations
	// that would crash when traversed. Counter (not bool) so nested loads
	// work correctly.
	static int s_loadInProgress;
};
