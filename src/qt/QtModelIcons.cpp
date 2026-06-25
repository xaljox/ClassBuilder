// qt/QtModelIcons.cpp -- the CB model tree icons, as QIcons.
//
// The table below is in exact CClassBuilderView::InitImageList order, so a
// model GetIcon() index lands on the matching .ico. The icons are embedded
// via qt/resources.qrc (prefix /icons); the .ico files are the very same
// res/*.ico the MFC build compiles into the EXE -- reused, not redrawn.

#include "QtModelIcons.h"

#include <QVector>
#include <QFile>
#include <QString>

namespace {

// InitImageList order -- index 0..66. Keep in sync with that function.
const char* const kIconFiles[] = {
    "file.ico",                       //  0  IDI_FILE
    "fileselected.ico",               //  1  IDI_FILESELECTED
    "class.ico",                      //  2  IDI_CLASS
    "inherit.ico",                    //  3  IDI_INHERIT
    "public_member.ico",              //  4  ICON_PUBLIC_MEMBER
    "protected_member.ico",           //  5  ICON_PROTECTED_MEMBER
    "private_member.ico",             //  6  ICON_PRIVATE_MEMBER
    "public_constructor.ico",         //  7  ICON_PUBLIC_CONSTRUCTOR
    "protected_constructor.ico",      //  8  ICON_PROTECTED_CONSTRUCTOR
    "private_constructor.ico",        //  9  ICON_PRIVATE_CONSTRUCTOR
    "public_destructor.ico",          // 10  ICON_PUBLIC_DESTRUCTOR
    "protected_destructor.ico",       // 11  ICON_PROTECTED_DESTRUCTOR
    "private_destructor.ico",         // 12  ICON_PRIVATE_DESTRUCTOR
    "public_method.ico",              // 13  ICON_PUBLIC_METHOD
    "protected_method.ico",           // 14  ICON_PROTECTED_METHOD
    "private_method.ico",             // 15  ICON_PRIVATE_METHOD
    "single_act.ico",                 // 16
    "single_pas.ico",                 // 17
    "owned_single_act.ico",           // 18
    "owned_single_pas.ico",           // 19
    "multi_act.ico",                  // 20
    "multi_pas.ico",                  // 21
    "owned_multi_act.ico",            // 22
    "owned_multi_pas.ico",            // 23
    "static_multi_act.ico",           // 24
    "static_multi_pas.ico",           // 25
    "static_owned_multi_act.ico",     // 26
    "static_owned_multi_pas.ico",     // 27
    "cr_single_act.ico",              // 28
    "cr_single_pas.ico",              // 29
    "cr_owned_single_act.ico",        // 30
    "cr_owned_single_pas.ico",        // 31
    "cr_multi_act.ico",               // 32
    "cr_multi_pas.ico",               // 33
    "cr_owned_multi_act.ico",         // 34
    "cr_owned_multi.ico",             // 35  (IDI_CR_OWNED_MULTI_PAS)
    "cr_static_multi_act.ico",        // 36
    "cr_static_multi_pas.ico",        // 37
    "cr_static_owned_multi_act.ico",  // 38
    "cr_static_owned_multi_pas.ico",  // 39
    "type.ico",                       // 40
    "argument.ico",                   // 41
    "methodgroup.ico",                // 42
    "membergroup.ico",                // 43
    "membermethodgroup.ico",          // 44
    "classdiagram.ico",               // 45
    "public_inline_constructor.ico",  // 46
    "protected_inline_constructor.ico",// 47
    "private_inline_constructor.ico", // 48
    "public_inline_destructor.ico",   // 49
    "protected_inline_destructor.ico",// 50
    "private_inline_destructor.ico",  // 51
    "public_inline_method.ico",       // 52
    "protected_inline_method.ico",    // 53
    "private_inline_method.ico",      // 54
    "externclass.ico",                // 55
    "public_constructor.ico",         // 56
    "protected_constructor.ico",      // 57
    "private_constructor.ico",        // 58
    "public_destructor.ico",          // 59
    "protected_destructor.ico",       // 60
    "private_destructor.ico",         // 61
    "public_empty_method.ico",        // 62
    "protected_empty_method.ico",     // 63
    "private_empty_method.ico",       // 64
    "sequencediagram.ico",            // 65
    "actor.ico",                      // 66
};
const int kIconCount = int(sizeof(kIconFiles) / sizeof(kIconFiles[0]));

// Load one model icon. A redrawn <name>.svg (qt/icons/, bundled when this Qt
// has the Svg module -- CB_HAVE_SVG) wins over the legacy <name>.ico, so the
// icon set can be modernised one glyph at a time. `icoFile` is the .ico name.
QIcon loadModelIcon(const char* icoFile)
{
#ifdef CB_HAVE_SVG
    QString svg = QString(":/icons/") + icoFile;
    svg.chop(4);            // drop ".ico"
    svg += ".svg";
    if (QFile::exists(svg))
        return QIcon(svg);
#endif
    return QIcon(QString(":/icons/") + icoFile);
}

} // namespace

QIcon Qt_ModelIcon(int index)
{
    // Built once, lazily -- needs QApplication (resource system) to be up.
    static QVector<QIcon> icons = [] {
        QVector<QIcon> v;
        v.reserve(kIconCount);
        for (int i = 0; i < kIconCount; ++i)
            v.append(loadModelIcon(kIconFiles[i]));
        return v;
    }();

    if (index < 0 || index >= icons.size())
        return QIcon();
    return icons[index];
}

QIcon Qt_PhaseIcon(int phase)
{
    // Index 1..5 (Phase enum); 0 == None_Phase -> no marker. Same .ico the MFC
    // state image list loads (res/Analysis.ico ...), via the SVG-override loader.
    static const char* const kPhaseFiles[] = {
        nullptr,                      // 0  None_Phase
        "analysis_phase.ico",         // 1  Analysis_Phase
        "design_phase.ico",           // 2  Design_Phase
        "implementation_phase.ico",   // 3  Implementation_Phase
        "test_phase.ico",             // 4  Test_Phase
        "complete_phase.ico",         // 5  Complete_Phase
    };
    static const int kPhaseCount = int(sizeof(kPhaseFiles) / sizeof(kPhaseFiles[0]));

    static QVector<QIcon> icons = [] {
        QVector<QIcon> v;
        v.reserve(kPhaseCount);
        for (int i = 0; i < kPhaseCount; ++i)
            v.append(kPhaseFiles[i] ? loadModelIcon(kPhaseFiles[i]) : QIcon());
        return v;
    }();

    if (phase < 1 || phase >= icons.size())
        return QIcon();
    return icons[phase];
}
