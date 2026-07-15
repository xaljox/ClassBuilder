// qt/QtCodeEditorClose.cpp -- Qt_CloseCodeEditor / Qt_ReplaceInOpenCodeEditor
// dispatch (see QtCodeEditor.h).
//
// qobject_cast picks the concrete editor type to detach + close / refresh.

#include "QtCodeEditor.h"
#include "MethodCodeDialog.h"
#include "ConstructorCodeDialog.h"

#include <QWidget>

void Qt_CloseCodeEditor(QWidget* pWidget)
{
    if (!pWidget)
        return;
    if (auto* m = qobject_cast<MethodCodeDialog*>(pWidget))
    {
        m->detachForDelete();
        return;
    }
    if (auto* c = qobject_cast<ConstructorCodeDialog*>(pWidget))
    {
        c->detachForDelete();
        return;
    }
}

void Qt_ReplaceInOpenCodeEditor(QWidget* pWidget, const CbString& oldString,
                                const CbString& newString)
{
    if (!pWidget)
        return;
    if (auto* m = qobject_cast<MethodCodeDialog*>(pWidget))
    {
        m->modelReplacedInCode(oldString, newString);
        return;
    }
    if (auto* c = qobject_cast<ConstructorCodeDialog*>(pWidget))
    {
        c->modelReplacedInCode(oldString, newString);
        return;
    }
}

void Qt_UndoRedoOpenCodeEditor(QWidget* pWidget, Method* pOldState)
{
    if (!pWidget || !pOldState)
        return;
    if (auto* m = qobject_cast<MethodCodeDialog*>(pWidget))
    {
        m->modelStateRestored(pOldState);
        return;
    }
    if (auto* c = qobject_cast<ConstructorCodeDialog*>(pWidget))
    {
        c->modelStateRestored(pOldState);
        return;
    }
}
