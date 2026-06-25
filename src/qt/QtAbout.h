// qt/QtAbout.h -- bridge from the MFC app code into the Qt About dialog.
//
// This header deliberately pulls in NO Qt headers, so ClassBuilder.cpp
// (a pure-MFC translation unit, afxwin.h + the DEBUG_NEW macro) can include
// it without ever dragging QtWidgets into an MFC compile. All the Qt code
// lives behind this one C++ function, defined in AboutDialog.cpp.
//
// First step of the MFC->Qt port: a single self-contained modal dialog.
#pragma once

// Shows the modal "About ClassBuilder" dialog. Lazily creates the process's
// one QApplication on first call; safe to call from an MFC command handler.
// ownerHwnd is the MFC owner window (an HWND, passed as void* to keep this
// header MFC-free); the dialog is made truly modal over it.
void Qt_ShowAboutDialog(void* ownerHwnd);
