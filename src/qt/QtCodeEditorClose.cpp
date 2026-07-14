// qt/QtCodeEditorClose.cpp -- Qt_CloseCodeEditor / Qt_ReplaceInOpenCodeEditor
// dispatch (see QtCodeEditor.h).
//
// qobject_cast picks the concrete editor type to detach + close / refresh.

#include "QtCodeEditor.h"
#include "MethodCodeDialog.h"
#include "ConstructorCodeDialog.h"

#include <QDialog>

void Qt_CloseCodeEditor(QDialog* pDialog)
{
    if (!pDialog)
        return;
    if (auto* m = qobject_cast<MethodCodeDialog*>(pDialog))
    {
        m->detachForDelete();
        return;
    }
    if (auto* c = qobject_cast<ConstructorCodeDialog*>(pDialog))
    {
        c->detachForDelete();
        return;
    }
}

void Qt_ReplaceInOpenCodeEditor(QDialog* pDialog, const CbString& oldString,
                                const CbString& newString)
{
    if (!pDialog)
        return;
    if (auto* m = qobject_cast<MethodCodeDialog*>(pDialog))
    {
        m->modelReplacedInCode(oldString, newString);
        return;
    }
    if (auto* c = qobject_cast<ConstructorCodeDialog*>(pDialog))
    {
        c->modelReplacedInCode(oldString, newString);
        return;
    }
}

void Qt_UndoRedoOpenCodeEditor(QDialog* pDialog, Method* pOldState)
{
    if (!pDialog || !pOldState)
        return;
    if (auto* m = qobject_cast<MethodCodeDialog*>(pDialog))
    {
        m->modelStateRestored(pOldState);
        return;
    }
    if (auto* c = qobject_cast<ConstructorCodeDialog*>(pDialog))
    {
        c->modelStateRestored(pOldState);
        return;
    }
}
