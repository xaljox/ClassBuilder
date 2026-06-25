// qt/SimilarLinesDialog.h -- the Qt "Insert similar lines" dialog.
//
// The form lives in SimilarLinesDialog.ui (Qt Designer). Ported from the MFC
// SimilarLinesDialog: expands a line template (with `@` member-name and `\n`
// line-break placeholders) over the checked members of a class, previewing
// and producing a generated code block for the editor. Drives the model
// directly.
#pragma once

#include <QDialog>
#include <QString>

class BaseClass;

namespace Ui { class SimilarLinesDialog; }

class SimilarLinesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SimilarLinesDialog(BaseClass* pBaseClass,
                                QWidget* parent = nullptr);
    ~SimilarLinesDialog();

    // The generated block (CR-separated lines), valid once OK is pressed.
    QString code() const { return _code; }

private:
    void fillMembersList();
    void rebuild();                 // recompute the preview + _code

    Ui::SimilarLinesDialog* _ui;
    BaseClass*              _pBaseClass;
    QString                 _code;
};
