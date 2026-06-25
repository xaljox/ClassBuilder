# MFC -> Qt dialog migration

Tracking list for porting ClassBuilder's MFC dialogs to Qt. The model is now
MFC-free and Qt-includable (`ClassBuilderInclude.h` with `FORWARD_ONLY`), so
ported dialogs call the model directly -- no value-struct bridge needed (the
`SearchParams` style was only a stop-gap before the model was includable).

Pattern per dialog: `qt/<Name>.ui` (Qt Designer form) + `qt/<Name>.{h,cpp}`
(the QDialog) + `qt/Qt<Name>.h` (the MFC-callable entry point). Add the files
to `ClassBuilderQt` in CMakeLists, rewire the MFC call site(s), then the MFC
`<Name>.{h,cpp}` become dead and can be deleted.

## Deferred polish (after the port)

- SequenceDiagramDialog (and 2 other diagram dialogs) had an illustration strip
  in the "Multiple page support" group -- the MFC .rc wired its SS_BITMAP to
  method-ICON resources, so it was dropped in the port. If a real page-layout
  illustration is wanted, add it as a QLabel + pixmap swapped by the
  orientation radios. Needs the source artwork.

- The tree icons (qt/QtModelIcons + the /icons resources.qrc set) are the
  original 20+-year-old 16x16/32x32 res/*.ico files. They look dated / soft
  on modern high-DPI monitors. Revisit with redrawn higher-resolution
  artwork; QtModelIcons + the .qrc are the only things to repoint.

- Dialog tree zoom: the MFC dialogs sized their tree image list to
  CClassBuilderView::GetDefaultIconSize(), so dialog trees tracked the main
  view's zoom level. The Qt ports (QtModelIcons + CbTreeWidget) use a fixed
  icon/row size and do NOT follow the main-view zoom. Accepted as OK for now
  (2026-05-18) -- a fixed size in dialogs is fine; only the value is open
  (20 vs ~24px). Dialog tree rows currently share the text lists'
  compactItemSize height, so a list and a tree side by side in one dialog
  line up -- keep that consistency. When the res/*.ico icons are redrawn as
  SVG, experiment with a comfortable fixed row height then. If main-view zoom
  sync is ever wanted, feed it into QtModelIcons / CbTreeWidget -- revisit
  when the main class tree is ported to Qt.

- Tree connector lines: qt/tree/*.svg, drawn by CbTreeWidget::drawBranches
  (NOT a stylesheet -- a QTreeView::branch stylesheet has no notion of depth,
  so it cannot tell a root leaf from a nested last-child). The SVG rect
  thicknesses deliberately differ (~vline 0.6, stub 0.9) and abut rather than
  overlap -- tuned for the current fixed-size dialog rows. When the zoomable
  main class tree is ported these may want re-tuning for larger rows.

## Dialog font / row height (settled 2026-05-19)

- UI font: **11 pt**, app-wide, one knob `CB_UI_FONT_PT` in qt/QtApp.cpp. Set
  BOTH via QApplication::setFont (so fontMetrics() is right at widget
  construction) AND a `QWidget { font-size }` stylesheet rule (authoritative
  for rendering). Use an ABSOLUTE point size -- the Windows default font is
  pixel-defined, so `pointSizeF()` returns -1 and "default + N" arithmetic
  silently does nothing. 12pt felt too big; 11pt ~= the MFC size.
- Row heights: a plain row = exactly `fontMetrics().height()` (no padding --
  the line height already includes leading). Tree: CbTreeWidget. List:
  QtCompact `compactItemSize` = `fm.height() + (checkable ? 2 : 0)`. The +2
  for checkbox rows just keeps the indicator from touching; plain rows get 0
  so a list and a tree line up. If a checkbox list ever sits beside a plain
  list, dropping its +2 to 0 for alignment is preferred over a mismatch.

## Shared Qt tree infrastructure

- qt/QtModelIcons.{h,cpp} -- maps a model GetIcon() index (the ICON_*
  constants) to a QIcon, loaded from the embedded res/*.ico set (resources.qrc
  prefix /icons). Order MUST match CClassBuilderView::InitImageList. Reuse for
  EVERY Qt tree/list dialog so they match the main class tree. First user:
  FindMethodDialog.

- qt/CbTreeWidget.{h,cpp} -- QTreeWidget subclass; drawBranches() picks a
  qt/tree/*.svg branch glyph per node (vline / branch_more / branch_end /
  branch_top / branch_closed / branch_open) from the node's depth + sibling
  state. Use CbTreeWidget (promote in the .ui) for EVERY Qt model tree so
  branches are consistent. Gated on CB_HAVE_SVG; falls back to the native
  QTreeWidget branches without the Svg module. Users: FindMethodDialog,
  IteratorWizardDialog.

## Done

- [x] AboutDialog
- [x] CommentHeaderDialog  (replaces the MFC CppHeaderDialog + HHeaderDialog)
- [x] SearchDialog         (Ctrl+F)
- [x] DataModelDialog      (first dialog calling the model directly)
- [x] AddSerializeDialog   (Project -> Add Serialize)
- [x] ActorDialog          (Actor -> edit attributes)
- [x] ProjectSettingsDialog (Project -> Settings)
- [x] GroupDialog          (Group -> edit attributes)
- [x] MemberMethodsDialog  (Member[class type] -> Add Method; first QListWidget dialog)
- [x] TypeDialog           (OtherType -> edit attributes)
- [x] ConstructorDialog    (Constructor -> edit attributes)
- [x] ContextDialog        (Assign Context; two-list transfer, 5 call sites)
- [x] DestructorDialog     (Destructor -> edit attributes)
- [x] DependencyDialog     (class diagram: dependency arrow -> edit attributes)
- [x] NoteShapeDialog      (class- and sequence-diagram notes; double-click a note)
- [x] TypeVariableDialog   (code editor: Insert -> Type Variable)
- [x] SequenceDiagramDialog (sequence diagram -> edit attributes; multi-page illustration)
- [x] RelationDiagramOnlyDialog (class diagram: diagram-only relation -> edit attributes)
- [x] UserSectionsDialog       (Edit -> User Sections, Ctrl+U; was the MFC
      MfcWizardSupportDialog -- renamed: it is just a six-//@START_USER editor)
- [x] ClassDiagramDialog       (class diagram -> edit attributes; paper size +
      orientation + multi-page illustration, like SequenceDiagramDialog)
- [x] IsClassMethodsDialog     (Class / MemberAndMethodGroup -> Add IsClass
      Methods; first of the MyCListBox list-based group)
- [x] ClassShapeDialog         (class diagram: Class -> Select Members/Methods
      on a single class shape; first two-list dialog)
- [x] ClassShapeOrderDialog    (class diagram: Class Diagram -> Reorder
      Members/Methods; two single-select lists, Move Up/Down replaces the
      MFC spin controls)
- [x] VirtualMethodsDialog     (ExternClass/MemberAndMethodGroup -> Add
      Virtuals; Method -> Add Virtuals; two modes in one Qt class)
- [x] ExceptionSpecificationDialog (Method -> Edit Exception Specification;
      transfer pair + per-type property panel; live-apply, caller rolls back
      on Cancel)

## Superseded / dead -- no port needed

- CppHeaderDialog, HHeaderDialog -- replaced by the Qt CommentHeaderDialog.
  (All four MFC files deleted 2026-05-17.)

## List-based dialogs -- ALL DONE

The QListWidget pattern is established (MemberMethodsDialog -- multi-select
list, model pointer per item in Qt::UserRole). These dialogs embed the custom
`MyCListBox` (a CListBox subclass -- the original flat-scan keyed on `CListBox`
literally and missed it). They are NOT flat, but no longer blocked; port like
MemberMethodsDialog.

All five ported: IsClassMethods, ClassShape, ClassShapeOrder, VirtualMethods,
ExceptionSpecification (see the Done list above).

STANDARD for every multi-pick checkbox-list dialog: per-row `Qt::ItemIsUser
Checkable` checkboxes (the tickability must be obvious -- clearer than
Ctrl/Shift extended-selection) PLUS a `&Select All` / `&Unselect All` button
row. The list AND the button row live together inside a titled `QGroupBox`
(the box title carries what was the list label) -- the frame makes the
buttons visibly belong to the list, so the second row does not look stray.
The dialog's OK/Cancel `QDialogButtonBox` sits below the group box. See
IsClassMethodsDialog / MemberMethodsDialog for the `setAllChecked(bool)`
helper + .ui layout to copy.


## Deferred -- bigger tree/list dialogs

More than a single list -- a tree, or list(s) plus heavy logic. Do after the
list-based group.

- [x] FindMethodDialog         (FindMethod -> Edit Attributes; first tree
      dialog -- QTreeWidget with the shared model icons (qt/QtModelIcons),
      non-live fields applied on OK)
- [x] IteratorWizardDialog   (code editor: Insert -> Iterator; variable tree
      + relation list + filter list, emits a C++ iterator snippet)
- [x] VariableMethodDialog    (code editor: Insert -> Variable->Method();
      single CbTreeWidget, emits a member/method access path)

## Flat dialogs to migrate

Flat = only text fields / checkboxes / combos / buttons. Ordered small -> large
by .cpp size (a rough effort gauge, not strict difficulty).

### Small (< 200 lines) -- good warm-ups


### Medium (200-400 lines)

- [x] ContextDeclarationDialog  (DataModel -> Edit Context; list + property
      panel, live-apply, caller rolls back on Cancel)
- [x] ExternClassDialog        (ExternClass -> Edit Attributes; flat: name/
      struct/member-prefix/template/suppress; non-live)
- [x] UserCodeDialog           (Class user-code section editor; modal;
      first consumer of the CodeEditor QPlainTextEdit widget)
- [x] SimilarLinesDialog       (code editor: Insert Similar Lines; template
      expanded over a checkbox member list)
- [x] SelectClassesDialog      (class diagram: Class Diagram -> Select
      Classes; checkbox tree, adds/removes class shapes on OK)
- [x] LifeLineDialog           (sequence diagram life line: open existing /
      create new; dual-mode -- two bridge entry points)
- [x] InheritDialog            (Inherit -> Edit Attributes; base-class combo /
      access / virtual / template / note; non-live)
- [~] WriteRtfDialog           (~390)  -- EXCISE, do not port: the whole RTF
      doc path is dropped (see auto-memory doc-rtf-roundtrip note)

### Large (> 400 lines) -- flat by widget type, heavy logic

- [x] ArgumentDialog           (Argument -> Edit Attributes; flat property
      editor; non-live, virtual-override propagation in applyFieldChanges)
- [x] SignalDialog            (sequence diagram signal: open / create; method
      combo + display flags + clause/label/return/note; applies on OK.
      New Method still routes through MFC Method::OnEditAttributes)
- [x] ClassDialog             (Class -> Edit Attributes; name/files/template/
      properties/note; non-live; serialize<->template mutual gating)
- [x] MethodDialog            (Method -> Edit Attributes; type/name/template/
      property flags; non-live; propagates to virtual overrides on apply)
- [x] RelationDialog           (From/ToRelation -> Edit Attributes; from/to
      class+name, association type, impl (standard/value-tree/avl-tree),
      member combo; non-live)
- [x] MemberDialog            (Member -> Edit Attributes; type/name/template/
      flags/get-set access/bit-field/init/note; non-live)

## Complex / late -- process / IO dialogs

Not property editors -- progress/console dialogs woven into long-running
operations. Defer with the code-edit dialogs.

- [x] ReadSourceDialog        -- "Read source files"; CReadSourceDialog extern
      class renamed to ParseLogInterface (abstract, const char* args so the
      parser's CString and the model's CbString both convert). Qt dialog
      implements it; auto-reads modifications on open. Read.y/.cpp parser
      retargeted (g_pDialog type + MyCString->CbStringBuilder).
- [x] SaveSourceDialog        -- code-generation progress sink; CSaveSourceDialog
      extern class renamed to SourceLogInterface (abstract, MFC/Qt-free) so the
      model's codegen methods need no change; Qt dialog implements it
- [x] PrintDialog             (Class/SequenceDiagramView -> File Print; flat
      radio 1/2/4/8/16 pages + orientation hint; pure UI, no model write)
- [~] WriteRtfDialog          (~390)  -- EXCISE, do not port: RTF doc path
      dropped; archive RTF code apart, vacate 4 DataModel _rtf* members

## Complex / late -- the code-edit dialogs   (DONE 2026-05-19)

MethodCodeDialog, ConstructorCodeDialog and SelectReplace -- the last three --
were ported together (they cross-reference each other's types, so all-or-
nothing for a green build). Notes for the record:

- the custom MFC CCodeEdit was replaced by the Qt `CodeEditor` widget
  (QPlainTextEdit subclass: monospace, C++ auto-indent, strippedCode /
  insertSnippet / static codeFont). CCodeEdit + CMultiTreeCtrl are now dead
  and were deleted.
- ported MODAL, not modeless: the MFC host still owns the message loop. The
  modeless `GetOpenDialog()`/`SetOpenDialog()`/`UpdateCode()` plumbing in
  Method.cpp + Constructor.cpp is now dead and was neutered. The modeless
  flip is deferred to the full-Qt stage.
- menu integration is a QMenuBar built in code (File / Edit / Add / Insert).
- hub deps (IteratorWizard, TypeVariable, VariableMethod, SimilarLines,
  Argument, ExceptionSpecification) were all ported earlier.

- [x] MethodCodeDialog        -- modal; marker strip + CodeEditor + menu bar
- [x] ConstructorCodeDialog   -- modal; two CodeEditors (init list + body),
      focus-tracked Edit/Insert routing

## Missed in the initial inventory (not *Dialog-named)

Found 2026-05-19 by a `: public CDialog` sweep -- these are real modal
dialogs whose class names don't end in "Dialog", so the first inventory
(built from *Dialog.{cpp,h}) skipped them. All modal, portable now.

- [x] InheritByTree + InheritFromTree -- merged into one Qt class
      InheritTreeDialog (mode InheritsFrom / InheritedBy); read-only
      CbTreeWidget, no buttons; MFC font/imagelist/sort plumbing dropped.
- [x] SelectMembersAndMethods -- ClassDiagramView; checkbox tree of each class
      shape's members/methods + 9 filter checkboxes; Apply/OK apply, Cancel
      rolls back to the ctor undo point. CbTreeWidget; no hub deps.
- [x] SelectReplace           -- method-rename ripple: on renaming a
      method, a checkbox tree of methods whose code body uses the old name;
      OK runs ReplaceInCode(old->new). Invoked from Method.cpp CODE_1398.
      Double-click opens the (modal) Method/ConstructorCodeDialog -- ported
      together with the code-editor group 2026-05-19.

## Notes

- Confirm liveness when picking a dialog up -- a few MFC dialogs may already be
  dead (as CppHeaderDialog/HHeaderDialog turned out to be). Check that its
  `C<Name>` class is still constructed somewhere live.
- The 3 diagram-shape dialogs (ClassShape, ClassShapeOrder, NoteShape) and the
  IO dialogs (Read/SaveSource, Print, WriteRtf) are not model-property editors;
  they may need their owning view ported around the same time.
