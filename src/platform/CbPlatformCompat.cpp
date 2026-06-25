// CbPlatformCompat.cpp -- see header.

#include "stdafx.h"

#include "CbPlatformCompat.h"

#include <cstdarg>
#include <cstdio>

namespace {
void* g_mainHwnd = nullptr;
}

void  Cb_SetMainHwnd(void* hwnd) { g_mainHwnd = hwnd; }
void* Cb_GetMainHwnd()           { return g_mainHwnd; }

void* Cb_OwnerHwnd()             { return g_mainHwnd; }

void Cb_BeginWaitCursor()        { ::SetCursor(::LoadCursorA(NULL, IDC_WAIT)); }
void Cb_EndWaitCursor()          { ::SetCursor(::LoadCursorA(NULL, IDC_ARROW)); }

#ifdef _DEBUG
void Cb_Trace(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    _vsnprintf_s(buf, _TRUNCATE, fmt, args);
    va_end(args);
    ::OutputDebugStringA(buf);
}
#endif

// (AfxGetMainWnd / AfxGetApp removed 2026-06-12 -- replaced by Cb_OwnerHwnd /
// Cb_BeginWaitCursor / Cb_EndWaitCursor above.)
