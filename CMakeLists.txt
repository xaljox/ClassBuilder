# CMakeLists.txt -- CMake build for ClassBuilder (single all-Qt executable).
#
# Layout (restructured 2026-06-24 from the old add-in DLL/EXE-split folders):
#   include/      generic CB_* container / tree / critical runtime (no deps)
#   value/        standalone value types: CbColor / CbGeometry / CbString / CbTime
#   serialize/    CbArchive (CbSerialize) + CbZstdStream -- the OPTIONAL ser. layer
#   src/model/    MFC-free model + logic + generated classes + ClassBuilder.CBZ
#   src/qt/       Qt views / dialogs / canvas (quarantined; Qt forces UNICODE)
#   src/platform/ CbPlatformCompat (the single #ifdef seam) + CbMessageBox / CbShellHooks
#   third_party/  zstd (CBZ compression) , json (command-server JSON)
#   res/ models/ tools/ docs/
#
# CRT runtime is shared (/MD); MFC itself is GONE (replaced by the Qt shell).
# The full static build (/MT + static Qt) is a later step.
cmake_minimum_required(VERSION 3.25)   # 3.25+ for CMAKE_MSVC_DEBUG_INFORMATION_FORMAT (CMP0141)

project(ClassBuilder LANGUAGES CXX)

# WIN32_LEAN_AND_MEAN gates out winspool.h (its DocumentProperties A/W macro
# otherwise clobbers Qt's QIcon::ThemeIcon::DocumentProperties); NOMINMAX keeps
# min/max macros out of std::. WIN32-only -> the Mac/Linux port is unaffected.
if(WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

# Debug uses /Z7 (debug info embedded per .obj), NOT /Zi -- /Zi funnels every
# parallel cl.exe through one shared vc145.pdb via mspdbsrv.exe, which races
# under the ~300-file parallel build (C1090). Release expands empty -> no debug
# info on Release, which is intended.
if(NOT DEFINED CB_DEBUG_FORMAT)
    set(CB_DEBUG_FORMAT "Embedded")
endif()
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:${CB_DEBUG_FORMAT}>")

# Shared DLL CRT (/MD, /MDd). The full static build will flip this to /MT.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# The model sources are a MultiByte (non-Unicode) build: deliberately do NOT
# define UNICODE/_UNICODE (the Qt code is quarantined in ClassBuilderQt).

# zstd (CBZ compression). The C API + headers are portable; the static archive
# differs per platform. The vendored zstd.h header (1.5.7) is used on every
# platform; only the static lib is resolved per-OS.
set(ZSTD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/zstd")
if(WIN32)
    # Per-platform libzstd lives under third_party/zstd/lib/<Win32|x64|ARM64>.
    # Resolve the arch token generator-agnostically (VS sets CMAKE_VS_PLATFORM_NAME;
    # Ninja does not, so fall back to the MSVC compiler architecture id).
    if(CMAKE_VS_PLATFORM_NAME)
        set(CB_ARCH "${CMAKE_VS_PLATFORM_NAME}")
    elseif(CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "x64")
        set(CB_ARCH "x64")
    elseif(CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "X86")
        set(CB_ARCH "Win32")
    elseif(CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "ARM64")
        set(CB_ARCH "ARM64")
    else()
        message(FATAL_ERROR
            "Cannot determine target arch for the zstd lib. "
            "CMAKE_CXX_COMPILER_ARCHITECTURE_ID='${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}'. "
            "This build needs the MSVC toolchain.")
    endif()
    set(ZSTD_LIB "${ZSTD_DIR}/lib/${CB_ARCH}/libzstd_static.lib")
else()
    # macOS / Linux: link the STATIC archive (libzstd.a) to match the all-static
    # ship goal. Force the .a suffix so find_library skips the .dylib/.so.
    set(_cb_saved_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    find_library(ZSTD_LIB NAMES zstd
        HINTS /opt/homebrew/opt/zstd/lib /usr/local/opt/zstd/lib
              /opt/homebrew/lib /usr/local/lib /usr/lib
              "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")  # Debian/Ubuntu multiarch (x86_64-linux-gnu, aarch64-linux-gnu)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${_cb_saved_suffixes})
    if(NOT ZSTD_LIB)
        message(FATAL_ERROR
            "Static libzstd.a not found. Install it (Linux: 'apt install libzstd-dev', "
            "macOS: 'brew install zstd') or build third_party/zstd from source.")
    endif()
    message(STATUS "zstd: static archive ${ZSTD_LIB}")
endif()

# Shared include set for both targets. Bare model includes ("ExternClasses.h")
# resolve via src/model; the codegen-baked "qt/QtXxx.h" resolves via src; the
# value / serialize / include tiers are the layered headers; json is the
# command server's JSON lib.
set(CB_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/src"             # so "qt/QtXxx.h" -> src/qt/QtXxx.h
    "${CMAKE_CURRENT_SOURCE_DIR}/src/model"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/value"
    "${CMAKE_CURRENT_SOURCE_DIR}/serialize"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/json"
    "${ZSTD_DIR}/include"
)

# --- Qt : the ported dialogs (quarantined static lib) ---------------------
# Qt 6's imported targets force UNICODE/_UNICODE onto every consumer. This is a
# deliberate MultiByte MFC-free build, so the Qt code is quarantined in its own
# static library: Qt6::Widgets is linked PRIVATE, keeping Qt's UNICODE define
# inside this lib while still propagating the Qt import libs to the EXE link.
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)  # Network = QTcpServer (command transport)
find_package(Qt6 QUIET OPTIONAL_COMPONENTS Svg)        # Svg = redrawn <name>.svg model icons

# Static Qt (C:/Qt-static) links the Qt libs + plugins into the EXE -> no
# Qt6*.dll beside it and no windeployqt. Qt6::Core's TYPE tells static/shared
# apart generator-agnostically.
get_target_property(_cb_qt_core_type Qt6::Core TYPE)
if(_cb_qt_core_type STREQUAL "STATIC_LIBRARY")
    set(CB_QT_STATIC TRUE)
    message(STATUS "Qt: static build -- plugins linked in, no windeployqt")
else()
    set(CB_QT_STATIC FALSE)
    message(STATUS "Qt: shared build -- windeployqt deploys the runtime DLLs")
endif()

# All of src/qt (flat): .cpp/.h (AUTOMOC), .ui (AUTOUIC), .qrc (AUTORCC). The
# SVG icons under src/qt/icons are bundled separately below.
file(GLOB CB_QT_SOURCES CONFIGURE_DEPENDS
     "src/qt/*.cpp" "src/qt/*.h" "src/qt/*.ui" "src/qt/*.qrc")
add_library(ClassBuilderQt STATIC ${CB_QT_SOURCES})
set_target_properties(ClassBuilderQt PROPERTIES AUTOMOC ON AUTORCC ON AUTOUIC ON)
target_compile_features(ClassBuilderQt PRIVATE cxx_std_17)
# Windows: link Qt PRIVATE so its forced UNICODE define stays inside this lib
# (the MultiByte quarantine). macOS/Linux: Qt does NOT force UNICODE, and the
# platform seam (CbPlatformCompat.cpp, compiled into the EXE) needs Qt's wait
# cursor -- so link PUBLIC there to propagate Qt's include dirs/libs to the EXE.
if(WIN32)
    target_link_libraries(ClassBuilderQt PRIVATE Qt6::Widgets Qt6::Network)
else()
    target_link_libraries(ClassBuilderQt PUBLIC Qt6::Widgets Qt6::Network)
endif()
target_include_directories(ClassBuilderQt PRIVATE ${CB_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/qt")   # AUTOUIC ui_*.h include bare "CodeEditor.h"

# Static Qt: the platform plugin + native style are linked in and registered
# via Q_IMPORT_PLUGIN (qt/QtApp.cpp). PRIVATE keeps Qt's UNICODE define off the
# MFC-free sources; the plugin .libs still propagate to the EXE. The plugin set
# is platform-specific (qwindows on Windows, qcocoa on macOS).
if(CB_QT_STATIC)
    if(WIN32)
        target_link_libraries(ClassBuilderQt PRIVATE
            Qt6::QWindowsIntegrationPlugin
            Qt6::QModernWindowsStylePlugin)
    elseif(APPLE)
        target_link_libraries(ClassBuilderQt PRIVATE
            Qt6::QCocoaIntegrationPlugin)
    elseif(UNIX AND NOT APPLE)
        # Linux static Qt: the xcb (X11) platform plugin must be linked AND imported
        # (Q_IMPORT_PLUGIN in qt/QtApp.cpp, alongside qwindows/qcocoa). Only relevant
        # once a STATIC Qt is built for Linux; the initial bring-up uses the distro's
        # SHARED Qt (CB_QT_STATIC=FALSE), which discovers qxcb at runtime -- nothing
        # to import. Add Qt6::QWaylandIntegrationPlugin here too if/when going static.
        target_link_libraries(ClassBuilderQt PRIVATE
            Qt6::QXcbIntegrationPlugin)
    endif()
    target_compile_definitions(ClassBuilderQt PRIVATE CB_STATIC_QT)
endif()

# SVG model icons (optional). When this Qt has Svg, QtModelIcons prefers a
# redrawn src/qt/icons/<name>.svg over the legacy res/<name>.ico.
if(Qt6Svg_FOUND)
    target_link_libraries(ClassBuilderQt PRIVATE Qt6::Svg)
    target_compile_definitions(ClassBuilderQt PRIVATE CB_HAVE_SVG)
    if(CB_QT_STATIC)
        target_link_libraries(ClassBuilderQt PRIVATE Qt6::QSvgIconPlugin Qt6::QSvgPlugin)
    endif()
    file(GLOB CB_SVG_ICONS CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_SOURCE_DIR}/src/qt/icons/*.svg")
    if(CB_SVG_ICONS)
        qt_add_resources(ClassBuilderQt "cb_svg_icons"
            PREFIX "/icons"
            BASE   "${CMAKE_CURRENT_SOURCE_DIR}/src/qt/icons"
            FILES  ${CB_SVG_ICONS})
    endif()
    message(STATUS "Qt: Svg module found -- SVG model icons enabled")
else()
    message(STATUS "Qt: no Svg module -- model icons use the legacy .ico set")
endif()

# --- ClassBuilder.exe : the all-Qt executable -----------------------------
# Model sources (src/model, incl. the tiny WinMain -> Cb_RunQtShell and the
# flex/bison Read.l.cpp/Read.y.cpp) + the serialization runtime (serialize) +
# the platform seam impl (src/platform). value/ is headers only.
file(GLOB CB_SOURCES CONFIGURE_DEPENDS
     "src/model/*.cpp"
     "serialize/*.cpp"
     "src/platform/*.cpp")

# The Win32 .rc (app + .cbz document icons) is MSVC-only; macOS uses a bundle.
if(WIN32)
    add_executable(ClassBuilder
        ${CB_SOURCES}
        res/ClassBuilder_app.rc)
    set_target_properties(ClassBuilder PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    # App icon: bundle the .icns into Contents/Resources and name it in the
    # Info.plist (CFBundleIconFile / CFBundleTypeIconFile). It also becomes the
    # .cbz document icon (the document type references the same file).
    set(CB_MAC_ICON "${CMAKE_CURRENT_SOURCE_DIR}/res/ClassBuilder.icns")
    set_source_files_properties(${CB_MAC_ICON} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
    add_executable(ClassBuilder ${CB_SOURCES} ${CB_MAC_ICON})
    set_target_properties(ClassBuilder PROPERTIES
        MACOSX_BUNDLE TRUE
        # Custom Info.plist: declares CB owns the .cbz model file
        # (CFBundleDocumentTypes), so a Finder double-click / "Open With" routes
        # the file to the app -> QFileOpenEvent in QtApp.cpp.
        MACOSX_BUNDLE_INFO_PLIST   "${CMAKE_CURRENT_SOURCE_DIR}/res/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME         "ClassBuilder"
        MACOSX_BUNDLE_GUI_IDENTIFIER      "com.xaljox.ClassBuilder"
        MACOSX_BUNDLE_ICON_FILE           "ClassBuilder"
        MACOSX_BUNDLE_BUNDLE_VERSION      "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
else()
    # Linux: a plain ELF executable -- no app bundle, no Win32 .rc resource.
    # The .cbz file association (icon + double-click routing) is a freedesktop
    # .desktop + shared-MIME-info step done at packaging time, not in the build.
    add_executable(ClassBuilder ${CB_SOURCES})
endif()
target_include_directories(ClassBuilder PRIVATE ${CB_INCLUDE_DIRS})

if(WIN32)
    target_compile_definitions(ClassBuilder PRIVATE
        WIN32 _WINDOWS
        _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
    )
    # Qt6's interface compile-definitions (UNICODE/_UNICODE) leak onto this
    # target through the ClassBuilderQt link interface. The MFC-free model is a
    # MultiByte build calling the ANSI Win32 APIs directly, so strip UNICODE back
    # off (cl.exe gives /U precedence over /D for the same symbol regardless of
    # order). Windows-only: Qt on macOS/Linux does not force UNICODE.
    target_compile_options(ClassBuilder PRIVATE /UUNICODE /U_UNICODE)
endif()

target_link_libraries(ClassBuilder PRIVATE "${ZSTD_LIB}" ClassBuilderQt)

# Precompiled header: StdAfx.h (windows.h + the model master include),
# force-included into every source. The flex-generated lexer opts out.
target_precompile_headers(ClassBuilder PRIVATE src/model/StdAfx.h)
set_source_files_properties(
    src/model/Read.l.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

# We supply WinMain ourselves (src/model/WinMain.cpp -> Cb_RunQtShell), so opt
# out of Qt6::Core's auto-linked Qt6EntryPoint WinMain.
set_target_properties(ClassBuilder PROPERTIES qt_no_entrypoint TRUE)

# /ignore:4098 -- harmless "defaultlib conflict" from libzstd's CRT marker.
# /SAFESEH:NO -- libzstd_static.lib (x86) has no safe-SEH table; x86 only.
# /LTCG (Release) -- libzstd's Release objects are /GL; pass it up front so the
# linker does not detect them and restart the whole link with /LTCG.
# MSVC-only linker switches.
if(MSVC)
    target_link_options(ClassBuilder PRIVATE
        /ignore:4098
        $<$<STREQUAL:${CB_ARCH},Win32>:/SAFESEH:NO>
        $<$<CONFIG:Release>:/LTCG>
    )
endif()

# windeployqt copies the Qt runtime DLLs next to the EXE for a shared Qt on
# Windows. Skipped for a static Qt (everything is inside the EXE). On macOS the
# app runs against brew Qt via rpath from the build tree; macdeployqt is only
# needed when shipping a relocatable .app (a later packaging step).
if(WIN32 AND NOT CB_QT_STATIC)
    add_custom_command(TARGET ClassBuilder POST_BUILD
        COMMAND Qt6::windeployqt --no-translations --no-system-d3d-compiler
                "$<TARGET_FILE:ClassBuilder>"
        COMMENT "Deploying the Qt runtime next to ClassBuilder.exe"
        VERBATIM)
endif()

set_target_properties(ClassBuilder PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
