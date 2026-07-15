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

bool isIdentChar(QChar c) { return c.isLetterOrNumber() || c == '_'; }

// Whole-identifier occurrence of `name` followed by '(' -- a CALL. A bare
// mention (a relation macro's class-name argument, a declaration, a
// comment) has no '(' behind it and is not a caller (JV).
bool callsName(const QString& code, const QString& name)
{
    const int len = code.length();
    int from = 0;
    while (true)
    {
        const int hit = code.indexOf(name, from);
        if (hit < 0)
            return false;
        from = hit + 1;
        if (hit > 0 && isIdentChar(code[hit - 1]))
            continue;
        int after = hit + name.length();
        if (after < len && isIdentChar(code[after]))
            continue;
        while (after < len && (code[after] == ' ' || code[after] == '\t'))
            ++after;
        if (after < len && code[after] == '(')
            return true;
    }
}

// Every method whose stored body (constructors: init list included) CALLS
// `name`, in model (tree) order. A method that calls itself lists itself --
// that is a true caller.
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
        if (!callsName(code, name))
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
    // Compact rows -- the windows11 style pads list items touch-friendly
    // tall, which reads as empty space here.
    list->setStyleSheet(
        "QListWidget::item { padding: 2px 4px; min-height: 0px; }");

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
