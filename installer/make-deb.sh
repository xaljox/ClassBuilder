#!/usr/bin/env bash
#
# Build the Linux ClassBuilder installer: a Debian .deb that installs the
# self-contained (static-Qt) binary plus the shipped extras and wires up the
# menu entry + the .cbz double-click association.
#
# This is the Linux counterpart of ClassBuilder.iss (Windows) and make-dmg.sh
# (macOS) and ships the SAME payload -- see crossplatform/INSTALLER.md.
#
#   ./installer/make-deb.sh [path/to/ClassBuilder]
#
# Requires a STATIC release build first (self-contained, no apt-Qt dependency):
#   cmake --build --preset linux-release        # static Qt at ~/Qt-6.11.1-static
# See crossplatform/PORTING_LINUX.md (option B) for building that Qt.
#
# ARCH: the package is built for the arch of the machine you run this on
# (dpkg --print-architecture -> amd64 / arm64). Linux gives no cross-run, so
# build amd64 on an x86_64 box and arm64 on an arm64 box. Build the arm64
# package on the OLDEST-glibc arm64 machine you target (the Pi's Debian rather
# than a newer Ubuntu) so it also runs on the newer one.
#
# No root / fakeroot needed: dpkg-deb --root-owner-group stamps root:root.
# Deliberately NOT here: `rm`. Staging is a fresh mktemp -d; the .deb is
# overwritten in place.
#
# This is a private, self-contained package (installs under /opt), not a
# Debian-archive-grade one: it has no changelog/copyright and will draw lintian
# warnings -- that is fine, it installs and uninstalls cleanly.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${1:-$REPO/out/build/linux-x64/bin/Release/ClassBuilder}"
OUTDIR="$REPO/installer/output"
LINUX="$REPO/installer/linux"

command -v dpkg-deb >/dev/null || { echo "error: dpkg-deb not found (apt install dpkg-dev)" >&2; exit 1; }

if [[ ! -x "$BIN" ]]; then
    echo "error: no executable at $BIN" >&2
    echo "       build one first:  cmake --build --preset linux-release" >&2
    echo "       (or pass the binary path as \$1)" >&2
    exit 1
fi

# --- Static-Qt gate -------------------------------------------------------
# The binary must NOT link libQt6* -- Qt comes from the static build at
# ~/Qt-6.11.1-static. System libs (xcb/GL/fontconfig/glibc) ARE linked and are
# expected: "static" on Linux only takes Qt out of the equation, never the
# desktop baseline (crossplatform/PORTING_LINUX.md). A shared-Qt build would
# package fine here and then fail on a box with a different apt Qt6.
if ldd "$BIN" | grep -qi 'libQt6'; then
    echo "error: $BIN links shared Qt6 -- this is not the static build." >&2
    ldd "$BIN" | grep -i 'libQt6' >&2
    echo "       Build the static Qt (PORTING_LINUX.md option B), use the linux preset." >&2
    exit 1
fi

VERSION="$(sed -n 's/.*set(CB_VERSION "\([^"]*\)").*/\1/p' "$REPO/CMakeLists.txt")"
[[ -n "$VERSION" ]] || { echo "error: could not read CB_VERSION from CMakeLists.txt" >&2; exit 1; }
ARCH="$(dpkg --print-architecture)"          # amd64 / arm64
PKG="classbuilder"
DEB="$OUTDIR/${PKG}_${VERSION}_${ARCH}.deb"

echo "==> ClassBuilder $VERSION ($ARCH)"

STAGE="$(mktemp -d "${TMPDIR:-/tmp}/cb-deb.XXXXXX")"
echo "==> staging in $STAGE"

# ---- payload: /opt/classbuilder/{ClassBuilder,doc,examples,runtime} ------
# Kept TOGETHER so Qt's applicationDirPath() -- resolved via /proc/self/exe,
# even through the /usr/bin symlink -- finds doc/examples/runtime beside the
# binary. That is exactly what the Help menu's cbPayloadDir() expects on
# non-Apple platforms (src/qt/QtShellWindow.cpp).
APP="$STAGE/opt/classbuilder"
mkdir -p "$APP/doc" "$APP/examples" "$APP/runtime/zstd/include"
install -m 0755 "$BIN" "$APP/ClassBuilder"

# The manual PDF is generated from the .docx via Word COM = Windows-only, so
# every non-Windows platform ships the COMMITTED file. Refresh it on Windows.
cp "$REPO/docs/manual/ClassBuilder_Manual.pdf" "$APP/doc/"
cp "$REPO/models/manual/Matrix.CBZ"            "$APP/examples/"

# Compile-runtime: what a user needs to COMPILE the code ClassBuilder generates.
for d in include value serialize; do
    mkdir -p "$APP/runtime/$d"
    cp -r "$REPO/$d/." "$APP/runtime/$d/"
done
cp "$REPO"/third_party/zstd/include/* "$APP/runtime/zstd/include/"
# The committed third_party/zstd/lib* are WINDOWS .lib files -- useless here.
# Linux users link the system zstd (apt install libzstd-dev); headers shipped.

# ---- /usr/bin launcher symlink -------------------------------------------
mkdir -p "$STAGE/usr/bin"
ln -s /opt/classbuilder/ClassBuilder "$STAGE/usr/bin/classbuilder"

# ---- desktop entry + MIME (.cbz) association + icons ---------------------
mkdir -p "$STAGE/usr/share/applications" "$STAGE/usr/share/mime/packages"
cp "$LINUX/classbuilder.desktop"  "$STAGE/usr/share/applications/"
cp "$LINUX/classbuilder-mime.xml" "$STAGE/usr/share/mime/packages/"

install_icon() {   # <src.png> <apps|mimetypes> <target-basename>
    local src="$1" ctx="$2" name="$3"
    [[ -f "$src" ]] || { echo "    NOTE: missing icon $src -- skipped"; return 0; }
    local dim; dim="$(file -b "$src" | sed -n 's/.*, \([0-9]\+\) x \([0-9]\+\),.*/\1x\2/p')"
    [[ -n "$dim" ]] || dim="256x256"
    local dir="$STAGE/usr/share/icons/hicolor/$dim/$ctx"
    mkdir -p "$dir"
    cp "$src" "$dir/$name.png"
}
install_icon "$LINUX/classbuilder.png"       apps      classbuilder
install_icon "$LINUX/classbuilder-model.png" mimetypes application-x-classbuilder-model

# ---- DEBIAN control + maintainer scripts ---------------------------------
mkdir -p "$STAGE/DEBIAN"

# Depends: derive the real shared-lib packages from the binary itself (static
# Qt still links xcb/GL/fontconfig/glibc from the system). Falls back to a
# desktop baseline if the ldd/dpkg-S derivation yields nothing.
derive_depends() {
    ldd "$BIN" 2>/dev/null \
        | awk '{for (i=1;i<=NF;i++) if ($i ~ /^\//) print $i}' \
        | sort -u | xargs -r dpkg -S 2>/dev/null \
        `# drop dpkg-divert lines ("diversion by libc6 from: /lib64/ld-linux..."` \
        `# -- the usr-merge migration diverts ld.so, and their ':' would parse` \
        `# as a bogus package name and break the Depends field` \
        | grep -v '^diversion ' \
        | cut -d: -f1 | tr ',' '\n' \
        | sed 's/^[[:space:]]*//; s/[[:space:]]*$//' \
        `# paste -d takes a LIST of delimiters used CYCLICALLY, so ', ' would` \
        `# alternate comma/space and dpkg rejects the space. Comma only -- the` \
        `# spaces after each comma are cosmetic and not required in Depends.` \
        | sort -u | grep -v '^$' | paste -sd, -
}
DEPS="$(derive_depends || true)"
[[ -n "$DEPS" ]] || DEPS="libc6, libstdc++6, libxcb1, libxcb-cursor0, libfontconfig1, libfreetype6, libgl1, libglib2.0-0, libxkbcommon0, zlib1g"

INSTALLED_KB="$(du -sk "$STAGE/opt" "$STAGE/usr" | awk '{s+=$1} END{print s}')"

cat > "$STAGE/DEBIAN/control" <<EOF
Package: $PKG
Version: $VERSION
Architecture: $ARCH
Maintainer: Jimmy Venema <jimmy.venema@gmail.com>
Section: devel
Priority: optional
Installed-Size: $INSTALLED_KB
Depends: $DEPS
Description: ClassBuilder object-model code generator
 Define an OO data model (classes, members, methods, relations, diagrams) in a
 GUI; ClassBuilder writes the C++ .h/.cpp source for that model. Self-contained
 build: Qt is linked statically, so only the standard desktop libraries are
 needed. Ships the manual (PDF), an example model and the compile-runtime under
 /opt/classbuilder (reachable from the Help menu).
EOF

cat > "$STAGE/DEBIAN/postinst" <<'EOF'
#!/bin/sh
set -e
update-mime-database /usr/share/mime >/dev/null 2>&1 || true
update-desktop-database /usr/share/applications >/dev/null 2>&1 || true
gtk-update-icon-cache -qtf /usr/share/icons/hicolor >/dev/null 2>&1 || true
exit 0
EOF

cat > "$STAGE/DEBIAN/postrm" <<'EOF'
#!/bin/sh
set -e
if [ "$1" = "remove" ] || [ "$1" = "purge" ]; then
    update-mime-database /usr/share/mime >/dev/null 2>&1 || true
    update-desktop-database /usr/share/applications >/dev/null 2>&1 || true
    gtk-update-icon-cache -qtf /usr/share/icons/hicolor >/dev/null 2>&1 || true
fi
exit 0
EOF
chmod 0755 "$STAGE/DEBIAN/postinst" "$STAGE/DEBIAN/postrm"

# ---- build ---------------------------------------------------------------
mkdir -p "$OUTDIR"
echo "==> building $DEB"
dpkg-deb --build --root-owner-group "$STAGE" "$DEB"

echo
echo "==> done: $DEB   ($(du -h "$DEB" | cut -f1))"
echo "    Depends: $DEPS"
echo "    install:   sudo apt install \"$DEB\"      (pulls Depends; or: sudo dpkg -i)"
echo "    uninstall: sudo apt remove classbuilder"
echo "    the menu entry + .cbz double-click association come up after install."
