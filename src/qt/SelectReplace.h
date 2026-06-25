// qt/SelectReplace.h -- the Qt "select & replace" dialog.
//
// The form lives in SelectReplace.ui. Ported from the MFC SelectReplace: a
// checkbox tree of every method whose body references a renamed method's old
// name, grouped by class. OK ripples the rename through the checked bodies;
// double-clicking a method opens its (modal) code editor. Drives the model
// directly.
#pragma once

#include <QDialog>

#include "CbString.h"

class DataModel;
class QTreeWidgetItem;

namespace Ui { class SelectReplace; }

class SelectReplace : public QDialog
{
    Q_OBJECT
public:
    SelectReplace(DataModel* pDataModel,
                  const CbString& oldString, const CbString& newString,
                  QWidget* parent = nullptr);
    ~SelectReplace();

private:
    void fillTree();
    void setAllChecked(bool checked);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void accept() override;

    Ui::SelectReplace* _ui;
    DataModel* _pDataModel;
    CbString   _oldString;
    CbString   _newString;
};
