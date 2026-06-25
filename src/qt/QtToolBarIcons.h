// qt/QtToolBarIcons.h -- the original MFC main-toolbar glyphs, as QIcons.
//
// The MFC build's main toolbar (IDR_MAINFRAME) is a single 16x16 bitmap strip,
// res/Toolbar.bmp -- NOT the per-node tree icons (those are the .ico set behind
// Qt_ModelIcon). Its tiles are the real Add-Class / Add-Note / Add-Lifeline /
// Add-Message / Read-Source / ... button glyphs. This slices that strip (bundled
// via qt/resources.qrc as :/Toolbar.bmp) into individual QIcons, keyed by the
// ToolGlyph index = the button's position in the IDR_MAINFRAME TOOLBAR list
// (separators don't count). The strip's Cb_RGB(192,192,192) is made transparent.
#pragma once

#include <QIcon>

// Shared icon size for the embedded view toolbars (tree + class/sequence
// diagram). One place to tune them together. Bumped from 16 now that crisp
// SVG glyphs can back them (CB_HAVE_SVG); the legacy raster upsamples a touch.
inline constexpr int CB_TOOLBAR_ICON_PX = 20;

// Tile index in res/Toolbar.bmp == button order in the IDR_MAINFRAME toolbar
// (see ClassBuilder/mfc_retired/ClassBuilder.rc). Named so call sites read
// clearly and don't bake magic numbers.
enum ToolGlyph {
    TG_FILE_NEW = 0, TG_FILE_OPEN, TG_FILE_SAVE,
    TG_EDIT_UNDO, TG_EDIT_REDO,
    TG_READ_SOURCE, TG_SAVE_SOURCE,
    TG_EDIT_CUT, TG_EDIT_COPY, TG_EDIT_PASTE, TG_EDIT_DELETE,
    TG_FILE_PRINT, TG_APP_ABOUT,
    TG_ZOOM_IN, TG_ZOOM_OUT, TG_ZOOM_FULL,
    TG_ADD_CLASSDIAGRAM, TG_ADD_SEQUENCEDIAGRAM, TG_ADD_GROUP,
    TG_ADD_RELATION_DIAGRAMONLY, TG_ADD_DEPENDENCY,
    TG_ADD_NOTE, TG_ADD_LIFELINE, TG_ADD_MESSAGE,
    TG_ADD_CLASS, TG_ADD_INHERIT, TG_ADD_RELATION,
    TG_ADD_MEMBER, TG_ADD_FUNCTION, TG_ADD_CONSTRUCTOR, TG_ADD_ARGUMENT,
    TG_ADD_VIRTUALS, TG_ADD_ISCLASS,
    TG_ADD_TYPE
};

// QIcon for one toolbar tile. Null QIcon for an out-of-range index.
QIcon Qt_ToolBarIcon(int glyph);
