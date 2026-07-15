// qt/SelectReplace.cpp -- the Qt "select & replace" dialog.
//
// Ported from the MFC SelectReplace. Shown when a method is renamed and its
// old name still appears in other method bodies: a checkbox tree of every
// affected method, grouped by class. OK ripples the rename through the
// checked bodies; double-clicking a method opens its modal code editor.

#include "SelectReplace.h"
#include "ui_SelectReplace.h"

#include "QtSelectReplace.h"      // Qt_ShowSelectReplace
#include "QtApp.h"                // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"          // toQ
#include "QtModelIcons.h"         // Qt_ModelIcon
#include "MethodCodeDialog.h"
#include "ConstructorCodeDialog.h"

#include <QEventLoop>
#include <QPushButton>
#include <QTreeWidgetItem>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {
// Child items carry the Method (or Constructor) pointer.
void setMethod(QTreeWidgetItem* item, Method* p)
{
    item->setData(0, Qt::UserRole,
                  QVariant::fromValue(static_cast<void*>(p)));
}
Method* getMethod(QTreeWidgetItem* item)
{
    return static_cast<Method*>(
        item->data(0, Qt::UserRole).value<void*>());
}
}

SelectReplace::SelectReplace(DataModel* pDataModel,
                             const CbString& oldString,
                             const CbString& newString,
                             QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::SelectReplace)
    , _pDataModel(pDataModel)
    , _oldString(oldString)
    , _newString(newString)
{
    _ui->setupUi(this);

    setWindowTitle(toQ(CbString("Replace '") + _oldString +
                       "', with '" + _newString + "'"));

    connect(_ui->buttonSelectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(true); });
    connect(_ui->buttonUnselectAll, &QPushButton::clicked,
            this, [this] { setAllChecked(false); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &SelectReplace::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &SelectReplace::reject);
    connect(_ui->tree, &QTreeWidget::itemDoubleClicked,
            this, &SelectReplace::onItemDoubleClicked);

    fillTree();
}

SelectReplace::~SelectReplace()
{
    delete _ui;
}

void SelectReplace::fillTree()
{
    DataModel::ClassIterator iClass(_pDataModel);
    while (++iClass)
    {
        QTreeWidgetItem* root = nullptr;

        BaseClass::MethodIterator iMethod(iClass.Get());
        while (++iMethod)
        {
            if (!iMethod->HasStringInCode(_oldString))
                continue;

            if (!root)
            {
                root = new QTreeWidgetItem(_ui->tree);
                root->setText(0, toQ(iClass->GetItemText()));
                root->setIcon(0, Qt_ModelIcon(iClass->GetIcon()));
                root->setFlags(root->flags() & ~Qt::ItemIsUserCheckable);
            }

            QTreeWidgetItem* item = new QTreeWidgetItem(root);
            item->setText(0, toQ(iMethod->GetItemText()));
            item->setIcon(0, Qt_ModelIcon(iMethod->GetIcon()));
            item->setCheckState(0, Qt::Unchecked);
            setMethod(item, iMethod.Get());
        }

        if (root)
            root->sortChildren(0, Qt::AscendingOrder);
    }

    _ui->tree->sortItems(0, Qt::AscendingOrder);
    _ui->tree->expandAll();
}

// Select All / Unselect All -- toggle every method (child) row.
void SelectReplace::setAllChecked(bool checked)
{
    const Qt::CheckState state = checked ? Qt::Checked : Qt::Unchecked;
    for (int i = 0; i < _ui->tree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* root = _ui->tree->topLevelItem(i);
        for (int c = 0; c < root->childCount(); ++c)
            root->child(c)->setCheckState(0, state);
    }
}

namespace {
// The code editors are plain QWidgets now (dock content elsewhere); the
// Replace flow needs one MODALLY -- peek at the code while this dialog
// stays up. Run it as an application-modal window with a local event loop
// until it closes (its own closeEvent still drives the save prompt).
void execEditorModal(QWidget* editor)
{
    editor->setWindowFlag(Qt::Window, true);
    editor->setAttribute(Qt::WA_DeleteOnClose, true);
    editor->setWindowModality(Qt::ApplicationModal);
    editor->show();
    QEventLoop loop;
    QObject::connect(editor, &QObject::destroyed, &loop, &QEventLoop::quit);
    loop.exec();
}
}

// Double-click a method row -> open its code editor modally.
void SelectReplace::onItemDoubleClicked(QTreeWidgetItem* item, int /*col*/)
{
    if (!item || !item->parent())          // ignore the class roots
        return;

    Method* pMethod = getMethod(item);
    if (!pMethod)
        return;

    if (Constructor* pConstructor = dynamic_cast<Constructor*>(pMethod))
        execEditorModal(new ConstructorCodeDialog(pConstructor, this));
    else
        execEditorModal(new MethodCodeDialog(pMethod, this));
}

// OK -- ripple the rename through every checked method body.
void SelectReplace::accept()
{
    for (int i = 0; i < _ui->tree->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* root = _ui->tree->topLevelItem(i);
        for (int c = 0; c < root->childCount(); ++c)
        {
            QTreeWidgetItem* item = root->child(c);
            if (item->checkState(0) == Qt::Checked)
            {
                Method* pMethod = getMethod(item);
                if (pMethod)
                    pMethod->ReplaceInCode(_oldString, _newString);
            }
        }
    }

    QDialog::accept();
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
void Qt_ShowSelectReplace(DataModel* pDataModel,
                          const CbString& oldString,
                          const CbString& newString,
                          void* ownerHwnd)
{
    Qt_EnsureApplication();

    SelectReplace dlg(pDataModel, oldString, newString);
    Qt_ExecModal(dlg, ownerHwnd);
}
