#!/usr/bin/env bash
#
# Build the self-contained ClassBuilder .deb INSIDE a low-glibc container, so the
# package runs on THAT glibc and every newer one. The arch follows the container
# (= the host, natively -- no emulation):
#
#   # arm64 .deb (glibc 2.35), on the Pi or an Apple-Silicon Parallels VM:
#   docker run --rm -v "$PWD":/src -w /src arm64v8/ubuntu:22.04 bash installer/build-in-container.sh
#
#   # amd64 .deb (glibc 2.35), on an x86_64 box:
#   docker run --rm -v "$PWD":/src -w /src        ubuntu:22.04 bash installer/build-in-container.sh
#
# ubuntu:22.04 = glibc 2.35 -> the .deb covers Ubuntu 22.04 / Debian 12 and newer.
# This is the LOCAL twin of .github/workflows/linux-amd64-deb.yml (which does the
# amd64 build in CI). Keep the two recipes in step.
#
# The static Qt build (~40 min) reruns each time in a --rm container. To CACHE it
# across runs, mount a named volume at the Qt prefix and it is reused if present:
#   docker run --rm -v "$PWD":/src -w /src -v cb-qt-static:/opt/Qt-6.11.1-static \
#              arm64v8/ubuntu:22.04 bash installer/build-in-container.sh
#
# Output: installer/output/classbuilder_<ver>_<arch>-glibc<floor>.deb  (in the
# mounted repo, so it lands on the host). make-deb.sh stamps the arch; this
# script appends the container's glibc to the name, matching the CI package.
set -euo pipefail

QT_VER=6.11.1
QT_PREFIX="${QT_PREFIX:-/opt/Qt-${QT_VER}-static}"
CMAKE_VER=3.31.6
JOBS="$(nproc)"
export DEBIAN_FRONTEND=noninteractive

case "$(dpkg --print-architecture)" in
    amd64) CMAKE_ARCH=x86_64  ;;
    arm64) CMAKE_ARCH=aarch64 ;;
    *) echo "error: unsupported arch $(dpkg --print-architecture)" >&2; exit 1 ;;
esac

echo "==> [1/5] build prerequisites (apt)"
apt-get update -qq
apt-get install -y --no-install-recommends \
    git curl ca-certificates xz-utils file pkg-config \
    build-essential gcc-12 g++-12 ninja-build python3 dpkg-dev binutils \
    libgl1-mesa-dev libfontconfig1-dev libfreetype-dev \
    libx11-dev libx11-xcb-dev libxext-dev libxfixes-dev libxi-dev \
    libxrender-dev libxcb1-dev libxcb-cursor-dev libxcb-glx0-dev \
    libxcb-keysyms1-dev libxcb-image0-dev libxcb-shm0-dev \
    libxcb-icccm4-dev libxcb-sync-dev libxcb-xfixes0-dev \
    libxcb-shape0-dev libxcb-randr0-dev libxcb-render-util0-dev \
    libxcb-util-dev libxcb-xinerama0-dev libxcb-xkb-dev \
    libxkbcommon-dev libxkbcommon-x11-dev libdbus-1-dev libzstd-dev >/dev/null
export CC=gcc-12 CXX=g++-12          # 22.04 ships gcc 11; 12 for Qt 6.11 (glibc stays 2.35)

echo "==> [2/5] recent CMake (${CMAKE_ARCH}) -- 22.04's 3.22 is too old for Qt 6.11"
curl -fsSL "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VER}/cmake-${CMAKE_VER}-linux-${CMAKE_ARCH}.tar.gz" \
    | tar xz -C /opt
export PATH="/opt/cmake-${CMAKE_VER}-linux-${CMAKE_ARCH}/bin:$PATH"

echo "==> [3/5] static Qt -> ${QT_PREFIX}"
if [ -f "${QT_PREFIX}/lib/libQt6Core.a" ]; then
    echo "    already present -- reusing (mounted cache)"
else
    mkdir -p /qt/src && cd /qt/src
    B="https://download.qt.io/official_releases/qt/6.11/${QT_VER}/submodules"
    for m in qtbase qtsvg; do
        [ -d "${m}-everywhere-src-${QT_VER}" ] || {
            curl -fsSLO "${B}/${m}-everywhere-src-${QT_VER}.tar.xz"
            tar xf "${m}-everywhere-src-${QT_VER}.tar.xz"
        }
    done
    # static, D-Bus ON (portal accent), xcb/xkbcommon/fontconfig forced ON so a
    # missing dev lib fails configure fast instead of building a GUI-less Qt.
    cmake -S "qtbase-everywhere-src-${QT_VER}" -B build-qtbase -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_INSTALL_PREFIX="${QT_PREFIX}" \
        -DQT_BUILD_TESTS=OFF -DQT_BUILD_EXAMPLES=OFF \
        -DFEATURE_sql=OFF -DFEATURE_dbus=ON -DFEATURE_icu=OFF \
        -DFEATURE_xcb=ON -DFEATURE_xcb_xlib=ON \
        -DFEATURE_xkbcommon=ON -DFEATURE_xkbcommon_x11=ON \
        -DFEATURE_fontconfig=ON
    cmake --build build-qtbase --parallel "${JOBS}"
    cmake --install build-qtbase
    cmake -S "qtsvg-everywhere-src-${QT_VER}" -B build-qtsvg -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_PREFIX_PATH="${QT_PREFIX}" -DCMAKE_INSTALL_PREFIX="${QT_PREFIX}"
    cmake --build build-qtsvg --parallel "${JOBS}"
    cmake --install build-qtsvg
fi

echo "==> [4/5] build ClassBuilder (static)"
cd /src
cmake --preset linux-x64 --fresh -DCMAKE_PREFIX_PATH="${QT_PREFIX}"
cmake --build --preset linux-release
BIN=out/build/linux-x64/bin/Release/ClassBuilder
if ldd "$BIN" | grep -qi 'libQt6'; then
    echo "error: shared Qt leaked into the binary -- not a static build" >&2
    ldd "$BIN" | grep -i libQt6 >&2; exit 1
fi

echo "==> [5/5] package .deb"
bash installer/make-deb.sh
VER="$(sed -n 's/.*set(CB_VERSION "\([^"]*\)").*/\1/p' CMakeLists.txt)"
ARCH="$(dpkg --print-architecture)"
GLIBC="$(getconf GNU_LIBC_VERSION | awk '{print $2}')"     # e.g. 2.35, from the container base
SRC="installer/output/classbuilder_${VER}_${ARCH}.deb"
DST="installer/output/classbuilder_${VER}_${ARCH}-glibc${GLIBC}.deb"
cp "$SRC" "$DST"

SYMFLOOR="$(objdump -T "$BIN" | grep -oE 'GLIBC_[0-9.]+' | sed 's/GLIBC_//' | sort -uV | tail -1)"
echo
echo "==> DONE: ${DST}"
echo "    arch=${ARCH}   labelled glibc=${GLIBC} (container base)   actual symbol floor=${SYMFLOOR}"
echo "    Depends: $(dpkg-deb -f "$DST" Depends)"
echo
echo "    attach to the v3.0 release with:"
echo "      gh release upload v3.0 \"${DST}\" --clobber"
