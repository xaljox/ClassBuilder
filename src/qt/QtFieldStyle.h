// qt/QtFieldStyle.h -- draw multi-line text fields with the SINGLE-line field's
// frame, so both kinds of input field look identical (focus included).
#pragma once

// Install app-wide, once, right after the QApplication exists.
//
// A QLineEdit's frame is drawn by the style's CONTROL primitive
// (PE_PanelLineEdit): rounded, focus-aware, with an inner contrast line. A
// QTextEdit / QPlainTextEdit is a QAbstractScrollArea, so its frame goes
// through the generic QFrame path (PE_Frame) -- which Fusion draws as one flat
// line and, measured, IGNORES the State_HasFocus it is handed. That is a gap in
// the style's emulation, not a difference the user should see: both widgets are
// "a field you type in", and Windows marks focus on both.
//
// So instead of approximating the control frame with a stylesheet (which cannot
// reproduce its second, inner row), route the multi-line frame through the very
// same primitive. Same code, so the result is identical by construction on
// every platform -- each one contributing its own field frame.
//
// Installs an event filter that puts the style on each field as it is polished,
// rather than setting an application style: QtShellWindow puts its own style on
// the shell window and Qt propagates that down the entire child tree, so an
// application-level style never reaches the fields inside it.
void Qt_InstallFieldFrameStyle();
