// qt/CbMultiDeleteDialog.cpp -- see header.

#include "CbMultiDeleteDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#include <vector>
#include <algorithm>

#include "QtModelText.h"    // toQ
#include "QtModelIcons.h"   // Qt_ModelIcon
#include "CbTreeWidget.h"   // shared tree look (central icon size, branch drawing)

#define FORWARD_ONLY        // collapse the 3 GUI view classes; model stays complete
#include "ClassBuilderInclude.h"   // model classes + iterators
#include "CbViewLock.h"
#include "CbMessageBox.h"

namespace {

constexpr int kGtiRole = Qt::UserRole;

void setItemGti(QTreeWidgetItem* item, Gti* pGti)
{
    item->setData(0, kGtiRole,
                  QVariant::fromValue<qulonglong>(reinterpret_cast<qulonglong>(pGti)));
}

Gti* itemGti(const QTreeWidgetItem* item)
{
    return reinterpret_cast<Gti*>(static_cast<quintptr>(item->data(0, kGtiRole).toULongLong()));
}

// Filter categories -- index == its checkbox in _filters. Cat_None = an
// uncategorised structural container (e.g. the DataModel): never a filter
// target, shown only as an ancestor of visible items.
enum Category {
    Cat_ClassGroup, Cat_Member, Cat_Method, Cat_CtorDtor, Cat_IsClass,
    Cat_Argument, Cat_Relation, Cat_Diagram, Cat_TypeActor,
    Cat_COUNT
};
const int Cat_None = Cat_COUNT;

const char* const kCatLabel[Cat_COUNT] = {
    "Classes && Groups", "Members", "Methods", "Constructors && Destructors",
    "IsClass methods", "Arguments", "Relations && Inheritance",
    "Diagrams", "Types && Actors"
};

// Most-derived first: IsClassMethod / Constructor / Destructor are Methods, and
// classes are ExternClasses are Types -- so never test the broad bases. This is
// the one place dynamic_cast (not Gti::IsX) is used: a single many-way
// categoriser that mustn't depend on which IsX predicates happen to exist.
int categoryOf(Gti* p)
{
    if (dynamic_cast<IsClassMethod*>(p))                                     return Cat_IsClass;
    if (dynamic_cast<Constructor*>(p) || dynamic_cast<Destructor*>(p))       return Cat_CtorDtor;
    if (dynamic_cast<Method*>(p))                                            return Cat_Method;
    if (dynamic_cast<Member*>(p))                                            return Cat_Member;
    if (dynamic_cast<Argument*>(p))                                          return Cat_Argument;
    if (dynamic_cast<ClassGroup*>(p) || dynamic_cast<MetaGroup*>(p) ||
        dynamic_cast<ExternClass*>(p))                                       return Cat_ClassGroup;
    if (dynamic_cast<Relation*>(p) || dynamic_cast<ToRelation*>(p) ||
        dynamic_cast<FromRelation*>(p) || dynamic_cast<Inherit*>(p))         return Cat_Relation;
    if (dynamic_cast<ClassDiagram*>(p) || dynamic_cast<SequenceDiagram*>(p)) return Cat_Diagram;
    if (dynamic_cast<Actor*>(p) || dynamic_cast<OtherType*>(p))              return Cat_TypeActor;
    return Cat_None;
}

bool deletable(Gti* p)
{
    return p->OnDelete(true) != 0;   // the side-effect-free enable gate (no UI)
}

std::vector<Gti*> sortedChildren(Gti* pGti)
{
    std::vector<Gti*> children;
    Gti::ChildIterator iChild(pGti);
    while (++iChild)
        children.push_back(iChild.Get());
    std::stable_sort(children.begin(), children.end(),
                     [](Gti* a, Gti* b) { return Gti::CompareTreeOrder(a, b) < 0; });
    return children;
}

// The undo-stack "already deleted in this open step" guard now lives in the
// model -- DataModelDoc::DeletedInOpenUndoStep -- since it walks model internals
// (the undo stack, UndoDelete/UndoSubDelete). Called from accept() below.

void setAllCheckable(QTreeWidgetItem* item, bool checked)
{
    if (item->flags() & Qt::ItemIsUserCheckable)
        item->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
    for (int i = 0; i < item->childCount(); ++i)
        setAllCheckable(item->child(i), checked);
}

// Expand only the paths that lead to a checkable (deletable) item; branches with
// nothing deletable below them -- a relation's generated sub-methods, the
// ClassBuilder-methods group -- stay collapsed (so they don't clutter, and don't
// re-open on a filter change). Returns whether item-or-descendant is checkable.
bool markExpansion(QTreeWidgetItem* item)
{
    bool below = false;
    for (int i = 0; i < item->childCount(); ++i)
        if (markExpansion(item->child(i)))
            below = true;
    item->setExpanded(below);
    return below || item->flags().testFlag(Qt::ItemIsUserCheckable);
}

} // namespace


CbMultiDeleteDialog::CbMultiDeleteDialog(Gti* pRoot, QWidget* parent)
    : QDialog(parent)
    , _pRoot(pRoot)
    , _pDoc(pRoot->GetDataModelDoc())
{
    setWindowTitle("Delete Multiple");
    resize(480, 580);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(new QLabel(
        QString("Tick the items to delete under \"%1\". Checking a parent includes "
                "its contents. The whole deletion is a single undo step.")
            .arg(toQ(pRoot->GetItemText())), this));

    // Type filters -- narrow what's visible. All on by default.
    QGroupBox* filterBox = new QGroupBox("Show", this);
    QGridLayout* fg = new QGridLayout(filterBox);
    _filters.resize(Cat_COUNT);
    for (int c = 0; c < Cat_COUNT; ++c)
    {
        QCheckBox* cb = new QCheckBox(kCatLabel[c], filterBox);
        cb->setChecked(true);
        connect(cb, &QCheckBox::toggled, this, [this] { rebuildTree(); });
        fg->addWidget(cb, c / 3, c % 3);
        _filters[c] = cb;
    }
    main->addWidget(filterBox);

    _tree = new CbTreeWidget(this);
    _tree->setHeaderHidden(true);
    _tree->setColumnCount(1);
    connect(_tree, &QTreeWidget::itemChanged, this, &CbMultiDeleteDialog::onItemChanged);
    main->addWidget(_tree, 1);

    QHBoxLayout* row = new QHBoxLayout();
    QPushButton* selAll  = new QPushButton("Select All", this);
    QPushButton* selNone = new QPushButton("Unselect All", this);
    connect(selAll,  &QPushButton::clicked, this, &CbMultiDeleteDialog::selectAllVisible);
    connect(selNone, &QPushButton::clicked, this, &CbMultiDeleteDialog::unselectAllVisible);
    row->addWidget(selAll);
    row->addWidget(selNone);
    row->addStretch(1);
    _countLabel = new QLabel(this);
    row->addWidget(_countLabel);
    main->addLayout(row);

    QDialogButtonBox* bb = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    bb->button(QDialogButtonBox::Ok)->setText("Delete");
    connect(bb, &QDialogButtonBox::accepted, this, &CbMultiDeleteDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &CbMultiDeleteDialog::reject);
    main->addWidget(bb);

    rebuildTree();
}

void CbMultiDeleteDialog::rebuildTree()
{
    _propagating = true;   // suppress itemChanged while (re)building
    _tree->clear();
    if (QTreeWidgetItem* root = buildNode(_pRoot, /*isRoot=*/true))
        _tree->addTopLevelItem(root);
    for (int i = 0; i < _tree->topLevelItemCount(); ++i)
        markExpansion(_tree->topLevelItem(i));
    _propagating = false;
    updateCount();
}

// Builds a DETACHED item (children attached) for pGti, honouring the filters.
// Returns nullptr if the node is neither a filter target nor an ancestor of one
// (pruned). The root is always kept (it is the scope).
QTreeWidgetItem* CbMultiDeleteDialog::buildNode(Gti* pGti, bool isRoot)
{
    QVector<QTreeWidgetItem*> kids;
    for (Gti* child : sortedChildren(pGti))
        if (QTreeWidgetItem* k = buildNode(child, false))
            kids.push_back(k);

    const int cat = categoryOf(pGti);
    const bool match = (cat != Cat_None) && _filters[cat]->isChecked();
    if (!isRoot && !match && kids.isEmpty())
        return nullptr;

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, toQ(pGti->GetItemText()));
    item->setIcon(0, Qt_ModelIcon(pGti->GetIcon()));
    setItemGti(item, pGti);
    for (QTreeWidgetItem* k : kids)
        item->addChild(k);

    if (match && deletable(pGti))
    {
        item->setFlags((item->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsAutoTristate);
        item->setCheckState(0, Qt::Unchecked);
    }
    else
    {
        // Structurally-needed but not a deletable target -> shown without a box.
        item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
    }
    return item;
}

void CbMultiDeleteDialog::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (_propagating || column != 0)
        return;
    if (!(item->flags() & Qt::ItemIsUserCheckable))
        return;

    // Checking a parent implies its contents.
    _propagating = true;
    setSubtreeChecked(item, item->checkState(0) == Qt::Checked);
    _propagating = false;
    updateCount();
}

void CbMultiDeleteDialog::setSubtreeChecked(QTreeWidgetItem* item, bool checked)
{
    for (int i = 0; i < item->childCount(); ++i)
    {
        QTreeWidgetItem* c = item->child(i);
        if (c->flags() & Qt::ItemIsUserCheckable)
            c->setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
        setSubtreeChecked(c, checked);
    }
}

void CbMultiDeleteDialog::selectAllVisible()
{
    _propagating = true;
    for (int i = 0; i < _tree->topLevelItemCount(); ++i)
        setAllCheckable(_tree->topLevelItem(i), true);
    _propagating = false;
    updateCount();
}

void CbMultiDeleteDialog::unselectAllVisible()
{
    _propagating = true;
    for (int i = 0; i < _tree->topLevelItemCount(); ++i)
        setAllCheckable(_tree->topLevelItem(i), false);
    _propagating = false;
    updateCount();
}

void CbMultiDeleteDialog::collectChecked(QTreeWidgetItem* item,
                                         QVector<QTreeWidgetItem*>& out) const
{
    if ((item->flags() & Qt::ItemIsUserCheckable) && item->checkState(0) == Qt::Checked)
        out.push_back(item);
    for (int i = 0; i < item->childCount(); ++i)
        collectChecked(item->child(i), out);
}

int CbMultiDeleteDialog::countChecked() const
{
    QVector<QTreeWidgetItem*> v;
    for (int i = 0; i < _tree->topLevelItemCount(); ++i)
        collectChecked(_tree->topLevelItem(i), v);
    return v.size();
}

void CbMultiDeleteDialog::updateCount()
{
    _countLabel->setText(QString("%1 selected").arg(countChecked()));
}

void CbMultiDeleteDialog::accept()
{
    QVector<QTreeWidgetItem*> checkedItems;
    for (int i = 0; i < _tree->topLevelItemCount(); ++i)
        collectChecked(_tree->topLevelItem(i), checkedItems);
    if (checkedItems.isEmpty())
    {
        QDialog::reject();
        return;
    }

    // Snapshot the model objects BEFORE any delete -- the live model + these
    // tree items both churn through the cascade.
    QVector<Gti*> targets;
    targets.reserve(checkedItems.size());
    for (QTreeWidgetItem* it : checkedItems)
        targets.push_back(itemGti(it));

    CbString msg;
    msg.Format("Delete %d selected item%s (and any contents)?\n\n"
               "This is a single undo step.",
               int(targets.size()), targets.size() == 1 ? "" : "s");
    if (CbMessageBox(msg, CBMB_ICONQUESTION | CBMB_YESNO) != CBMB_IDYES)
        return;   // leave the dialog open

    {
        // One coalesced refresh + wait cursor for the whole batch (inner per-
        // delete locks nest harmlessly). MarkLastUndo INSIDE the lock so the
        // diagram recompute it does is flushed by the lock's dtor.
        CbViewLock lock(_pDoc);
        for (Gti* p : targets)
        {
            if (!p)
                continue;
            DataModelDocObject* obj = p;            // Gti : public DataModelDocObject
            if (_pDoc->DeletedInOpenUndoStep(obj))  // cascaded by an earlier delete -> skip
                continue;
            if (p->OnDelete(true))                  // deletable gate (no UI)
                obj->Delete();                      // cascade delete, no per-item prompt
        }
        _pDoc->MarkLastUndo();                      // collapse the batch into one undo step
    }

    QDialog::accept();
}
