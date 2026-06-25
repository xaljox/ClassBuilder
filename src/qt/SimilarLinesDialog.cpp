// qt/SimilarLinesDialog.cpp -- the Qt "Insert similar lines" dialog.
//
// Ported from the MFC SimilarLinesDialog. Launched from the code editor's
// Insert -> Similar lines: a line template is expanded over the checked
// members of a class -- `@` becomes the member name, `\n` a line break --
// previewed live and produced as a code block for the editor.
//
// The generated block uses bare CR (\r) line breaks, the code editor's
// convention (matches IteratorWizard / VariableMethod).

#include "SimilarLinesDialog.h"
#include "ui_SimilarLinesDialog.h"

#include "QtSimilarLinesDialog.h"    // Qt_ShowSimilarLinesDialog
#include "QtApp.h"                   // Qt_EnsureApplication / Qt_ExecModal
#include "QtModelText.h"             // toQ
#include "QtModelIcons.h"            // Qt_ModelIcon

#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QDialogButtonBox>


#define FORWARD_ONLY
#include "ClassBuilderInclude.h"

namespace {
// C-identifier character (the MFC __iscsym): letter, digit or underscore.
bool isCSym(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_'); }
}

SimilarLinesDialog::SimilarLinesDialog(BaseClass* pBaseClass, QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::SimilarLinesDialog)
    , _pBaseClass(pBaseClass)
{
    _ui->setupUi(this);

    // Class picker -- every base class in the document; select the start one.
    DataModelDoc::BaseClassIterator iBaseClass(_pBaseClass->GetDataModelDoc());
    while (++iBaseClass)
    {
        _ui->comboClass->addItem(toQ(iBaseClass->GetName()),
            QVariant::fromValue(
                reinterpret_cast<qulonglong>(iBaseClass.Get())));
        if (iBaseClass.Get() == _pBaseClass)
            _ui->comboClass->setCurrentIndex(_ui->comboClass->count() - 1);
    }

    // Line-template history (the doc keeps a newline-separated list).
    const QString history =
        toQ(_pBaseClass->GetDataModelDoc()->GetSimilarLinesList());
    for (const QString& tmpl : history.split('\n', Qt::SkipEmptyParts))
        _ui->comboLine->addItem(tmpl);
    _ui->comboLine->setCurrentText(QString());

    fillMembersList();

    connect(_ui->comboClass, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int i) {
        _pBaseClass = reinterpret_cast<BaseClass*>(
            _ui->comboClass->itemData(i).toULongLong());
        fillMembersList();
        rebuild();
    });
    connect(_ui->comboLine, &QComboBox::editTextChanged,
            this, [this] { rebuild(); });
    connect(_ui->listMembers, &QListWidget::itemChanged,
            this, [this] { rebuild(); });
    connect(_ui->checkReverse, &QCheckBox::toggled,
            this, [this] { rebuild(); });
    connect(_ui->checkStatic, &QCheckBox::toggled,
            this, [this] { fillMembersList(); rebuild(); });
    connect(_ui->checkChrono, &QCheckBox::toggled,
            this, [this] { fillMembersList(); rebuild(); });
    connect(_ui->buttonBox, &QDialogButtonBox::accepted,
            this, &SimilarLinesDialog::accept);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected,
            this, &SimilarLinesDialog::reject);

    rebuild();
}

SimilarLinesDialog::~SimilarLinesDialog()
{
    delete _ui;
}

// Populate the member list: every member of the class (static ones only when
// "Static Members" is ticked), each checkable and checked. Sorted by name
// unless "Chronological Order" is ticked. Each item carries its Member*.
void SimilarLinesDialog::fillMembersList()
{
    _ui->listMembers->blockSignals(true);   // do not fire rebuild per item
    _ui->listMembers->clear();

    BaseClass::MemberIterator iMember(_pBaseClass);
    while (++iMember)
    {
        if (iMember->GetStatic() && !_ui->checkStatic->isChecked())
            continue;
        QListWidgetItem* item = new QListWidgetItem(
            toQ(iMember->GetItemText()), _ui->listMembers);
        item->setIcon(Qt_ModelIcon(iMember->GetIcon()));
        item->setData(Qt::UserRole, QVariant::fromValue(
            reinterpret_cast<qulonglong>(iMember.Get())));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }

    if (!_ui->checkChrono->isChecked())
        _ui->listMembers->sortItems();

    _ui->listMembers->blockSignals(false);
}

// Expand the template over the checked members -> preview + _code.
void SimilarLinesDialog::rebuild()
{
    const QString line = _ui->comboLine->currentText();
    const bool reverse = _ui->checkReverse->isChecked();
    const int count = _ui->listMembers->count();

    QString preview;
    _code.clear();

    for (int n = 0; n < count; ++n)
    {
        QListWidgetItem* item =
            _ui->listMembers->item(reverse ? count - 1 - n : n);
        if (item->checkState() != Qt::Checked)
            continue;

        Member* pMember = reinterpret_cast<Member*>(
            item->data(Qt::UserRole).toULongLong());

        QString newLine = line;

        // `@` -> member name. If the `@` is preceded by an identifier char
        // it is a name fragment -> FirstUpperName; otherwise PrefixedName.
        int index;
        while ((index = newLine.indexOf(QLatin1Char('@'))) != -1)
        {
            const QString name =
                (index > 0 && isCSym(newLine[index - 1]))
                    ? toQ(pMember->GetFirstUpperName())
                    : toQ(pMember->GetPrefixedName());
            newLine = newLine.left(index) + name + newLine.mid(index + 1);
        }

        // Literal "\n" in the template -> a line break.
        while ((index = newLine.indexOf(QStringLiteral("\\n"))) != -1)
        {
            preview += newLine.left(index) + "\n";
            _code   += newLine.left(index) + "\r";
            newLine = newLine.mid(index + 2);
        }
        preview += newLine + "\n";
        _code   += newLine + "\r";
    }

    _ui->editCode->setPlainText(preview);
    if (!_code.isEmpty())
        _code.chop(1);              // drop the trailing CR
}

// ---------------------------------------------------------------------------
// MFC entry point
// ---------------------------------------------------------------------------
bool Qt_ShowSimilarLinesDialog(BaseClass* pBaseClass, CbString& code,
                               void* ownerHwnd)
{
    Qt_EnsureApplication();

    SimilarLinesDialog dlg(pBaseClass);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;

    // Bare-CR line breaks pass through verbatim (NOT toCb).
    code = CbString(dlg.code().toLocal8Bit().constData());
    return true;
}
