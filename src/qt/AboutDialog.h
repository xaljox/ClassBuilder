// qt/AboutDialog.h -- the Qt "About ClassBuilder" dialog.
//
// The first widget of the MFC->Qt port. Plain hand-coded QDialog (no .ui
// file yet -- Qt Designer comes in at the next port step).
#pragma once

#include <QDialog>

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent = nullptr);
};
