// CbMessageBox.h -- MFC/Win32-free message box for the model layer.
//
// Isolates the model from GUI/MFC vocabulary: model code calls
//     CbMessageBox(text, CBMB_ICONQUESTION | CBMB_YESNO)
// and compares the result against CBMB_IDYES / CBMB_IDOK / ... -- no
// AfxMessageBox, no MB_*/ID*, no windows.h. The Qt backend (qt/QtMessageBox.cpp)
// maps CBMB_* to a QMessageBox.
//
// The CBMB_* values are deliberately IDENTICAL to the Win32 MB_*/ID* numbers so
// the migration off AfxMessageBox is value-compatible: during the transition the
// old AfxMessageBox(text, MB_*) calls and the new CbMessageBox(text, CBMB_*)
// calls pass the same integer to the same backend. (For the Mac/Linux port the
// values are ours, not windows.h's -- this header has no Win32 dependency.)
//
// Deliberately MFC-free AND Qt-free: included from the MultiByte model lib.
#pragma once

// --- button-set flags (low nibble) ---
enum
{
    CBMB_OK              = 0x0000,
    CBMB_OKCANCEL        = 0x0001,
    CBMB_YESNOCANCEL     = 0x0003,
    CBMB_YESNO           = 0x0004,
    CBMB_TYPEMASK        = 0x000F,

    // --- icon flags ---
    CBMB_ICONSTOP        = 0x0010,   // == CBMB_ICONERROR (same Win32 value)
    CBMB_ICONERROR       = 0x0010,
    CBMB_ICONQUESTION    = 0x0020,
    CBMB_ICONEXCLAMATION = 0x0030,
    CBMB_ICONINFORMATION = 0x0040,
    CBMB_ICONMASK        = 0x00F0,

    // --- default-button flags ---
    CBMB_DEFBUTTON1      = 0x0000,
    CBMB_DEFBUTTON2      = 0x0100,
    CBMB_DEFMASK         = 0x0F00,
};

// --- result codes ---
enum
{
    CBMB_IDOK     = 1,
    CBMB_IDCANCEL = 2,
    CBMB_IDYES    = 6,
    CBMB_IDNO     = 7,
};

// MFC/Win32-free message box. `type` is a CBMB_* flag combination; the return
// value is a CBMB_ID* code. Backed by Qt (qt/QtMessageBox.cpp).
int CbMessageBox(const char* text, unsigned int type = CBMB_OK);
