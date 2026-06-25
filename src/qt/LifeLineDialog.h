// qt/LifeLineDialog.h -- the Qt Life Line dialog.
//
// The form lives in LifeLineDialog.ui (Qt Designer). Ported from the MFC
// LifeLineDialog: name / actor-or-class / template / show-activations / note
// of a sequence-diagram life line. Two modes (see QtLifeLineDialog.h): edit an
// existing shape, or pick the actor/class for a new one. Drives the model
// directly.
#pragma once

#include <QDialog>
#include <QString>

class LifeLineShape;
class DataModelDoc;
class Actor;
class BaseClass;

namespace Ui { class LifeLineDialog; }

class LifeLineDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LifeLineDialog(LifeLineShape* pLifeLineShape,
                            QWidget* parent = nullptr);   // edit mode
    explicit LifeLineDialog(DataModelDoc* pDataModelDoc,
                            QWidget* parent = nullptr);   // new mode
    ~LifeLineDialog();

    Actor*     actor() const     { return _pActor; }
    BaseClass* baseClass() const { return _pBaseClass; }
    QString    templateText() const;
    bool       showActivations() const;
    QString    nameText() const;
    QString    noteText() const;

private:
    void buildCommon();
    void onSelectionChanged();
    void accept() override;

    Ui::LifeLineDialog* _ui;
    LifeLineShape* _pLifeLineShape = nullptr;
    DataModelDoc*  _pDataModelDoc  = nullptr;
    Actor*         _pActor         = nullptr;   // original, then result
    BaseClass*     _pBaseClass     = nullptr;   // original, then result
    BaseClass*     _origBaseClass  = nullptr;
    QString        _origTemplate;
};
