// qt/QtImportDialog.h -- Qt-free bridge into the "Import / Clone" action.
// Copy classes and types from another OPEN project into this one -- the GUI over
// the CbImport engine (model/CbImport.h), shared with the import_plan /
// import_execute commands.
//
// Qt-free signature so the MFC-free model (ClassBuilderDoc) can call it.
#pragma once

class DataModelDoc;

// Modal over ownerHwnd. Target = pTarget (the active project); source = another
// open project (a dropdown when there is more than one). Lists the source's
// classes / extern classes / other types the target lacks; each row is imported
// as the form ticked in its Class / Extern class / Other type column, and a Class
// pick pre-ticks what it needs. Imports as ONE undo step. Shows an info box (and
// does nothing else) when no other project is open.
void Qt_ShowImportDialog(DataModelDoc* pTarget, void* ownerHwnd);
