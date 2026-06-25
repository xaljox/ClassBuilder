// qt/DestructorDialog.h -- the Qt Destructor properties dialog.
//
// The form lives in DestructorDialog.ui (Qt Designer). Drives the model
// directly: handed the live Destructor*, reads it into the controls, writes
// back on OK.
#pragma once

#include <QDialog>

class Destructor;

namespace Ui { class DestructorDialog; }

class DestructorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DestructorDialog(Destructor* pDestructor,
                              QWidget* parent = nullptr);
    ~DestructorDialog();

    // Valid after exec() returns Accepted.
    bool modelChanged() const { return _modelChanged; }

private slots:
    void onImplementToggled();       // Implement gates the Inline checkbox
    void onVirtualToggled();         // Virtual gates the Pure checkbox

private:
    void accept() override;          // apply edits to the model

    Ui::DestructorDialog* _ui;
    Destructor*           _pDestructor;
    bool                  _modelChanged = false;
};
