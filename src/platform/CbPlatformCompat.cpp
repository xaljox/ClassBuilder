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

#ifdef _WIN32

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

#else  // macOS / Linux -- Qt-backed bodies (this TU is in the model world, but
       // the platform tier is allowed to pull in Qt for the non-Windows seam).

#include <QGuiApplication>
#include <QCursor>
#include <QtGlobal>   // qDebug

void Cb_BeginWaitCursor() { QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor)); }
void Cb_EndWaitCursor()   { QGuiApplication::restoreOverrideCursor(); }

#ifdef _DEBUG
void Cb_Trace(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    qDebug("%s", buf);
}
#endif

#endif

// (AfxGetMainWnd / AfxGetApp removed 2026-06-12 -- replaced by Cb_OwnerHwnd /
// Cb_BeginWaitCursor / Cb_EndWaitCursor above.)
