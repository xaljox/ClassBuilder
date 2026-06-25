// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
// MFC-FREE since the Qt-shell switch (2026-06-09): the afxwin/afxext/afxcview
// block is gone. The model code still compiles against Win32 vocabulary
// (UINT / COLORREF / RGB / MB_* / IDC_*; BOOL itself is gone since the
// 2026-06-10 bool conversion), so windows.h stays, plus the CbPlatformCompat
// shims for the few Afx idioms baked into generated bodies.

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers
#define NOMINMAX            // keep windows.h's min/max macros out of the way

#include <iosfwd>           // std::ostream / std::istream forward declarations

#include <windows.h>

#include "CbPlatformCompat.h"      // AfxGetMainWnd / AfxGetApp / TRACE shims (+ CbMessageBox via CbMessageBox.h)

#include "CbString.h"         // MFC-free string -- replaces CString in the model

#include "CbSerialize.h"

#include "ClassBuilderInclude.h"
