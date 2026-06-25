// qt/QtModelIcons.h -- the CB model tree icons, as QIcons.
//
// The MFC views build a CImageList of ~67 icons in a fixed order; the model
// objects' GetIcon() returns an index into it (the ICON_* constants in
// ClassBuilderInclude.h). Qt tree dialogs reuse those same icons so they look
// familiar against the main class tree -- this maps the index to a QIcon
// loaded from the embedded .ico resources (qt/resources.qrc, prefix /icons).
//
// The index order here MUST match CClassBuilderView::InitImageList.
//
// A redrawn vector icon dropped in qt/icons/<name>.svg is used in preference
// to the legacy <name>.ico (when this Qt has the Svg module), so the set can
// be modernised one glyph at a time. See qt/icons/README.md.
#pragma once

#include <QIcon>

// QIcon for the given CB icon index (an ICON_* value / GetIcon() result).
// Returns a null QIcon for an out-of-range index.
QIcon Qt_ModelIcon(int index);

// QIcon for a phase state marker (the MFC tree's state image): phase 1..5 =
// Analysis / Design / Implementation / Test / Complete. Returns a null QIcon
// for 0 (None_Phase) or out of range -- callers show no marker then.
QIcon Qt_PhaseIcon(int phase);
