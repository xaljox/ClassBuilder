// qt/CleanupExternClassesDialog.cpp -- the "Clean Up Extern Classes" and "Clean
// Up Other Types" actions (see QtCleanupExternClassesDialog.h).
//
// Both are the same checklist (default-checked, Select All / Unselect All; OK
// deletes the checked ones as ONE undo step, mirroring CbMultiDeleteDialog) over
// a reverse-lookup analysis straight off each type -- no full-model scan:
//   * Extern classes: unused == nobody inherits it AND no member/argument is
//     typed as it (GetInheritCount()==0 && GetVariableCount()==0). An extern can
//     itself use another extern, so a round that frees more offers a re-run.
//   * Other types: unused == the model's own delete gate allows it, OnDelete(true)
//     -- which is "no member/argument typed as it, and not a protected built-in
//     (void/int)". Other types reference nothing, so there is no re-run.
// GUI layer over list_unused_extern_classes / list_unused_other_types.
#include "QtCleanupExternClassesDialog.h"
#include "QtApp.h"                  // Qt_EnsureApplication / Qt_ExecModal

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

#include <vector>
#include <string>
#include <algorithm>
#include <functional>

#include "QtModelText.h"            // toQ
#include "QtCompact.h"              // compactItemSize (shared compact row height)
#include "ClassBuilderInclude.h"    // model classes + iterators
#include "CbViewLock.h"

namespace {

// Extern classes nothing uses: no incoming inherit AND no member/argument typed
// as them (reverse lookup off each extern -- the Type's owned Variable count).
std::vector<std::string> CollectUnusedExternClasses(DataModelDoc* pDoc)
{
    std::vector<std::string> names;
    DataModelDoc::BaseClassIterator iBC(pDoc);
    while (++iBC)
    {
        if (iBC->IsExternClass() && !iBC->IsClass() && iBC->GetAdded()
            && iBC->GetInheritCount()  == 0
            && iBC->GetVariableCount() == 0)
            names.push_back((const char*)iBC->GetName());
    }
    std::sort(names.begin(), names.end());
    return names;
}

// Other types the model's own delete gate allows removing: OnDelete(true) is
// side-effect-free and returns false for a referenced type and for the protected
// built-ins (void/int) -- so Cleanup follows the model's rule, no special-casing.
std::vector<std::string> CollectUnusedOtherTypes(DataModelDoc* pDoc)
{
    std::vector<std::string> names;
    DataModelDoc::TypeIterator iType(pDoc);
    while (++iType)
    {
        // GetAdded() skips internal types that aren't in the tree -- notably the
        // special "..." varargs OtherType, which is never Add()'ed.
        if (iType->IsOtherType() && iType->GetAdded() && iType->OnDelete(true))
            names.push_back((const char*)iType->GetName());
    }
    std::sort(names.begin(), names.end());
    return names;
}

// Delete the named types as ONE undo step (mirrors CbMultiDeleteDialog). Resolve
// via FindType so it works for extern classes and other types alike.
void DeleteTypesByName(DataModelDoc* pDoc, const std::vector<std::string>& names)
{
    std::vector<Gti*> targets;                      // Type* -> Gti* (Type : Gti)
    for (const std::string& n : names)
    {
        Type* t = pDoc->FindType(CbString(n.c_str()));
        if (t)
            targets.push_back(t);
    }
    if (targets.empty())
        return;

    CbViewLock lock(pDoc);                          // one coalesced refresh + wait cursor
    for (Gti* p : targets)
    {
        DataModelDocObject* obj = p;                // Gti : DataModelDocObject
        if (pDoc->DeletedInOpenUndoStep(obj))       // cascaded by an earlier delete -> skip
            continue;
        if (p->OnDelete(true))                      // model's deletable gate, no UI
            obj->Delete();
    }
    pDoc->MarkLastUndo();                           // collapse into one undo step
}

// The shared checklist. `collect` yields the current candidates; `offerRerun`
// asks to run again when a round frees further candidates (externs only).
void RunCleanup(DataModelDoc* pDoc, void* ownerHwnd,
                const QString& title, const QString& intro, const QString& noneMsg,
                const std::function<std::vector<std::string>(DataModelDoc*)>& collect,
                bool defaultChecked, bool offerRerun)
{
    if (!pDoc)
        return;
    Qt_EnsureApplication();

    for (;;)
    {
        std::vector<std::string> items = collect(pDoc);
        if (items.empty())
        {
            QMessageBox::information(nullptr, title, noneMsg);
            return;
        }

        QDialog dlg;
        dlg.setWindowTitle(title);
        QVBoxLayout* v = new QVBoxLayout(&dlg);

        QLabel* label = new QLabel(intro, &dlg);
        label->setWordWrap(true);
        v->addWidget(label);

        QListWidget* list = new QListWidget(&dlg);
        for (const std::string& n : items)
        {
            const QString t = toQ(CbString(n.c_str()));
            QListWidgetItem* it = new QListWidgetItem(t, list);
            it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
            it->setCheckState(defaultChecked ? Qt::Checked : Qt::Unchecked);
            it->setSizeHint(compactItemSize(list, t, /*checkable*/ true));  // shared compact row height
        }
        v->addWidget(list);

        QHBoxLayout* h = new QHBoxLayout();
        QPushButton* selAll  = new QPushButton(QStringLiteral("Select All"),  &dlg);
        QPushButton* selNone = new QPushButton(QStringLiteral("Unselect All"), &dlg);
        h->addWidget(selAll);
        h->addWidget(selNone);
        h->addStretch();
        v->addLayout(h);
        auto setAll = [list](Qt::CheckState s) {
            for (int i = 0; i < list->count(); ++i) list->item(i)->setCheckState(s);
        };
        QObject::connect(selAll,  &QPushButton::clicked, &dlg, [setAll]{ setAll(Qt::Checked);   });
        QObject::connect(selNone, &QPushButton::clicked, &dlg, [setAll]{ setAll(Qt::Unchecked); });

        QDialogButtonBox* bb = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
        QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        v->addWidget(bb);

        if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
            return;

        std::vector<std::string> toDelete;
        for (int i = 0; i < list->count(); ++i)
            if (list->item(i)->checkState() == Qt::Checked)
                toDelete.push_back(items[i]);       // rows are in `items` order
        if (toDelete.empty())
            return;

        DeleteTypesByName(pDoc, toDelete);

        if (!offerRerun)
            return;

        // Did deleting those free FURTHER candidates (a type used only by one we
        // just deleted)? Anything a candidate now that was NOT one before.
        std::vector<std::string> after = collect(pDoc);
        std::vector<std::string> newlyFreed;
        for (const std::string& n : after)
            if (std::find(items.begin(), items.end(), n) == items.end())
                newlyFreed.push_back(n);
        if (newlyFreed.empty())
            return;                                 // nothing freed -> no nagging

        QString freedList;
        for (const std::string& n : newlyFreed)
            freedList += QStringLiteral("\n    \xE2\x80\xA2 ") + toQ(CbString(n.c_str()));
        QMessageBox::StandardButton ans = QMessageBox::question(nullptr, title,
            QStringLiteral("Deleting those freed %1 more unused extern class(es):%2"
                           "\n\nClean up again?")
                .arg(int(newlyFreed.size())).arg(freedList),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ans != QMessageBox::Yes)
            return;
        // else loop -> re-collect and re-offer
    }
}

} // namespace

void Qt_ShowCleanupExternClassesDialog(DataModelDoc* pDoc, void* ownerHwnd)
{
    RunCleanup(pDoc, ownerHwnd,
        QStringLiteral("Clean Up Extern Classes"),
        QStringLiteral("These extern classes are not used by any class, member or "
                       "argument. The checked ones will be deleted:"),
        QStringLiteral("There are no unused extern classes."),
        CollectUnusedExternClasses,
        /*defaultChecked*/ true, /*offerRerun*/ true);
}

void Qt_ShowCleanupOtherTypesDialog(DataModelDoc* pDoc, void* ownerHwnd)
{
    RunCleanup(pDoc, ownerHwnd,
        QStringLiteral("Clean Up Other Types"),
        QStringLiteral("These types are not used by any member or argument. "
                       "The checked ones will be deleted:"),
        QStringLiteral("There are no unused other types."),
        CollectUnusedOtherTypes,
        /*defaultChecked*/ false, /*offerRerun*/ false);
}
