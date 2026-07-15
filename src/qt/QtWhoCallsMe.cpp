// qt/QtWhoCallsMe.cpp -- see QtWhoCallsMe.h.

#include "QtWhoCallsMe.h"
#include "CodeEditor.h"
#include "ModelCompletionProvider.h"  // callsMethod (receiver-verified hits)
#include "QtApp.h"           // Qt_SelectInModelTree
#include "QtModelText.h"     // toQ
#include "QtModelIcons.h"    // Qt_ModelIcon

#include <QApplication>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QListWidget>
#include <QMouseEvent>
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

// The popup list itself: QAbstractItemView ignores Esc instead of closing
// the popup window (there is no parent to bubble to), and an outside click
// never reached this widget's own mouse handler either -- so an
// APPLICATION-level filter watches every press and closes the popup when
// the click's global position falls outside it (the filter dies with the
// popup; the click itself is not swallowed). Esc and focus loss also close.
class CallerListPopup : public QListWidget
{
public:
    explicit CallerListPopup(QWidget* parent) : QListWidget(parent)
    {
        qApp->installEventFilter(this);
    }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (!frameGeometry().contains(
                    mouseEvent->globalPosition().toPoint()))
                close();
        }
        return QListWidget::eventFilter(obj, event);
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        if (event->key() == Qt::Key_Escape)
        {
            close();
            return;
        }
        QListWidget::keyPressEvent(event);
    }

    void focusOutEvent(QFocusEvent* event) override
    {
        QListWidget::focusOutEvent(event);
        close();
    }
};

// Every method whose stored body (constructors: init list included) CALLS
// pEdited, in model (tree) order. A textual hit is only the shortlist:
// ModelCompletionProvider::callsMethod then resolves each call's receiver
// in the candidate's own context and keeps the hit only when it really is
// (or can dynamically dispatch to) the edited method. A method that calls
// itself lists itself -- that is a true caller.
QList<Caller> findCallers(Method* pEdited)
{
    QList<Caller> out;
    if (toQ(pEdited->GetName()).isEmpty())
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
        ModelCompletionProvider context(pMethod);  // the CALLER's context
        if (!context.callsMethod(code, pEdited))
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

    auto* list = new CallerListPopup(editor);
    list->setWindowFlags(Qt::Popup);
    list->setAttribute(Qt::WA_DeleteOnClose, true);
    list->setFont(CodeEditor::codeFont());
    list->setIconSize(QSize(16, 16));

    // Compact rows via an explicit per-item size hint -- the windows11
    // style pads list items touch-friendly tall (a stylesheet padding
    // override proved to change nothing there).
    const QFontMetrics metrics(list->font());
    const int rowHeight = metrics.height() + 2;   // as tight as it reads

    if (callers.isEmpty())
    {
        // A silent no-op reads as "the command is broken" -- say it.
        auto* item = new QListWidgetItem("(no callers found)", list);
        item->setFlags(Qt::NoItemFlags);
        item->setSizeHint(QSize(metrics.horizontalAdvance(item->text()) + 48,
                                rowHeight));
    }
    for (const Caller& caller : callers)
    {
        auto* item = new QListWidgetItem(caller.display, list);
        item->setIcon(Qt_ModelIcon(caller.pMethod->GetIcon()));
        item->setData(Qt::UserRole,
                      QVariant::fromValue<void*>(caller.pMethod));
        item->setSizeHint(QSize(metrics.horizontalAdvance(caller.display) + 48,
                                rowHeight));
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
    int width = 0;
    for (int i = 0; i < list->count(); ++i)
        width = qMax(width, list->item(i)->sizeHint().width());
    width += 8;                        // frame
    const int rows = qMin(list->count(), 14);
    list->resize(qMax(width, 240), rows * rowHeight + 8);

    const QRect strip = editor->headerGlobalRect();
    list->move(strip.isNull()
                   ? editor->mapToGlobal(QPoint(4, 4))
                   : strip.bottomLeft() + QPoint(0, 2));
    list->show();
    list->setFocus();
}
