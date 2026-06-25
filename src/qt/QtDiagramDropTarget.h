// qt/QtDiagramDropTarget.h -- interface implemented by the Qt diagram canvases
// (ClassDiagramCanvas, SequenceDiagramCanvas) so the main-tree drag can drop a
// Ctrl-dragged class/actor onto whichever diagram is under the cursor and preview
// a footprint ghost, without the tree knowing the concrete canvas type. The tree
// finds it by dynamic_cast'ing the widget under the cursor up its parent chain.
#pragma once

#include <QPoint>

class Gti;

class DiagramDropTarget
{
public:
    virtual ~DiagramDropTarget() = default;

    // Release: add the dragged node to this diagram at the drop point. Returns
    // false if it can't land here (DropTarget) -- the tree then falls back to the
    // in-tree drop.
    virtual bool dropFromTree(Gti* pGti, QPoint globalPos) = 0;

    // Hover: preview where the shape would land (only when it can land here);
    // returns whether a ghost is shown. clearDropGhost removes it.
    virtual bool showDropGhost(Gti* pGti, QPoint globalPos) = 0;
    virtual void clearDropGhost() = 0;
};
