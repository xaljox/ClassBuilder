// qt/CleanupExternClassesDialog.cpp -- the "Clean Up Extern Classes" action
// (see QtCleanupExternClassesDialog.h).
//
// Reverse-lookup analysis straight off each ExternClass (GetInheritCount()==0 &&
// GetVariableCount()==0 -- nobody inherits it AND no member/argument is typed as
// it), a flat checklist (default-checked, Select All / Unselect All), a
// delete-as-ONE-undo-step (mirrors CbMultiDeleteDialog), and -- because an extern
// can itself use another extern (Inherit / a member typed as one) -- a re-run
// offer when a round frees more. GUI layer over list_unused_extern_classes.
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

#include "QtModelText.h"            // toQ
#include "ClassBuilderInclude.h"    // model classes + iterators
#include "CbViewLock.h"

namespace {

// Every pure ExternClass that nothing uses: no class inherits it (incoming
// inherit == 0) AND no member/argument is typed as it (the Type's owned Variable
// count == 0). Reverse lookup off each extern -- no full-model scan.
std::vector<std::string> CollectUnusedExternClasses(DataModelDoc* pDoc)
{
    std::vector<std::string> names;
    DataModelDoc::BaseClassIterator iBC(pDoc);
    while (++iBC)
    {
        if (iBC->IsExternClass() && !iBC->IsClass()
            && iBC->GetInheritCount()  == 0
            && iBC->GetVariableCount() == 0)
            names.push_back((const char*)iBC->GetName());
    }
    std::sort(names.begin(), names.end());
    return names;
}

// Delete the named ExternClasses as ONE undo step (mirrors CbMultiDeleteDialog).
void DeleteExternClasses(DataModelDoc* pDoc, const std::vector<std::string>& names)
{
    // Resolve names to live objects BEFORE deleting (the model churns).
    std::vector<DataModelDocObject*> targets;
    for (const std::string& n : names)
    {
        BaseClass* pBC = pDoc->FindBaseClass(CbString(n.c_str()));
        if (pBC && pBC->IsExternClass() && !pBC->IsClass())
            targets.push_back(pBC);
    }
    if (targets.empty())
        return;

    CbViewLock lock(pDoc);                          // one coalesced refresh + wait cursor
    for (DataModelDocObject* obj : targets)
    {
        if (pDoc->DeletedInOpenUndoStep(obj))       // cascaded by an earlier delete -> skip
            continue;
        Gti* p = dynamic_cast<Gti*>(obj);
        if (p && p->OnDelete(true))                 // deletable gate, no UI
            obj->Delete();
    }
    pDoc->MarkLastUndo();                           // collapse into one undo step
}

} // namespace

void Qt_ShowCleanupExternClassesDialog(DataModelDoc* pDoc, void* ownerHwnd)
{
    if (!pDoc)
        return;
    Qt_EnsureApplication();

    for (;;)   // loop so a round that frees more externs can offer another pass
    {
        std::vector<std::string> unused = CollectUnusedExternClasses(pDoc);
        if (unused.empty())
        {
            QMessageBox::information(nullptr,
                QStringLiteral("Clean Up Extern Classes"),
                QStringLiteral("There are no unused extern classes."));
            return;
        }

        QDialog dlg;
        dlg.setWindowTitle(QStringLiteral("Clean Up Extern Classes"));
        QVBoxLayout* v = new QVBoxLayout(&dlg);

        QLabel* intro = new QLabel(
            QStringLiteral("These extern classes are not used anywhere \xE2\x80\x94 "
                           "nothing inherits from them and no member or argument is "
                           "typed as them. Checked ones will be deleted:"), &dlg);
        intro->setWordWrap(true);
        v->addWidget(intro);

        QListWidget* list = new QListWidget(&dlg);
        for (const std::string& n : unused)
        {
            QListWidgetItem* it = new QListWidgetItem(toQ(CbString(n.c_str())), list);
            it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
            it->setCheckState(Qt::Checked);
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
                toDelete.push_back(unused[i]);     // list rows are in `unused` order
        if (toDelete.empty())
            return;

        DeleteExternClasses(pDoc, toDelete);

        // Did deleting those free FURTHER externs? Anything unused now that was
        // NOT already unused before is newly freed (an extern used only by one we
        // just deleted). Only then is it worth offering another pass.
        std::vector<std::string> after = CollectUnusedExternClasses(pDoc);
        std::vector<std::string> newlyFreed;
        for (const std::string& n : after)
            if (std::find(unused.begin(), unused.end(), n) == unused.end())
                newlyFreed.push_back(n);
        if (newlyFreed.empty())
            return;                                 // nothing freed -> no nagging

        QString freedList;
        for (const std::string& n : newlyFreed)
            freedList += QStringLiteral("\n    \xE2\x80\xA2 ") + toQ(CbString(n.c_str()));
        QMessageBox::StandardButton ans = QMessageBox::question(nullptr,
            QStringLiteral("Clean Up Extern Classes"),
            QStringLiteral("Deleting those freed %1 more unused extern class(es):%2"
                           "\n\nClean up again?")
                .arg(int(newlyFreed.size())).arg(freedList),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ans != QMessageBox::Yes)
            return;
        // else loop -> re-collect and re-offer
    }
}
