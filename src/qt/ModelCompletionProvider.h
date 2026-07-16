// qt/ModelCompletionProvider.h -- model-aware code completion.
//
// The CodeCompletionProvider implementation that taps the live data model --
// the knowledge no generic editor component has:
//
//   var. / var->   the variable's type is resolved (argument type, `TypeName
//                  var` declaration scanned from the body, member of the
//                  owning class, or `this`) and, when it is a model class,
//                  that class's methods are offered -- inherited ones and the
//                  relation macro methods included. An iterator variable
//                  (RowIterator row) dereferences to its relation's target
//                  class, so row-> offers Row's methods.
//   Class::        static methods + the class's relation iterator types.
//   plain typing   arguments, declared variables, the owning class's methods
//                  and members, model types and iterator types.
//
// Methods insert as "Name()" with the caret placed inside the parens when
// the method takes arguments.
//
// One provider serves both editors of a constructor dialog (init + body);
// the context comes entirely from the text handed in per call.
#pragma once

#include "CodeEditor.h"

class BaseClass;
class Gti;
class Method;

class ModelCompletionProvider : public CodeCompletionProvider
{
public:
    explicit ModelCompletionProvider(Method* pMethod);

    QList<CodeCompletionItem> completions(const QString& textToCursor,
                                          int& prefixLen) override;

    // Hover documentation: signature line (monospace) plus the model's
    // @NOTE text for the method / member / argument / class named at `pos`,
    // resolved like definitionAtCursor. Empty when nothing resolves.
    QString hoverText(const QString& text, int pos) override;

    // Parameter hint: the signature of the call being typed (innermost open
    // '(' before the caret), active argument bold, default values shown.
    // The overload is picked by the argument index reached so far. Empty
    // when the caret is not inside a resolvable call.
    QString parameterHint(const QString& textToCursor) override;

    // Who-calls-me support: does `code` -- a body of THIS provider's method
    // -- contain a call of `pTarget`? A textual hit (whole identifier
    // followed by '(') is verified by resolving the call's receiver in this
    // method's context: a receiver whose class does not carry pTarget (nor
    // a base declaration it overrides -- dynamic dispatch) is rejected; an
    // unresolvable receiver keeps the hit.
    bool callsMethod(const QString& code, Method* pTarget);

    // The model object named by the identifier at `pos` in `text` (the FULL
    // editor text) -- a Method, with the receiver resolved from what
    // precedes it (`var->Name` / `expr().Name` / `Class::Name`, else the
    // owning class); or, for a class name (`new Row(...)`, a declaration),
    // that class's constructor when it has one, else the class itself.
    // Null when nothing resolves. Drives F12 go-to-definition.
    Gti* definitionAtCursor(const QString& text, int pos);

    // Constructor init-list mode (the init pane): at a naming position --
    // top level, no member access -- completion offers exactly the members
    // NOT yet initialized in the init text, inserting `_x()` with the caret
    // inside the parens. Inside an initializer value it falls back to normal
    // completion. Off by default; the body pane keeps the normal provider.
    void setInitListMode(bool on) { _initListMode = on; }

private:
    QList<CodeCompletionItem> initListCompletions(const QString& text);

    Method* _pMethod;
    bool    _initListMode = false;
};
