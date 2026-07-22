// qt/QtWhoCallsMe.cpp -- see QtWhoCallsMe.h.

#include "QtWhoCallsMe.h"
#include "CodeEditor.h"
#include "ModelCompletionProvider.h"  // callsMethod (receiver-verified hits)
#include "QtApp.h"           // Qt_SelectInModelTree
#include "QtModelText.h"     // toQ
#include "QtModelIcons.h"    // Qt_ModelIcon
#include "QtSoftSelection.h" // Qt_ApplySoftSelection

#include <QApplication>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
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

// The direct-pick shortcut label for a popup row: Cmd+1..9 on macOS,
// Ctrl+1..9 elsewhere. Kept in lockstep with the completion popup's
// rowShortcutLabel (CodeEditor.cpp) so the two read consistently -- nine
// slots, empty past the ninth row.
QString rowShortcutLabel(int row)
{
    if (row < 0 || row >= 9)
        return QString();
#ifdef __APPLE__
    return QChar(0x2318) + QString::number(row + 1);        // ⌘1
#else
    return "Ctrl+" + QString::number(row + 1);
#endif
}

// Row painter: the base delegate draws the icon + caller text; we add the
// row's direct-pick shortcut right-aligned in a muted colour, mirroring the
// completion popup's CompletionItemDelegate. Disabled rows -- the
// "(no callers found)" placeholder -- get no shortcut.
class CallerItemDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        QStyledItemDelegate::paint(painter, option, index);

        if (!(option.state & QStyle::State_Enabled))
            return;
        const QString hint = rowShortcutLabel(index.row());
        if (hint.isEmpty())
            return;

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const QWidget* w = opt.widget;
        QStyle* style = w ? w->style() : QApplication::style();
        const QRect textRect =
            style->subElementRect(QStyle::SE_ItemViewItemText, &opt, w);

        // The soft accent tint keeps a selected row light, so the normal
        // Text colour muted with alpha reads in every state (same reasoning
        // as the completion delegate).
        QPalette::ColorGroup cg = (option.state & QStyle::State_Active)
            ? QPalette::Normal : QPalette::Inactive;
        QColor c = opt.palette.color(cg, QPalette::Text);
        c.setAlpha(120);

        painter->save();
        painter->setFont(opt.font);
        painter->setPen(c);
        painter->drawText(textRect.adjusted(0, 0, -6, 0),
                          Qt::AlignRight | Qt::AlignVCenter, hint);
        painter->restore();
    }
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
        // Cmd+1..9 (mac; ControlModifier = the Cmd key there) / Ctrl+1..9:
        // activate the Nth row directly, matching the completion popup (see
        // rowShortcutLabel). Keypad digits carry KeypadModifier on macOS --
        // mask it out. The disabled "(no callers found)" row is skipped.
        const Qt::KeyboardModifiers mods =
            event->modifiers() & ~Qt::KeypadModifier;
        if (mods == Qt::ControlModifier &&
            event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9)
        {
            if (QListWidgetItem* row = item(event->key() - Qt::Key_1))
                if (row->flags() & Qt::ItemIsEnabled)
                    emit itemActivated(row);
            return;
        }
        // macOS: QAbstractItemView only emits itemActivated on double-click
        // (on Windows Return activates too) -- route Return/Enter (keypad
        // sends Key_Enter) to the same path by hand.
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            if (QListWidgetItem* item = currentItem())
                emit itemActivated(item);
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
    // Selection look: the tree's soft accent tint, matching the completion
    // popup (the two must read consistently) -- see QtSoftSelection.h.
    Qt_ApplySoftSelection(list);
    // ...and the same right-aligned Ctrl/Cmd+1..9 direct-pick shortcuts.
    list->setItemDelegate(new CallerItemDelegate(list));

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
    for (int i = 0; i < callers.size(); ++i)
    {
        const Caller& caller = callers.at(i);
        auto* item = new QListWidgetItem(caller.display, list);
        item->setIcon(Qt_ModelIcon(caller.pMethod->GetIcon()));
        item->setData(Qt::UserRole,
                      QVariant::fromValue<void*>(caller.pMethod));
        int w = metrics.horizontalAdvance(caller.display) + 48;
        const QString hint = rowShortcutLabel(i);   // reserve the shortcut column
        if (!hint.isEmpty())
            w += metrics.horizontalAdvance(hint) + 12;
        item->setSizeHint(QSize(w, rowHeight));
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
