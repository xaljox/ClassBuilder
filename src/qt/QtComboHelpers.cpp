// qt/QtComboHelpers.cpp -- see header.

#include "QtComboHelpers.h"

#include <QComboBox>
#include <QCompleter>

void Qt_MakeSearchableCombo(QComboBox* combo)
{
    if (!combo)
        return;

    // Editable + popup completion: typing accumulates and narrows the
    // dropdown to matches. The non-editable QComboBox default is
    // type-to-find by first letter only (typing "CbP" jumps to the first
    // entry starting with "P", not the entry starting with "CbP").
    // NoInsert keeps typed text from being appended as a phantom item.
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);
    if (QCompleter* c = combo->completer())
    {
        c->setCompletionMode(QCompleter::PopupCompletion);
        c->setCaseSensitivity(Qt::CaseInsensitive);
        c->setFilterMode(Qt::MatchContains);
    }
}
