// qt/QtSearchDialog.h -- bridge into the Qt Find dialog (Ctrl+F).
//
// Qt-free and MFC-free, so the MFC view code can include it. The MFC
// SearchDialog was a pure value dialog (no model access) -- its settings are
// carried here as a plain struct.
#pragma once

#include <string>

// Find-dialog parameters. The defaults reproduce the original first-use
// behaviour (case-insensitive substring search across all kinds).
struct SearchParams
{
    std::string text;
    bool matchCase       = false;
    bool wholeName       = false;
    bool searchTypes     = true;
    bool searchMethods   = true;
    bool searchArguments = true;
    bool searchMembers   = true;
};

// Shows the modal Find dialog. `params` is in/out -- seeded into the controls
// on entry, written back on OK. `ownerHwnd` is the MFC owner window (an HWND
// passed as void* to keep this header MFC-free). Returns true if OK pressed.
bool Qt_ShowSearchDialog(SearchParams& params, void* ownerHwnd);
