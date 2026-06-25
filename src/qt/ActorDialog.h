// qt/ActorDialog.h -- the Qt Actor properties dialog.
//
// The form lives in ActorDialog.ui (Qt Designer). Drives the model directly:
// handed the live Actor*, reads it into the controls, writes back on OK.
#pragma once

#include <QDialog>

class Actor;

namespace Ui { class ActorDialog; }

class ActorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ActorDialog(Actor* pActor, QWidget* parent = nullptr);
    ~ActorDialog();

    // Valid after exec() returns Accepted.
    bool modelChanged() const { return _modelChanged; }

private:
    void accept() override;          // validate, then write the model

    Ui::ActorDialog* _ui;
    Actor*           _pActor;
    bool             _modelChanged = false;
};
