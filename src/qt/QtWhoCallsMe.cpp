// qt/QtWhoCallsMe.cpp -- see QtWhoCallsMe.h.

#include "QtWhoCallsMe.h"
#include "CodeEditor.h"
#include "QtApp.h"           // Qt_SelectInModelTree
#include "QtModelText.h"     // toQ
#include "QtModelIcons.h"    // Qt_ModelIcon

#include <QListWidget>
#include <QVariant>

#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {

// One caller row: display text + the method behind it.
struct Caller
{
    QString display;
    Method* pMethod = nullptr;
};

// Every method whose stored body (constructors: init list included) contains
// a whole-identifier occurrence of `name`, in model (tree) order. A method
// that calls itself lists itself -- that is a true caller.
QList<Caller> findCallers(Method* pEdited)
{
    QList<Caller> out;
    const QString name = toQ(pEdited->GetName());
    if (name.isEmpty())
        return out;

    DataModelDoc::GtiIterator iGti(pEdited->GetDataModelDoc());
    while (++iGti)
    {
        if (!iGti->IsMethod())
            continue;
        Method* pMethod = static_cast<Method*>(iGti.Get());
        QString code = toQ(pMethod->GetCode());
        if (Constructor* pConstructor = dynamic_cast<Constructor*>(pMethod))
            code += '\n' + toQ(pConstructor->GetInit());
        if (CodeEditor::identifierCount(code, name) == 0)
            continue;

        Caller caller;
        caller.pMethod = pMethod;
        caller.display = toQ(pMethod->GetBaseClass()->GetName()) + "::" +
                         toQ(pMethod->GetName()) + "()";
        out.append(caller);
    }
    return out;
}

} // namespace

void Qt_ShowWhoCallsMe(CodeEditor* editor, Method* pMethod)
{
    if (!editor || !pMethod)
        return;

    const QList<Caller> callers = findCallers(pMethod);

    auto* list = new QListWidget(editor);
    list->setWindowFlags(Qt::Popup);
    list->setAttribute(Qt::WA_DeleteOnClose, true);
    list->setFont(CodeEditor::codeFont());

    if (callers.isEmpty())
    {
        // A silent no-op reads as "the command is broken" -- say it.
        auto* item = new QListWidgetItem("(no callers found)", list);
        item->setFlags(Qt::NoItemFlags);
    }
    for (const Caller& caller : callers)
    {
        auto* item = new QListWidgetItem(caller.display, list);
        item->setIcon(Qt_ModelIcon(caller.pMethod->GetIcon()));
        item->setData(Qt::UserRole,
                      QVariant::fromValue<void*>(caller.pMethod));
    }
    if (!callers.isEmpty())
        list->setCurrentRow(0);

    // Enter / double-click: select the caller in the tree + open its editor
    // (OnOpen routes method vs constructor and refocuses an open one).
    QObject::connect(list, &QListWidget::itemActivated, list,
        [list](QListWidgetItem* item)
        {
            auto* pCaller =
                static_cast<Method*>(item->data(Qt::UserRole).value<void*>());
            list->close();
            if (!pCaller)
                return;
            Qt_SelectInModelTree(pCaller->GetDataModelDoc(), pCaller);
            if (pCaller->IsNonMacroMethod())
                pCaller->OnOpen();
        });

    // Size to content (capped), anchored under the signature strip -- the
    // mouse is already up there on the Ctrl+Click path, and the list drops
    // open over the code without covering the strip itself.
    const QFontMetrics metrics(list->font());
    int width = 0;
    for (int i = 0; i < list->count(); ++i)
        width = qMax(width,
                     metrics.horizontalAdvance(list->item(i)->text()));
    width += 56;                       // icon + frame + breathing room
    const int rowHeight = qMax(list->sizeHintForRow(0), 1);
    const int rows = qMin(list->count(), 14);
    list->resize(qMax(width, 240), rows * rowHeight + 8);

    const QRect strip = editor->headerGlobalRect();
    list->move(strip.isNull()
                   ? editor->mapToGlobal(QPoint(4, 4))
                   : strip.bottomLeft() + QPoint(0, 2));
    list->show();
    list->setFocus();
}
