// qt/MergeTypesDialog.cpp -- the "Merge Types" action (see QtMergeTypesDialog.h).
//
// Pick a type to merge away ("from") and the type to merge it into ("to"); OK
// re-points every member/argument/return typed `from` onto `to`, migrates the
// name in code, and deletes `from` -- all as ONE undo step. The model operation
// is Cb_MergeOtherType, shared with the merge_types command.
#include "QtMergeTypesDialog.h"
#include "QtApp.h"                  // Qt_EnsureApplication / Qt_ExecModal

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>

#include <vector>
#include <string>
#include <algorithm>

#include "QtModelText.h"            // toQ
#include "ClassBuilderInclude.h"    // model classes + iterators
#include "CbViewLock.h"
#include "CbTypeMerge.h"            // Cb_MergeOtherType

namespace {

// The OtherTypes in the model (added ones, i.e. in the tree). `includeBuiltins`
// controls whether the protected built-ins void/int (which cannot be merged
// AWAY) are offered -- they are still valid merge TARGETS.
std::vector<std::string> CollectOtherTypes(DataModelDoc* pDoc, bool includeBuiltins)
{
    std::vector<std::string> names;
    DataModelDoc::TypeIterator iType(pDoc);
    while (++iType)
    {
        if (!iType->IsOtherType() || !iType->GetAdded())
            continue;
        std::string n = (const char*)iType->GetName();
        if (n.empty())
            continue;
        if (!includeBuiltins && (n == "void" || n == "int"))
            continue;
        names.push_back(n);
    }
    std::sort(names.begin(), names.end());
    return names;
}

} // namespace

void Qt_ShowMergeTypesDialog(DataModelDoc* pDoc, void* ownerHwnd)
{
    if (!pDoc)
        return;
    Qt_EnsureApplication();

    const QString title = QStringLiteral("Merge Types");

    std::vector<std::string> fromTypes = CollectOtherTypes(pDoc, /*includeBuiltins*/ false);
    std::vector<std::string> toTypes   = CollectOtherTypes(pDoc, /*includeBuiltins*/ true);
    if (fromTypes.empty() || toTypes.size() < 2)
    {
        QMessageBox::information(nullptr, title,
            QStringLiteral("The model needs at least two other types to merge."));
        return;
    }

    QDialog dlg;
    dlg.setWindowTitle(title);
    QVBoxLayout* v = new QVBoxLayout(&dlg);

    QLabel* label = new QLabel(
        QStringLiteral("Merge one type into another. Every member, argument and "
                       "return typed as the first type is retyped to the second, "
                       "the name is migrated in code, and the first type is deleted."),
        &dlg);
    label->setWordWrap(true);
    v->addWidget(label);

    QComboBox* fromCombo = new QComboBox(&dlg);
    for (const std::string& n : fromTypes) fromCombo->addItem(toQ(CbString(n.c_str())));
    QComboBox* toCombo = new QComboBox(&dlg);
    for (const std::string& n : toTypes)   toCombo->addItem(toQ(CbString(n.c_str())));

    QFormLayout* form = new QFormLayout();
    form->addRow(QStringLiteral("Merge away (deleted):"), fromCombo);
    form->addRow(QStringLiteral("Into (kept):"),          toCombo);
    v->addLayout(form);

    // Default the two combos to different entries so OK isn't a no-op.
    if (toCombo->count() > 1 &&
        toCombo->itemText(0) == fromCombo->itemText(0))
        toCombo->setCurrentIndex(1);

    QCheckBox* replaceCode = new QCheckBox(
        QStringLiteral("Also migrate the name in code (method bodies, user blocks, "
                       "initializations)"), &dlg);
    replaceCode->setChecked(true);
    v->addWidget(replaceCode);

    QDialogButtonBox* bb = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    v->addWidget(bb);

    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return;

    const std::string from = fromCombo->currentText().toStdString();
    const std::string to   = toCombo->currentText().toStdString();

    CbMergeResult res;
    std::string error;
    bool ok;
    {
        CbViewLock lock(pDoc);                 // one coalesced refresh + wait cursor
        ok = Cb_MergeOtherType(pDoc, from, to, replaceCode->isChecked(), res, error);
    }

    if (!ok)
    {
        QMessageBox::warning(nullptr, title, toQ(CbString(error.c_str())));
        return;
    }

    const int codeTotal = res.bodies + res.userBlocks + res.inits;
    QMessageBox::information(nullptr, title,
        QStringLiteral("Merged '%1' into '%2'.\n\n"
                       "%3 type usage(s) re-pointed.\n"
                       "%4 name occurrence(s) migrated in code.\n"
                       "'%1' deleted.")
            .arg(toQ(CbString(from.c_str())))
            .arg(toQ(CbString(to.c_str())))
            .arg(res.variablesMoved)
            .arg(codeTotal));
}
