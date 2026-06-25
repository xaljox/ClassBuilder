// qt/QtShell.h -- entry point of the Qt application shell.
//
// MFC-free and Qt-free: included by the EXE's WinMain TU (which is compiled
// in the MultiByte model world, without Qt include paths). The implementation
// lives in qt/QtShellWindow.cpp inside the ClassBuilderQt static lib.
#pragma once

// Runs the whole app: QApplication, the shell main window, the pipe command
// server, optional initial document. Returns the process exit code.
// `fileToOpen` may be null/empty (start without a document).
int Cb_RunQtShell(const char* fileToOpen);
