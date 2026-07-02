// WinMain.cpp -- the EXE entry point since the Qt-shell switch (2026-06-09).
//
// Deliberately tiny and Qt-free (this TU compiles in the MultiByte model
// world without Qt include paths): it parses the command line and hands off
// to Cb_RunQtShell in the ClassBuilderQt static lib, which owns the
// QApplication, the shell window, and the pipe command server.

#include "StdAfx.h"

#include "qt/QtShell.h"

#if defined(__linux__)
#include <cstdlib>   // getenv / setenv -- Qt-free platform default (see main() below)
#endif

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/,
                     LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    // __argc/__argv: the CRT's pre-parsed ANSI argv (handles quoting). A
    // single file argument = the .cbz to open (shell association /
    // double-click), same contract as the old MFC shell.
    const char* fileToOpen = (__argc > 1) ? __argv[1] : nullptr;

    return Cb_RunQtShell(fileToOpen);
}
#else
// macOS / Linux: standard C entry point. Same contract -- one optional file
// argument = the .cbz to open -- handing off to the same Cb_RunQtShell in the
// ClassBuilderQt static lib.
int main(int argc, char** argv)
{
#if defined(__linux__)
    // CB manages its own child-window placement (MDI / floating tool windows),
    // which native Wayland forbids -- windows can't be dragged there. Default to
    // the XCB backend (X11 / XWayland) where that works. Guards: only under a
    // Wayland session (WAYLAND_DISPLAY) with XWayland actually available
    // (DISPLAY), and never override an explicit user choice (QT_QPA_PLATFORM).
    if (!getenv("QT_QPA_PLATFORM") && getenv("WAYLAND_DISPLAY") && getenv("DISPLAY"))
        setenv("QT_QPA_PLATFORM", "xcb", 0);
#endif

    const char* fileToOpen = (argc > 1) ? argv[1] : nullptr;

    return Cb_RunQtShell(fileToOpen);
}
#endif
