// qt/ConstructorDialog.h -- the Qt Constructor properties dialog.
//
// The form lives in ConstructorDialog.ui (Qt Designer). Drives the model
// directly: handed the live Constructor*, reads it into the controls, writes
// back on OK.
#pragma once

#include <QDialog>

class Constructor;

namespace Ui { class ConstructorDialog; }

class ConstructorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConstructorDialog(Constructor* pConstructor,
                               QWidget* parent = nullptr);
    ~ConstructorDialog();

    // Valid after exec() returns Accepted.
    bool modelChanged() const { return _modelChanged; }

private slots:
    void onImplementToggled();       // Implement gates the Inline checkbox
    void onDeleteToggled();          // "= delete" forces Implement off + locked

private:
    void accept() override;          // apply edits to the model

    Ui::ConstructorDialog* _ui;
    Constructor*           _pConstructor;
    bool                   _modelChanged = false;
};
