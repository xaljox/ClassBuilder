// qt/DependencyDialog.h -- the Qt Dependency properties dialog.
//
// The form lives in DependencyDialog.ui (Qt Designer). Drives the model
// directly. Edit and Create share applyControls() (one apply path). Create
// gathers the spec only; the shape is built by the bridge AFTER OK (a dependency
// is a standalone diagram shape with no tree object, so it is created directly).
#pragma once

#include <QDialog>

class DependencyShape;
class ClassDiagram;
class ClassShape;

namespace Ui { class DependencyDialog; }

class DependencyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DependencyDialog(DependencyShape* pDependencyShape,
                              QWidget* parent = nullptr);
    DependencyDialog(ClassDiagram* pClassDiagram, ClassShape* pInitFrom,
                     ClassShape* pInitTo, QWidget* parent = nullptr);
    ~DependencyDialog();

    // Valid after exec() returns Accepted.
    bool modelChanged() const { return _modelChanged; }

    // Create entry point (used by the bridge after OK). applyAttributes writes the
    // dialog's fields onto a shape (NO routing -- the caller routes).
    ClassShape* fromShape() const;
    ClassShape* toShape() const;
    void applyAttributes(DependencyShape* pShape) const;

private:
    void accept() override;          // edit path: apply edits to the live shape

    Ui::DependencyDialog* _ui;
    DependencyShape*      _pDependencyShape;   // edit path (null on create)
    bool                  _modelChanged = false;
};
