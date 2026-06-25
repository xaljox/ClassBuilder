// CbShellHooks.h -- the seam between the model/pipe layer and the Qt shell.
//
// The pipe server (CbCommandServer) used to reach the document through MFC's
// frame/doc-template machinery; with the Qt shell there is ONE document,
// owned by the shell window, exposed through these accessors. Implemented in
// qt/QtShellWindow.cpp. MFC-free and Qt-free.
#pragma once

class CClassBuilderDoc;

// The single open document (never null once the shell is up).
CClassBuilderDoc* Cb_ActiveDoc();
void              Cb_SetActiveDoc(CClassBuilderDoc* pDoc);

// Shell document actions, used by the pipe's new_model* / open_doc commands.
// Both run the full shell flow (tree rebuild, window title) and return the
// document on success, null on failure/cancel.
CClassBuilderDoc* Cb_ShellNewDocument();
CClassBuilderDoc* Cb_ShellOpenDocument(const char* path);

// Open-document enumeration + headless close, for the pipe's document-selection
// commands (list_documents / select_document / current_document / close_document).
// Index is into the shell's open-document list (oldest first); the doc POINTER is
// the stable handle the pipe stores as its sticky server-side target.
int               Cb_DocumentCount();
CClassBuilderDoc* Cb_DocumentAt(int index);                  // null if out of range
bool              Cb_IsDocumentOpen(CClassBuilderDoc* pDoc); // is the pointer still a live doc?
// Close WITHOUT a save prompt (headless). save=true writes the .cbz first, but
// only if the doc already has a path (an untitled doc is closed without saving).
// Returns false if the doc is not found.
bool              Cb_CloseDocument(CClassBuilderDoc* pDoc, bool save);
