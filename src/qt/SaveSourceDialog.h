// qt/SaveSourceDialog.h -- the Qt "Save source files" dialog.
//
// The form lives in SaveSourceDialog.ui (Qt Designer). Ported from the MFC
// CSaveSourceDialog. It both runs the save (Save All / Save Modifications
// buttons drive DataModel::SaveAllFiles / SaveModifiedFiles) and *is* the
// progress sink: it implements SourceLogInterface so the model's codegen
// reports log lines and progress back into the dialog's widgets.
#pragma once

#include <QDialog>

#include "SourceLogInterface.h"

class DataModel;

namespace Ui { class SaveSourceDialog; }

class SaveSourceDialog : public QDialog, public SourceLogInterface
{
    Q_OBJECT
public:
    explicit SaveSourceDialog(DataModel* pDataModel, QWidget* parent = nullptr);
    ~SaveSourceDialog();

    // SourceLogInterface -- called by the model during code generation.
    void AddLog(const CbString& log) override;
    void AddLogError(const CbString& log) override;
    void AddLogWarning(const CbString& log) override;
    void StepProgress() override;

private:
    void runSave(bool modifiedOnly);

    Ui::SaveSourceDialog* _ui;
    DataModel* _pDataModel;
    bool _error   = false;
    bool _warning = false;
};
