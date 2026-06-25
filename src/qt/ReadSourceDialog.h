// qt/ReadSourceDialog.h -- the Qt "Read source files" dialog.
//
// The form lives in ReadSourceDialog.ui (Qt Designer). Ported from the MFC
// CReadSourceDialog. It reads the on-disk .h/.cpp back into the model and is
// the parse-log sink: it implements ParseLogInterface so the model's reader
// and the flex/yacc parser report log lines and progress into its widgets.
#pragma once

#include <QDialog>

#include "ParseLogInterface.h"

class DataModel;

namespace Ui { class ReadSourceDialog; }

class ReadSourceDialog : public QDialog, public ParseLogInterface
{
    Q_OBJECT
public:
    explicit ReadSourceDialog(DataModel* pDataModel, QWidget* parent = nullptr);
    ~ReadSourceDialog();

    // ParseLogInterface -- called by the model / parser during reading.
    void AddLog(const char* log) override;
    void AddLogError(const char* log) override;
    void AddLogWarning(const char* log) override;
    void StepProgress() override;

private:
    void runRead(bool readAll);

    Ui::ReadSourceDialog* _ui;
    DataModel* _pDataModel;
    bool _error   = false;
    bool _warning = false;
};
