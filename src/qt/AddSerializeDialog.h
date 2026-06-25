// qt/AddSerializeDialog.h -- the Qt "Add Serialize" dialog.
//
// The form lives in AddSerializeDialog.ui (Qt Designer). Like DataModelDialog
// it drives the model directly -- it is handed the live DataModelDoc* and
// validates against it. See QtAddSerializeDialog.h for the MFC entry point.
#pragma once

#include <QDialog>
#include <QString>

class DataModelDoc;

namespace Ui { class AddSerializeDialog; }

class AddSerializeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddSerializeDialog(DataModelDoc* pDataModelDoc,
                                QWidget* parent = nullptr);
    ~AddSerializeDialog();

    // Valid after exec() returns Accepted.
    QString className() const { return _className; }

private:
    void accept() override;          // validate, then close

    Ui::AddSerializeDialog* _ui;
    DataModelDoc*           _pDataModelDoc;
    QString                 _className;
};
