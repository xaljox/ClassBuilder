// qt/RelationDiagramOnlyDialog.h -- the Qt "Relation ClassDiagram Only"
// properties dialog.
//
// The form lives in RelationDiagramOnlyDialog.ui (Qt Designer). Drives the
// model directly. Two entry points share one apply path (applyControls):
//   - Edit:   handed a live shape; reads it in, writes back on OK.
//   - Create: handed the diagram + seed classes, NO shape. It only gathers the
//             spec; the shape is created (by the bridge) AFTER OK and the spec
//             applied to it -- the diagram-only / dependency connection is a
//             standalone shape with no tree object, so it is built directly.
#pragma once

#include <QDialog>

class RelationDiagramOnlyShape;
class ClassDiagram;
class ClassShape;

namespace Ui { class RelationDiagramOnlyDialog; }

class RelationDiagramOnlyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RelationDiagramOnlyDialog(RelationDiagramOnlyShape* pShape,
                                       QWidget* parent = nullptr);
    RelationDiagramOnlyDialog(ClassDiagram* pClassDiagram, ClassShape* pInitFrom,
                              ClassShape* pInitTo, QWidget* parent = nullptr);
    ~RelationDiagramOnlyDialog();

    // Valid after exec() returns Accepted.
    bool modelChanged() const { return _modelChanged; }

    // Create entry point (used by the bridge after OK): the chosen endpoints and
    // the shared write of the dialog's attributes onto a shape (NO routing -- the
    // caller routes: the create path does one clean MakeNewRouting()).
    ClassShape* fromShape() const;
    ClassShape* toShape() const;
    void applyAttributes(RelationDiagramOnlyShape* pShape) const;

private slots:
    void onFromClassChanged();       // combo -> the From name field
    void onToClassChanged();         // combo -> the To name field
    void onOwnedToggled();           // owned radios -> From multiplicity
    void onTypeToggled();            // type radios  -> To multiplicity

private:
    void accept() override;          // edit path: apply edits to the live shape
    void wireSignals();

    Ui::RelationDiagramOnlyDialog* _ui;
    RelationDiagramOnlyShape*      _pShape;          // edit path (null on create)
    bool                           _modelChanged = false;
};
