// CbColor.h -- MFC-/windows.h-free packed colour type for ClassBuilder.
//
// Replaces the Win32 COLORREF (and its RGB()/GetRValue macros) in the model
// and the painter abstraction, so neither needs windows.h for colours. The
// value is a 32-bit 0x00BBGGRR word -- the SAME byte layout COLORREF used, so
// existing .cbz files (which serialized colours as a 4-byte word) stay
// readable. `unsigned int` is 4 bytes on every target (Win32/64, Linux, mac),
// so the wire size is stable across platforms.
//
// The model's CbColorRef OtherType declaration field is just `#include
// "CbColor.h"`, so generated code and the hand-written painter share one
// definition. Header guard (not just the model's emit-once) so a translation
// unit that pulls it in via both the master model header and CbPainter.h
// doesn't double-define the helpers.
#pragma once

typedef unsigned int CbColorRef;

inline CbColorRef Cb_RGB(int r, int g, int b)
{
    return (CbColorRef)((r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16));
}

inline int Cb_GetR(CbColorRef c) { return (int)( c        & 0xFF); }
inline int Cb_GetG(CbColorRef c) { return (int)((c >>  8) & 0xFF); }
inline int Cb_GetB(CbColorRef c) { return (int)((c >> 16) & 0xFF); }
