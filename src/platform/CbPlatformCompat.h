// CbPlatformCompat.h -- the ONE place the model code touches the window system.
//
// Everything platform-flavoured the ~150 generated .cpp files need lives
// behind these few Cb_* calls (owner handle for dialogs, wait cursor, TRACE,
// CbMessageBox via CbMessageBox.h). The implementations are plain Win32 for
// now; porting them means changing ONLY CbPlatformCompat.cpp. History: the bodies
// originally used MFC idioms (AfxMessageBox -> CbMessageBox 2026-06-10;
// AfxGetMainWnd/AfxGetApp call sites -> Cb_OwnerHwnd/Cb_*WaitCursor
// 2026-06-12). The Qt shell calls Cb_SetMainHwnd() at startup so owner
// handles / message boxes attach to the QMainWindow.
//
// Deliberately MFC-free AND Qt-free: included from stdafx.h into every model
// translation unit.
#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include "CbMessageBox.h"   // CbMessageBox + CBMB_* (the AfxMessageBox replacement)

// Set by the Qt shell once its main window exists (and cleared on teardown).
void  Cb_SetMainHwnd(void* hwnd);
void* Cb_GetMainHwnd();

// The shell main window as an owner handle for dialogs (void* HWND; null
// before the shell window exists). THE one-liner for every dialog-opening
// body -- replaces the scattered MFC idiom
//   AfxGetMainWnd() ? AfxGetMainWnd()->GetSafeHwnd() : nullptr
void* Cb_OwnerHwnd();

// Busy/normal mouse cursor for long operations -- replaces the scattered MFC
// idiom SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT / IDC_ARROW)).
// Win32 today; becomes QGuiApplication::setOverrideCursor/restoreOverrideCursor
// in the all-Qt end state by changing ONLY these two bodies.
void Cb_BeginWaitCursor();
void Cb_EndWaitCursor();

#ifndef TRACE
// Debug-trace used in a couple of model bodies; route to OutputDebugString in
// debug builds, compile out in release (mirrors the old MFC TRACE).
#ifdef _DEBUG
void Cb_Trace(const char* fmt, ...);
#define TRACE Cb_Trace
#else
#define TRACE(...) ((void)0)
#endif
#endif
#ifndef TRACE0
#define TRACE0(s) TRACE("%s", s)
#endif

// (AfxMessageBox removed 2026-06-10 -- the model now calls CbMessageBox directly;
// CbMessageBox + CBMB_* come from CbMessageBox.h, included above.)
//
// (AfxGetMainWnd / AfxGetApp shim classes removed 2026-06-12 -- every call
// site now uses Cb_OwnerHwnd() / Cb_BeginWaitCursor() / Cb_EndWaitCursor()
// above, so no Afx name remains outside this header's history notes.)
