// qt/ImportDialog.cpp -- the "Import / Clone" action (see QtImportDialog.h).
//
// One row per source entity the target lacks; three form columns -- Class /
// Extern class / Other type -- of which at most one is ticked: the form the row
// is imported as. Ticking Class on a row makes CbImport pre-tick what that class
// needs on the OTHER rows:
//   * its member / argument / return types  -> Other type, locked (a member or
//     argument is never dropped; you may still pick a richer form);
//   * its bases                             -> Extern class, untickable (untick
//     = the inheritance is dropped);
//   * a richer form worth having            -> a grey (partially checked)
//     suggestion, e.g. Class on a relation partner (a relation only comes along
//     when both ends are imported as Class).
// Pre-ticked (required) rows are shown in italics with the reason in "Needed
// by". All rules live in the shared model engine; this file only renders the
// plan and forwards clicks.
#include "QtImportDialog.h"
#include "QtApp.h"                  // Qt_EnsureApplication / Qt_ExecModal

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include <string>
#include <vector>

#include "QtModelText.h"            // toQ
#include "QtCompact.h"              // compactItemSize, compactCombo
#include "CbTreeWidget.h"
#include "ClassBuilderInclude.h"    // model classes
#include "ClassBuilderDoc.h"
#include "CbShellHooks.h"           // Cb_DocumentCount / Cb_DocumentAt
#include "CbImport.h"

namespace {

enum Column { ColName = 0, ColKind, ColClass, ColExtern, ColOther, ColNeededBy, ColCount };

// A form cell that is a SUGGESTION: drawn as a grey (disabled-look) tick, so it
// reads as "worth ticking" without being mistaken for a real tick (Qt's
// partially-checked box is drawn filled in the accent). Paint-only -- clicks go
// through the dialog's itemClicked handler like any other cell.
const int SuggestedRole = Qt::UserRole + 1;

class SuggestionDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        QStyledItemDelegate::paint(painter, option, index);   // cell as usual (empty box)
        if (!index.data(SuggestedRole).toBool())
            return;
        // Only the indicator again, as a disabled tick. Disabling the WHOLE cell
        // would make the stylesheet paint a hovered/current cell accent-filled.
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const QWidget* w = option.widget;
        QStyle* style = w ? w->style() : QApplication::style();
        opt.rect  = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, w);
        opt.state &= ~(QStyle::State_Enabled | QStyle::State_MouseOver |
                       QStyle::State_Selected | QStyle::State_HasFocus | QStyle::State_Off);
        opt.state |= QStyle::State_On;
        opt.checkState = Qt::Checked;
        style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &opt, painter, w);
    }
};

int FormOfColumn(int col)
{
    switch (col)
    {
    case ColClass:  return CB_IMPORT_CLASS;
    case ColExtern: return CB_IMPORT_EXTERNCLASS;
    case ColOther:  return CB_IMPORT_OTHERTYPE;
    default:        return CB_IMPORT_NONE;
    }
}

QString FormText(int form)
{
    switch (form)
    {
    case CB_IMPORT_CLASS:       return QStringLiteral("Class");
    case CB_IMPORT_EXTERNCLASS: return QStringLiteral("Extern class");
    case CB_IMPORT_OTHERTYPE:   return QStringLiteral("Other type");
    default:                    return QString();
    }
}

QString Plural(int n, const char* one, const char* many)
{
    return QStringLiteral("%1 %2").arg(n).arg(QString::fromLatin1(n == 1 ? one : many));
}

struct SourceDoc
{
    DataModelDoc* dmd;
    QString       title;
};

// One click on a form cell of row r -- the whole interaction model.
void ApplyClick(CbImportRow& r, int form)
{
    if (form == CB_IMPORT_NONE || form > r.sourceKind)
        return;                                 // no such form for this row
    if (form == r.effectiveForm)
    {
        if (r.explicitForm == form)
            r.explicitForm = CB_IMPORT_NONE;    // untick your own pick
        else if (form == CB_IMPORT_EXTERNCLASS && !r.optOut)
            r.optOut = true;                    // decline an (optional) base
        // else: a mandatory Other type -- locked
        return;
    }
    r.explicitForm = form;                      // tick a different form
    r.optOut = false;
}

} // namespace

void Qt_ShowImportDialog(DataModelDoc* pTgt, void* ownerHwnd)
{
    if (!pTgt)
        return;
    Qt_EnsureApplication();
    const QString title = QStringLiteral("Import / Clone");

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
            QStringLiteral("Import / Clone copies classes and types from another open "
                           "project into this one.\n\nOpen the project to import from "
                           "as well (File > Open), then run Import / Clone again."));
        return;
    }

    QDialog dlg;
    dlg.setWindowTitle(title);
    dlg.resize(900, 580);
    QVBoxLayout* v = new QVBoxLayout(&dlg);

    // --- source / target ---------------------------------------------------
    QHBoxLayout* top = new QHBoxLayout();
    top->addWidget(new QLabel(QStringLiteral("From:"), &dlg));
    QComboBox* sourceCombo = new QComboBox(&dlg);
    for (const SourceDoc& s : sources)
        sourceCombo->addItem(s.title);
    sourceCombo->setEnabled(sources.size() > 1);   // a choice only with 3+ projects open
    compactCombo(sourceCombo);
    top->addWidget(sourceCombo, 1);
    top->addSpacing(16);
    top->addWidget(new QLabel(QStringLiteral("Into: <b>%1</b>").arg(targetTitle.toHtmlEscaped()), &dlg));
    top->addStretch(1);
    v->addLayout(top);

    QLabel* intro = new QLabel(QStringLiteral(
        "Tick the form each item is imported as. A <b>Class</b> brings its members, "
        "methods, constructors and bases, and pre-ticks what it needs (<i>italic</i>): "
        "member and argument types at least as <b>Other type</b> (cannot be dropped), "
        "bases as <b>Extern class</b> (untick to drop the inheritance). A grey tick is a "
        "suggestion: a relation only comes along when both ends are imported as Class. "
        "Serialize is never imported; add it afterwards with Project &gt; Add Serialize."),
        &dlg);
    intro->setWordWrap(true);
    v->addWidget(intro);

    // --- filter --------------------------------------------------------------
    QHBoxLayout* filterRow = new QHBoxLayout();
    filterRow->addWidget(new QLabel(QStringLiteral("Filter:"), &dlg));
    QLineEdit* filter = new QLineEdit(&dlg);
    filter->setClearButtonEnabled(true);
    filter->setPlaceholderText(QStringLiteral("name contains..."));
    filterRow->addWidget(filter, 1);
    QCheckBox* involvedOnly = new QCheckBox(QStringLiteral("Show only ticked and suggested"), &dlg);
    filterRow->addWidget(involvedOnly);
    v->addLayout(filterRow);

    // --- the table -----------------------------------------------------------
    CbTreeWidget* tree = new CbTreeWidget(&dlg);
    tree->setColumnCount(ColCount);
    tree->setHeaderLabels({ QStringLiteral("Name"), QStringLiteral("Kind"),
                            QStringLiteral("Class"), QStringLiteral("Extern class"),
                            QStringLiteral("Other type"), QStringLiteral("Needed by") });
    tree->setRootIsDecorated(false);
    tree->setIndentation(0);            // flat list: CbTreeWidget's connector glyphs
                                        // are placed by indentation(), not by depth
    tree->setItemDelegate(new SuggestionDelegate(tree));
    tree->setUniformRowHeights(true);
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    tree->setAlternatingRowColors(true);
    QHeaderView* header = tree->header();
    header->setSectionsMovable(false);
    header->setStretchLastSection(true);
    header->resizeSection(ColName,   220);
    header->resizeSection(ColKind,    90);
    header->resizeSection(ColClass,   70);
    header->resizeSection(ColExtern,  90);
    header->resizeSection(ColOther,   80);
    v->addWidget(tree, 1);

    QLabel* summary = new QLabel(&dlg);
    v->addWidget(summary);
    QPlainTextEdit* notes = new QPlainTextEdit(&dlg);
    notes->setReadOnly(true);
    notes->setMaximumHeight(notes->fontMetrics().lineSpacing() * 4 + 10);
    v->addWidget(notes);

    QHBoxLayout* bottom = new QHBoxLayout();
    QPushButton* clearBtn = new QPushButton(QStringLiteral("Clear"), &dlg);
    bottom->addWidget(clearBtn);
    bottom->addStretch(1);
    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    bb->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Import"));
    bottom->addWidget(bb);
    v->addLayout(bottom);

    // --- state + rendering ---------------------------------------------------
    CbImportPlan plan;
    DataModelDoc* pSrc = sources.front().dmd;
    std::vector<QTreeWidgetItem*> items;          // parallel to plan.rows

    auto applyFilter = [&]() {
        const QString text = filter->text().trimmed();
        for (size_t i = 0; i < items.size(); ++i)
        {
            const CbImportRow& r = plan.rows[i];
            bool show = text.isEmpty() ||
                        QString::fromStdString(r.name).contains(text, Qt::CaseInsensitive);
            if (involvedOnly->isChecked() &&
                r.effectiveForm == CB_IMPORT_NONE && r.suggestedForm == CB_IMPORT_NONE)
                show = false;
            items[i]->setHidden(!show);
        }
    };

    auto render = [&]() {
        for (size_t i = 0; i < items.size(); ++i)
        {
            const CbImportRow& r = plan.rows[i];
            QTreeWidgetItem* it = items[i];
            const bool required = r.explicitForm == CB_IMPORT_NONE &&
                                  r.effectiveForm != CB_IMPORT_NONE;
            QFont f = tree->font();
            f.setItalic(required);
            f.setBold(r.effectiveForm != CB_IMPORT_NONE);
            it->setFont(ColName, f);

            for (int col = ColClass; col <= ColOther; ++col)
            {
                const int form = FormOfColumn(col);
                it->setData(col, SuggestedRole, false);
                if (form > r.sourceKind)
                {
                    it->setData(col, Qt::CheckStateRole, QVariant());   // no such form
                    it->setToolTip(col, QString());
                    continue;
                }
                Qt::CheckState state = Qt::Unchecked;
                QString tip;
                if (form == r.effectiveForm)
                {
                    state = Qt::Checked;
                    if (required && r.mandatory && form == CB_IMPORT_OTHERTYPE)
                        tip = QStringLiteral("Required (%1). A member or argument type cannot "
                                             "be dropped; tick a richer form to upgrade it.")
                                  .arg(QString::fromStdString(r.requiredBy));
                    else if (required)
                        tip = QStringLiteral("Required (%1). Untick to drop it.")
                                  .arg(QString::fromStdString(r.requiredBy));
                    else
                        tip = QStringLiteral("Imported as %1. Click to untick.").arg(FormText(form));
                }
                else if (form == r.suggestedForm)
                {
                    it->setData(col, SuggestedRole, true);   // grey tick (delegate)
                    tip = QStringLiteral("Suggested (%1). Click to import as %2.")
                              .arg(QString::fromStdString(r.requiredBy), FormText(form));
                }
                else
                    tip = QStringLiteral("Click to import as %1.").arg(FormText(form));
                it->setCheckState(col, state);
                it->setToolTip(col, tip);
            }
            QString needed = QString::fromStdString(r.requiredBy);
            it->setText(ColNeededBy, needed);
            it->setToolTip(ColNeededBy, needed);
            QFont fn = tree->font();
            fn.setItalic(required);
            it->setFont(ColNeededBy, fn);
        }

        int total = plan.classes + plan.externClasses + plan.otherTypes;
        if (total == 0)
            summary->setText(QStringLiteral("%1 in <b>%2</b> that <b>%3</b> does not have. Nothing ticked yet.")
                .arg(Plural((int)plan.rows.size(), "item", "items"),
                     sourceCombo->currentText().toHtmlEscaped(), targetTitle.toHtmlEscaped()));
        else
            summary->setText(QStringLiteral("Import: <b>%1</b>, <b>%2</b>, <b>%3</b> &nbsp;&middot;&nbsp; "
                                            "relations kept %4, dropped %5")
                .arg(Plural(plan.classes, "class", "classes"),
                     Plural(plan.externClasses, "extern class", "extern classes"),
                     Plural(plan.otherTypes, "other type", "other types"))
                .arg(plan.relationsKept).arg(plan.relationsDropped));

        QStringList lines;
        for (const std::string& n : plan.notes)
            lines << QString::fromStdString(n);
        notes->setPlainText(lines.join(QLatin1Char('\n')));
        notes->setVisible(!lines.isEmpty());
        bb->button(QDialogButtonBox::Ok)->setEnabled(total > 0);
        applyFilter();
    };

    auto loadSource = [&](int index) {
        pSrc = sources[(size_t)index].dmd;
        Cb_ImportCandidates(pSrc, pTgt, plan);
        tree->clear();
        items.clear();
        for (const CbImportRow& r : plan.rows)
        {
            QTreeWidgetItem* it = new QTreeWidgetItem(tree);
            const QString name = QString::fromStdString(r.name);
            it->setText(ColName, name);
            it->setText(ColKind, FormText(r.sourceKind));
            for (int col = ColClass; col <= ColOther; ++col)
                it->setTextAlignment(col, Qt::AlignCenter);
            it->setSizeHint(ColName, compactItemSize(tree, name, /*checkable*/ true));
            items.push_back(it);
        }
        render();
    };

    QObject::connect(tree, &QTreeWidget::itemClicked, &dlg, [&](QTreeWidgetItem* it, int col) {
        const int form = FormOfColumn(col);
        if (form == CB_IMPORT_NONE)
            return;
        const int index = tree->indexOfTopLevelItem(it);
        if (index < 0 || index >= (int)plan.rows.size())
            return;
        ApplyClick(plan.rows[(size_t)index], form);
        Cb_ImportResolve(pSrc, pTgt, plan);
        render();
    });
    QObject::connect(sourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     &dlg, [&](int index) { if (index >= 0) loadSource(index); });
    QObject::connect(filter, &QLineEdit::textChanged, &dlg, [&]() { applyFilter(); });
    QObject::connect(involvedOnly, &QCheckBox::toggled, &dlg, [&]() { applyFilter(); });
    QObject::connect(clearBtn, &QPushButton::clicked, &dlg, [&]() {
        for (CbImportRow& r : plan.rows) { r.explicitForm = CB_IMPORT_NONE; r.optOut = false; }
        Cb_ImportResolve(pSrc, pTgt, plan);
        render();
    });
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    loadSource(0);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return;

    CbImportResult res;
    std::string error;
    if (!Cb_ImportExecute(pSrc, pTgt, plan, res, error))
    {
        QMessageBox::warning(nullptr, title, QString::fromStdString(error));
        return;
    }

    QMessageBox box(QMessageBox::Information, title,
        QStringLiteral("Imported into %1:\n\n"
                       "  %2 (%3, %4, %5)\n"
                       "  %6, %7\n"
                       "  %8, %9")
            .arg(targetTitle,
                 Plural(res.classes, "class", "classes"),
                 Plural(res.members, "member", "members"),
                 Plural(res.methods, "method", "methods"),
                 Plural(res.constructors, "constructor", "constructors"),
                 Plural(res.externClasses, "extern class", "extern classes"),
                 Plural(res.otherTypes, "other type", "other types"),
                 Plural(res.inherits, "inheritance", "inheritances"),
                 Plural(res.relations, "relation", "relations")),
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
