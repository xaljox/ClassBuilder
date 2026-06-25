// qt/ProjectSettingsDialog.h -- the Qt Project Settings dialog.
//
// The form lives in ProjectSettingsDialog.ui (Qt Designer). Drives the model
// directly: handed the live DataModelDoc*, reads it into the controls and
// writes back on OK.
#pragma once

#include <QDialog>

class DataModelDoc;

namespace Ui { class ProjectSettingsDialog; }

class ProjectSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProjectSettingsDialog(DataModelDoc* pDataModelDoc,
                                   QWidget* parent = nullptr);
    ~ProjectSettingsDialog();

private:
    void accept() override;          // apply changes to the model

    Ui::ProjectSettingsDialog* _ui;
    DataModelDoc*              _pDataModelDoc;
};
