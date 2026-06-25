// qt/QtComboHelpers.h -- shared QComboBox setup for long pickers.
//
// The default non-editable QComboBox is type-to-find by first letter only:
// typing "CbP" jumps to the first C, then the first B, then the first P, so
// finding "CbPainter" in a long list of model types is hard. This helper
// flips the combo into editable mode and installs a popup completer that
// accumulates keystrokes and narrows the dropdown to matches.
//
// Composes with the existing helpers in qt/QtCompact.h (`compactCombo` for
// row height) and any per-dialog sort -- call AFTER those, after the items
// are populated.
#pragma once

class QComboBox;

void Qt_MakeSearchableCombo(QComboBox* combo);
