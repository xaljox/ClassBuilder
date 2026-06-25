// qt/QtPrintDialog.h -- bridge into the Qt Print dialog.
//
// Qt-free and MFC-free, so the MFC code can include it. The dialog only
// reads the diagram (width / height / multi-page) -- it does not write the
// model.
#pragma once

class ClassDiagram;
class SequenceDiagram;

// Show the modal Print dialog for a class / sequence diagram over the MFC
// owner window. Returns the number of pages to print (1, 2, 4, 8 or 16), or
// 0 when the dialog was cancelled.
int Qt_ShowPrintDialogClass(ClassDiagram* pClassDiagram, void* ownerHwnd);
int Qt_ShowPrintDialogSequence(SequenceDiagram* pSequenceDiagram,
                               void* ownerHwnd);
