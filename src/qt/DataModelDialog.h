// qt/DataModelDialog.h -- the Qt DataModel properties dialog.
//
// The form lives in DataModelDialog.ui (Qt Designer), compiled by AUTOUIC
// into ui_DataModelDialog.h. First Qt dialog of the port that drives the
// model directly: it is handed the live DataModel* and reads / writes it --
// no value-struct bridge. See QtDataModelDialog.h for the MFC entry point.
#pragma once

#include <QDialog>
#include <QString>

class DataModel;

namespace Ui { class DataModelDialog; }

class DataModelDialog : public QDialog
{
    Q_OBJECT
public:
    DataModelDialog(DataModel* pDataModel, const QString& classNameIn,
                    QWidget* parent = nullptr);
    ~DataModelDialog();

    // Valid after exec() returns Accepted.
    QString className()   const { return _className; }
    bool    modelChanged() const { return _modelChanged; }

private slots:
    void onSerializeToggled();
    void onNameChanged();
    void onCppHeader();
    void onHHeader();
    void onCompactVersion();

private:
    void accept() override;          // validate, then write the model
    void markModelChanged();         // _modelChanged = true + mark doc dirty

    Ui::DataModelDialog* _ui;
    DataModel*           _pDataModel;
    QString              _className;
    bool                 _modelChanged = false;
};
