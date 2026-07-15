// qt/QtWhoCallsMe.h -- the "who calls me" popup for the code editors.
//
// For the method being edited, every method body in the model is searched
// for a whole-identifier occurrence of its name (the model IS the index --
// no parsing), and the callers are listed as Class::method() in a popup
// anchored under the editor's signature strip. Enter / double-click selects
// that caller in the tree and opens its editor; Esc or clicking elsewhere
// closes. Name-based, so a same-named method of another class hits too --
// cheap and transparent.
//
// Triggers (wired in the code dialogs): Ctrl/Cmd+Click on the signature
// strip (go-to-definition ON the definition = show the references),
// Shift+F12, and the Edit menu.
#pragma once

class CodeEditor;
class Method;

void Qt_ShowWhoCallsMe(CodeEditor* editor, Method* pMethod);
