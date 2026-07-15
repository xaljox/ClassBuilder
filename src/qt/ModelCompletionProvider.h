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
class Method;

class ModelCompletionProvider : public CodeCompletionProvider
{
public:
    explicit ModelCompletionProvider(Method* pMethod);

    QList<CodeCompletionItem> completions(const QString& textToCursor,
                                          int& prefixLen) override;

private:
    Method* _pMethod;
};
