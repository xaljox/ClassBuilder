// qt/SearchDialog.h -- the Qt Find dialog (Ctrl+F).
//
// First .ui-based dialog of the MFC->Qt port: the form lives in
// SearchDialog.ui (Qt Designer), compiled by AUTOUIC into ui_SearchDialog.h.
#pragma once

#include <QDialog>

#include "QtSearchDialog.h"   // SearchParams

namespace Ui { class SearchDialog; }

class SearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SearchDialog(QWidget* parent = nullptr);
    ~SearchDialog();

    void setParams(const SearchParams& p);   // controls <- params
    void getParams(SearchParams& p) const;   // controls -> params

private:
    Ui::SearchDialog* _ui;
};
