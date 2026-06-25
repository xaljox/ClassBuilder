// qt/QtCodeEditorClose.cpp -- Qt_CloseCodeEditor dispatch (see QtCodeEditor.h).
//
// qobject_cast picks the concrete editor type to detach + close.

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
