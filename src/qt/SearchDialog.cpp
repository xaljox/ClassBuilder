// qt/SearchDialog.cpp -- Qt Find dialog + the MFC bridge.
//
// Pure Qt: no MFC headers. The form is SearchDialog.ui (Qt Designer);
// AUTOUIC generates ui_SearchDialog.h with the Ui::SearchDialog class.

#include "SearchDialog.h"
#include "ui_SearchDialog.h"
#include "QtApp.h"

#include <QString>

SearchDialog::SearchDialog(QWidget* parent)
    : QDialog(parent)
    , _ui(new Ui::SearchDialog)
{
    _ui->setupUi(this);
}

SearchDialog::~SearchDialog()
{
    delete _ui;
}

void SearchDialog::setParams(const SearchParams& p)
{
    _ui->lineEditText->setText(QString::fromLocal8Bit(p.text.c_str()));
    _ui->checkMatchCase->setChecked(p.matchCase);
    _ui->checkWholeName->setChecked(p.wholeName);
    _ui->checkTypes->setChecked(p.searchTypes);
    _ui->checkMethods->setChecked(p.searchMethods);
    _ui->checkArguments->setChecked(p.searchArguments);
    _ui->checkMembers->setChecked(p.searchMembers);
    _ui->lineEditText->selectAll();   // ready to overtype, like the MFC dialog
}

void SearchDialog::getParams(SearchParams& p) const
{
    const QByteArray text = _ui->lineEditText->text().toLocal8Bit();
    p.text.assign(text.constData(), static_cast<size_t>(text.size()));
    p.matchCase       = _ui->checkMatchCase->isChecked();
    p.wholeName       = _ui->checkWholeName->isChecked();
    p.searchTypes   = _ui->checkTypes->isChecked();
    p.searchMethods   = _ui->checkMethods->isChecked();
    p.searchArguments = _ui->checkArguments->isChecked();
    p.searchMembers   = _ui->checkMembers->isChecked();
}

// --- MFC bridge (declared in QtSearchDialog.h) -----------------------------

bool Qt_ShowSearchDialog(SearchParams& params, void* ownerHwnd)
{
    Qt_EnsureApplication();

    SearchDialog dlg;
    dlg.setParams(params);
    if (Qt_ExecModal(dlg, ownerHwnd) != QDialog::Accepted)
        return false;
    dlg.getParams(params);
    return true;
}
