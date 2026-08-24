#!/usr/bin/env bash
#
# Build the macOS ClassBuilder installer: a .dmg holding the self-contained
# ClassBuilder.app plus a drag-to-/Applications target.
#
# This is the macOS counterpart of ClassBuilder.iss (Inno Setup / Windows) and
# ships the SAME payload -- see crossplatform/INSTALLER.md for the shared table.
# Where the Windows installer writes {app}\doc, \examples and \runtime, the Mac
# equivalents go INSIDE the bundle at Contents/Resources/, so they survive the
# drag to /Applications (files merely sitting beside the app in the .dmg would
# be lost at install time).
#
#   ./installer/make-dmg.sh [path/to/ClassBuilder.app]
#
# Requires a STATIC release build first (self-contained, no Qt frameworks):
#   cmake --build --preset mac-static-release
# That preset lives in the gitignored CMakeUserPresets.json -- recreate it if
# this is a fresh machine (crossplatform/INSTALLER.md + PORTING_MAC.md).
#
# Deliberately NOT here: `rm`. Staging happens in a fresh mktemp -d each run and
# the .dmg is overwritten via hdiutil -ov, so the script never deletes anything.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP="${1:-$REPO/out/build/mac-static/bin/Release/ClassBuilder.app}"
OUTDIR="$REPO/installer/output"

if [[ ! -d "$APP" ]]; then
    echo "error: no app bundle at $APP" >&2
    echo "       build one first:  cmake --build --preset mac-static-release" >&2
    echo "       (or pass the .app path as \$1)" >&2
    exit 1
fi

# --- Verify it really is the self-contained build -------------------------
# A shared-Qt build would "work" here and then fail on someone else's Mac with
# a missing-framework dialog, so this is a hard gate, not a warning.
if otool -L "$APP/Contents/MacOS/ClassBuilder" | tail -n +2 \
        | grep -vE '/usr/lib/|/System/Library/' | grep -q .; then
    echo "error: $APP links non-system libraries -- this is not the static build." >&2
    echo "       Offending links:" >&2
    otool -L "$APP/Contents/MacOS/ClassBuilder" | tail -n +2 \
        | grep -vE '/usr/lib/|/System/Library/' >&2
    echo "       Use the mac-static preset, not mac-patched." >&2
    exit 1
fi

VERSION="$(plutil -extract CFBundleShortVersionString raw "$APP/Contents/Info.plist")"
ARCH="$(lipo -archs "$APP/Contents/MacOS/ClassBuilder")"
VOLNAME="ClassBuilder $VERSION"
DMG="$OUTDIR/ClassBuilder-$VERSION-mac-$ARCH.dmg"

echo "==> ClassBuilder $VERSION ($ARCH)"

# --- Stage: copy the app, then add the shared payload ---------------------
STAGE="$(mktemp -d "${TMPDIR:-/tmp}/cb-dmg.XXXXXX")"
echo "==> staging in $STAGE"
ditto "$APP" "$STAGE/ClassBuilder.app"

RES="$STAGE/ClassBuilder.app/Contents/Resources"
mkdir -p "$RES/doc" "$RES/examples" "$RES/runtime/zstd/include"

# The manual PDF is generated from the .docx via Word COM = Windows-only, so
# every other platform ships the COMMITTED file. Refresh it on Windows.
cp "$REPO/docs/manual/ClassBuilder_Manual.pdf" "$RES/doc/"
cp "$REPO/models/manual/Matrix.CBZ"            "$RES/examples/"

# MIT licence text. The project went public + MIT in 2026-08; the shipped
# compile-runtime (include/ value/ serialize/) is a substantial portion of the
# Software and carries no per-file notice, and the generated headers point at
# "the LICENSE file" -- so that file has to actually travel with the package.
cp "$REPO/LICENSE" "$RES/LICENSE"

# Compile-runtime: what a user needs to COMPILE the code ClassBuilder generates.
for d in include value serialize; do
    ditto "$REPO/$d" "$RES/runtime/$d"
done
cp "$REPO"/third_party/zstd/include/* "$RES/runtime/zstd/include/"

# zstd static lib: the committed third_party ones are WINDOWS .lib files and are
# useless here, so bundle the Mac .a when brew has it. Not fatal if absent --
# the user can `brew install zstd` instead; the headers above are the portable
# half and are always shipped.
ZSTD_A="$(brew --prefix zstd 2>/dev/null)/lib/libzstd.a"
if [[ -f "$ZSTD_A" ]] && [[ "$(lipo -archs "$ZSTD_A" 2>/dev/null)" == *"$ARCH"* ]]; then
    mkdir -p "$RES/runtime/zstd/lib"
    cp "$ZSTD_A" "$RES/runtime/zstd/lib/"
    echo "    bundled $ARCH libzstd.a"
else
    echo "    NOTE: no $ARCH libzstd.a found -- shipping headers only"
    echo "          (users: brew install zstd)"
fi

# --- Sign -----------------------------------------------------------------
# Ad-hoc (`-`), i.e. UNSIGNED as far as Gatekeeper is concerned: a recipient
# needs right-click -> Open, or `xattr -dr com.apple.quarantine`. Real
# distribution wants a Developer ID cert + notarytool -- see INSTALLER.md.
echo "==> ad-hoc signing"
codesign --force --deep --sign - "$STAGE/ClassBuilder.app"
codesign --verify --deep --strict "$STAGE/ClassBuilder.app"

# Drag-to-install target.
ln -s /Applications "$STAGE/Applications"

# Read Me, visible in the .dmg window. It carries the Gatekeeper instructions
# (unsigned build) and tells the user where the manual / runtime live -- they
# are inside the bundle, which Finder shows as a single file, so without this
# they are effectively invisible.
cp "$REPO/installer/dmg-readme.txt" "$STAGE/Read Me.txt"

# ...and visible in the .dmg window itself, before anyone installs anything.
cp "$REPO/LICENSE" "$STAGE/LICENSE.txt"

# --- Build the .dmg -------------------------------------------------------
mkdir -p "$OUTDIR"
echo "==> building $DMG"
hdiutil create -ov -quiet \
    -volname "$VOLNAME" -srcfolder "$STAGE" \
    -format UDZO -fs HFS+ "$DMG"

echo
echo "==> done: $DMG"
echo "    $(du -h "$DMG" | cut -f1)  |  unsigned -- recipient runs:"
echo "    xattr -dr com.apple.quarantine /Applications/ClassBuilder.app"
echo "    (right-click -> Open stopped working on macOS 15+; see Read Me.txt)"
