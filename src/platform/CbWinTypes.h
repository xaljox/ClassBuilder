// CbWinTypes.h -- windows.h-free Win32 vocabulary for the non-Windows port.
//
// The MFC-free model still compiles against a thin slice of Win32 vocabulary:
// a handful of scalar typedefs (UINT/DWORD/...), the RGB() colour macro, and
// the GDI constants the painter abstraction mirrors (PS_*, TA_*, ETO_*,
// TRANSPARENT/OPAQUE, DT_*, MM_*, NULL_BRUSH). On Windows these come from
// <windows.h>; on macOS/Linux this header supplies them instead.
//
// IMPORTANT: the numeric values below are the REAL Win32 values. The shared Qt
// painter backend (CbPainter_QPainter) reads these same constants on every
// platform, so matching the Win32 values keeps its behaviour identical to the
// Windows CDC backend. Do not renumber.
//
// Included (on non-Windows only) by StdAfx.h and CbPainter.h in place of
// <windows.h>. Window/system *handles* are not real here -- HWND/HINSTANCE are
// opaque void* only so the few residual declarations compile; the actual window
// system is reached through CbPlatformCompat / the Qt layer, never these.
#pragma once

#ifndef _WIN32

#include <cstdint>
#include <cctype>     // isalnum (for __iscsym)
#include <cstdio>     // snprintf (for sprintf_s)
#include <sys/stat.h> // struct stat / stat (for _stat)
#include <unistd.h>   // chdir / getcwd (POSIX equivalents of <direct.h>)

// --- MSVC CRT spellings -> POSIX -----------------------------------------
// __iscsym: MSVC predicate "valid C identifier character" (alnum or '_').
#ifndef __iscsym
#  define __iscsym(c) (isalnum((unsigned char)(c)) || (c) == '_')
#endif

// _stat: one macro covers BOTH `struct _stat` -> `struct stat` and the
// `_stat(path,&buf)` call -> `stat(...)`. st_mtime etc. are the same members.
#ifndef _stat
#  define _stat stat
#endif

// sprintf_s(buffer, fmt, ...): the MSVC bounded form deduces the buffer size
// from the array. The call sites all pass a stack char[] array, so sizeof()
// gives the true capacity -> snprintf is the exact POSIX equivalent.
#ifndef sprintf_s
#  define sprintf_s(buf, ...) snprintf((buf), sizeof(buf), __VA_ARGS__)
#endif

// The model's codegen calls the MSVC CRT names (_chdir before writing bare
// filenames). Map them to the POSIX spellings so the call sites stay identical
// across platforms (single codebase, no per-call #ifdef).
#ifndef _chdir
#  define _chdir  chdir
#endif
#ifndef _getcwd
#  define _getcwd getcwd
#endif

// --- Scalar typedefs ------------------------------------------------------
typedef unsigned int       UINT;
typedef unsigned long      DWORD;
typedef unsigned short     WORD;
typedef unsigned char      BYTE;
typedef long               LONG;
typedef unsigned long      ULONG;
typedef int                BOOL;
typedef intptr_t           LONG_PTR;
typedef uintptr_t          UINT_PTR;
typedef intptr_t           LPARAM;
typedef uintptr_t          WPARAM;
typedef intptr_t           LRESULT;

typedef char*              LPSTR;
typedef const char*        LPCSTR;
typedef const char*        LPCTSTR;   // model is a MultiByte build: TCHAR == char

typedef void*              HANDLE;
typedef void*              HWND;
typedef void*              HINSTANCE;

typedef DWORD              COLORREF;

#ifndef TRUE
#  define TRUE  1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

// --- Colour macros (0x00BBGGRR, the COLORREF byte layout) -----------------
#ifndef RGB
#  define RGB(r,g,b) ((COLORREF)(((BYTE)(r))|(((WORD)((BYTE)(g)))<<8)|(((DWORD)((BYTE)(b)))<<16)))
#endif
#ifndef GetRValue
#  define GetRValue(c) ((BYTE)((c)        & 0xFF))
#endif
#ifndef GetGValue
#  define GetGValue(c) ((BYTE)(((c) >> 8) & 0xFF))
#endif
#ifndef GetBValue
#  define GetBValue(c) ((BYTE)(((c) >> 16)& 0xFF))
#endif

// --- GDI constants (real Win32 values; see header note) -------------------
// Pen styles (wingdi.h)
#define PS_SOLID       0
#define PS_DASH        1
#define PS_DOT         2
#define PS_DASHDOT     3
#define PS_DASHDOTDOT  4
#define PS_NULL        5

// Background mode (SetBkMode)
#define TRANSPARENT 1
#define OPAQUE      2

// Text alignment (SetTextAlign)
#define TA_NOUPDATECP 0
#define TA_UPDATECP   1
#define TA_LEFT       0
#define TA_RIGHT      2
#define TA_CENTER     6
#define TA_TOP        0
#define TA_BOTTOM     8
#define TA_BASELINE   24

// ExtTextOut options
#define ETO_OPAQUE  0x0002
#define ETO_CLIPPED 0x0004

// Stock brush index (SelectStockObject)
#define NULL_BRUSH 5

// Mapping modes (SetMapMode)
#define MM_TEXT      1
#define MM_ISOTROPIC 7

// DrawText format flags
#define DT_LEFT             0x00000000
#define DT_CENTER           0x00000001
#define DT_RIGHT            0x00000002
#define DT_TOP              0x00000000
#define DT_VCENTER          0x00000004
#define DT_BOTTOM           0x00000008
#define DT_WORDBREAK        0x00000010
#define DT_SINGLELINE       0x00000020
#define DT_EXPANDTABS       0x00000040
#define DT_NOCLIP           0x00000100
#define DT_EXTERNALLEADING  0x00000200
#define DT_CALCRECT         0x00000400
#define DT_NOPREFIX         0x00000800

#endif // !_WIN32
