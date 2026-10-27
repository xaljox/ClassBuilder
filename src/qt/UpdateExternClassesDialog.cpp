// qt/UpdateExternClassesDialog.cpp -- the "Update Extern Classes" action (see
// QtUpdateExternClassesDialog.h).
//
// One checkable row per extern class of this project that has a same-named class
// in the source; its children list what an update changes (+ added, - removed,
// ~ changed, with before -> after). Out-of-date rows start checked, up-to-date
// rows are hidden unless asked for. All rules live in the shared model engine;
// this file only renders the plan and forwards the checked names.
#include "QtUpdateExternClassesDialog.h"
#include "QtApp.h"                  // Qt_EnsureApplication / Qt_ExecModal

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include <string>
#include <vector>

#include "QtModelText.h"            // toQ
#include "QtCompact.h"              // compactCombo
#include "CbTreeWidget.h"
#include "ClassBuilderInclude.h"    // model classes
#include "ClassBuilderDoc.h"
#include "CbShellHooks.h"           // Cb_DocumentCount / Cb_DocumentAt
#include "CbUpdate.h"

namespace {

enum Column { ColName = 0, ColDetail, ColCount };

struct SourceDoc
{
    DataModelDoc* dmd;
    QString       title;
};

QString Plural(int n, const char* one, const char* many)
{
    return QStringLiteral("%1 %2").arg(n).arg(QString::fromLatin1(n == 1 ? one : many));
}

QString OpText(const CbUpdateChange& c)
{
    const QString what = QString::fromStdString(c.what);
    switch (c.op)
    {
    case '+': return QStringLiteral("+ add %1").arg(what);
    case '-': return QStringLiteral("\xE2\x88\x92 remove %1").arg(what);   // U+2212 minus
    default:  return QStringLiteral("~ change %1").arg(what);
    }
}

QString DetailText(const CbUpdateChange& c)
{
    const QString before = QString::fromStdString(c.before);
    const QString after  = QString::fromStdString(c.after);
    switch (c.op)
    {
    case '+': return after;
    case '-': return before;
    default:  return QStringLiteral("%1  \xE2\x86\x92  %2").arg(before, after);  // U+2192 arrow
    }
}

int OutOfDate(const CbUpdatePlan& plan)
{
    int n = 0;
    for (const CbUpdateRow& r : plan.rows)
        if (!r.changes.empty())
            ++n;
    return n;
}

} // namespace

void Qt_ShowUpdateExternClassesDialog(DataModelDoc* pTgt, void* ownerHwnd)
{
    if (!pTgt)
        return;
    Qt_EnsureApplication();
    const QString title = QStringLiteral("Update Extern Classes");

    // Target = this project; sources = every other open project.
    std::vector<SourceDoc> sources;
    QString targetTitle;
    for (int i = 0; i < Cb_DocumentCount(); ++i)
    {
        CClassBuilderDoc* d = Cb_DocumentAt(i);
        if (!d)
            continue;
        if (&d->GetDataModelDoc() == pTgt)
            targetTitle = toQ(d->GetTitle());
        else
            sources.push_back({ &d->GetDataModelDoc(), toQ(d->GetTitle()) });
    }
    if (sources.empty())
    {
        QMessageBox::information(nullptr, title,
            QStringLiteral("Update Extern Classes refreshes this project's extern classes "
                           "from the same-named classes of another open project.\n\nOpen "
                           "that project as well (File > Open), then run Update Extern "
                           "Classes again."));
        return;
    }

    // Start on the first source that has matching externs, preferring one with
    // something to update.
    int startIndex = -1;
    for (int pass = 0; pass < 2 && startIndex < 0; ++pass)
        for (size_t i = 0; i < sources.size(); ++i)
        {
            CbUpdatePlan p;
            Cb_UpdatePlan(sources[i].dmd, pTgt, p);
            if (!p.rows.empty() && (pass == 1 || OutOfDate(p) > 0))
            {
                startIndex = (int)i;
                break;
            }
        }
    if (startIndex < 0)
    {
        QMessageBox::information(nullptr, title,
            QStringLiteral("No extern class in %1 has a same-named class in %2.\n\n"
                           "An extern class is updated from the class with the same name "
                           "in the other project.")
                .arg(targetTitle,
                     sources.size() == 1 ? sources.front().title
                                         : QStringLiteral("any other open project")));
        return;
    }

    QDialog dlg;
    dlg.setWindowTitle(title);
    dlg.resize(860, 540);
    QVBoxLayout* v = new QVBoxLayout(&dlg);

    // --- source / target ---------------------------------------------------
    QHBoxLayout* top = new QHBoxLayout();
    top->addWidget(new QLabel(QStringLiteral("From:"), &dlg));
    QComboBox* sourceCombo = new QComboBox(&dlg);
    for (const SourceDoc& s : sources)
        sourceCombo->addItem(s.title);
    sourceCombo->setCurrentIndex(startIndex);
    sourceCombo->setEnabled(sources.size() > 1);   // a choice only with 3+ projects open
    compactCombo(sourceCombo);
    top->addWidget(sourceCombo, 1);
    top->addSpacing(16);
    top->addWidget(new QLabel(QStringLiteral("Into: <b>%1</b>").arg(targetTitle.toHtmlEscaped()), &dlg));
    top->addStretch(1);
    v->addLayout(top);

    QLabel* intro = new QLabel(QStringLiteral(
        "Each extern class below has a same-named <b>class</b> in the source project. "
        "Updating gives the extern that class's members (with their getters and setters) "
        "and its non-private methods, without bodies (relation and macro methods are not "
        "part of it). "
        "Changes are made in place, so sequence-diagram messages on a changed method "
        "are kept. Types the interface needs that this project lacks are added."),
        &dlg);
    intro->setWordWrap(true);
    v->addWidget(intro);

    // --- the checklist (group box: list + Select All / Unselect All) ----------
    QGroupBox* group = new QGroupBox(QStringLiteral("Extern classes to update"), &dlg);
    QVBoxLayout* gv = new QVBoxLayout(group);
    CbTreeWidget* tree = new CbTreeWidget(group);
    tree->setColumnCount(ColCount);
    tree->setHeaderLabels({ QStringLiteral("Extern class"), QStringLiteral("Changes") });
    tree->setUniformRowHeights(true);
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    QHeaderView* header = tree->header();
    header->setSectionsMovable(false);
    header->setStretchLastSection(true);
    header->resizeSection(ColName, 240);
    gv->addWidget(tree, 1);

    QHBoxLayout* selRow = new QHBoxLayout();
    QPushButton* selAll  = new QPushButton(QStringLiteral("&Select All"), group);
    QPushButton* selNone = new QPushButton(QStringLiteral("&Unselect All"), group);
    selRow->addWidget(selAll);
    selRow->addWidget(selNone);
    selRow->addStretch(1);
    QCheckBox* showUpToDate = new QCheckBox(QStringLiteral("Show up-to-date extern classes"), group);
    selRow->addWidget(showUpToDate);
    gv->addLayout(selRow);
    v->addWidget(group, 1);

    QLabel* summary = new QLabel(&dlg);
    v->addWidget(summary);

    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    bb->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Update"));
    v->addWidget(bb);

    // --- state + rendering ---------------------------------------------------
    CbUpdatePlan plan;
    DataModelDoc* pSrc = sources[(size_t)startIndex].dmd;
    std::vector<QTreeWidgetItem*> items;           // parallel to plan.rows

    auto refreshSummary = [&]() {
        int checked = 0, changes = 0;
        for (size_t i = 0; i < items.size(); ++i)
            if (items[i]->checkState(ColName) == Qt::Checked)
            {
                ++checked;
                changes += (int)plan.rows[i].changes.size();
            }
        const int outOfDate = OutOfDate(plan);
        if (outOfDate == 0)
            summary->setText(QStringLiteral("All %1 up to date with <b>%2</b>.")
                .arg(Plural((int)plan.rows.size(), "extern class is", "extern classes are"),
                     sourceCombo->currentText().toHtmlEscaped()));
        else
            summary->setText(QStringLiteral("%1 of %2 out of date &nbsp;&middot;&nbsp; "
                                            "update <b>%3</b> (%4)")
                .arg(outOfDate).arg(Plural((int)plan.rows.size(), "extern class", "extern classes"))
                .arg(Plural(checked, "extern class", "extern classes"),
                     Plural(changes, "change", "changes")));
        bb->button(QDialogButtonBox::Ok)->setEnabled(checked > 0);
    };

    auto applyVisibility = [&]() {
        for (size_t i = 0; i < items.size(); ++i)
            items[i]->setHidden(plan.rows[i].changes.empty() && plan.rows[i].notes.empty() &&
                                !showUpToDate->isChecked());
    };

    auto loadSource = [&](int index) {
        pSrc = sources[(size_t)index].dmd;
        Cb_UpdatePlan(pSrc, pTgt, plan);
        QSignalBlocker block(tree);                // no itemChanged while building
        tree->clear();
        items.clear();
        QFont bold = tree->font();
        bold.setBold(true);
        QFont italic = tree->font();
        italic.setItalic(true);
        for (const CbUpdateRow& r : plan.rows)
        {
            QTreeWidgetItem* it = new QTreeWidgetItem(tree);
            it->setText(ColName, QString::fromStdString(r.name));
            if (r.changes.empty())
            {
                it->setText(ColDetail, QStringLiteral("up to date"));
                it->setDisabled(true);
            }
            else
            {
                it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
                it->setCheckState(ColName, Qt::Checked);
                it->setFont(ColName, bold);
                it->setText(ColDetail, Plural((int)r.changes.size(), "change", "changes"));
            }
            for (const CbUpdateChange& c : r.changes)
            {
                QTreeWidgetItem* ch = new QTreeWidgetItem(it);
                ch->setText(ColName, OpText(c));
                QString detail = DetailText(c);
                if (c.op == '-' && c.sdUses > 0)
                    detail += QStringLiteral("   (on %1)")
                                  .arg(Plural(c.sdUses, "sequence-diagram message",
                                              "sequence-diagram messages"));
                ch->setText(ColDetail, detail);
                ch->setToolTip(ColDetail, detail);
            }
            for (const std::string& n : r.notes)
            {
                QTreeWidgetItem* ch = new QTreeWidgetItem(it);
                ch->setText(ColName, QStringLiteral("note"));
                ch->setText(ColDetail, QString::fromStdString(n));
                ch->setFont(ColName, italic);
                ch->setFont(ColDetail, italic);
            }
            it->setExpanded(!r.changes.empty());
            items.push_back(it);
        }
        applyVisibility();
        refreshSummary();
    };

    auto setAll = [&](Qt::CheckState s) {
        QSignalBlocker block(tree);
        for (size_t i = 0; i < items.size(); ++i)
            if (!plan.rows[i].changes.empty())
                items[i]->setCheckState(ColName, s);
        refreshSummary();
    };
    QObject::connect(selAll,  &QPushButton::clicked, &dlg, [&]{ setAll(Qt::Checked);   });
    QObject::connect(selNone, &QPushButton::clicked, &dlg, [&]{ setAll(Qt::Unchecked); });
    QObject::connect(tree, &QTreeWidget::itemChanged, &dlg,
                     [&](QTreeWidgetItem*, int col) { if (col == ColName) refreshSummary(); });
    QObject::connect(showUpToDate, &QCheckBox::toggled, &dlg, [&]() { applyVisibility(); });
    QObject::connect(sourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     &dlg, [&](int index) { if (index >= 0) loadSource(index); });
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    loadSource(startIndex);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return;

    std::vector<std::string> names;
    for (size_t i = 0; i < items.size(); ++i)
        if (!plan.rows[i].changes.empty() && items[i]->checkState(ColName) == Qt::Checked)
            names.push_back(plan.rows[i].name);
    if (names.empty())
        return;

    CbUpdateResult res;
    std::string error;
    if (!Cb_UpdateExecute(pSrc, pTgt, names, res, error))
    {
        QMessageBox::warning(nullptr, title, QString::fromStdString(error));
        return;
    }

    QMessageBox box(QMessageBox::Information, title,
        QStringLiteral("Updated %1 in %2:\n\n"
                       "  %3 added, %4 removed, %5 changed\n"
                       "  %6")
            .arg(Plural(res.externs, "extern class", "extern classes"), targetTitle)
            .arg(res.added).arg(res.removed).arg(res.changed)
            .arg(Plural(res.types, "new type", "new types")),
        QMessageBox::Ok);
    if (!res.notes.empty())
    {
        QStringList lines;
        for (const std::string& n : res.notes)
            lines << QString::fromStdString(n);
        box.setDetailedText(lines.join(QLatin1Char('\n')));
    }
    box.exec();
}
