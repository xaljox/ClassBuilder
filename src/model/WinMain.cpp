// WinMain.cpp -- the EXE entry point since the Qt-shell switch (2026-06-09).
//
// Deliberately tiny and Qt-free (this TU compiles in the MultiByte model
// world without Qt include paths): it parses the command line and hands off
// to Cb_RunQtShell in the ClassBuilderQt static lib, which owns the
// QApplication, the shell window, and the pipe command server.

#include "stdafx.h"

#include "qt/QtShell.h"

int APIENTRY WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
                     LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    // __argc/__argv: the CRT's pre-parsed ANSI argv (handles quoting). A
    // single file argument = the .cbz to open (shell association /
    // double-click), same contract as the old MFC shell.
    const char* fileToOpen = (__argc > 1) ? __argv[1] : nullptr;

    return Cb_RunQtShell(fileToOpen);
}
