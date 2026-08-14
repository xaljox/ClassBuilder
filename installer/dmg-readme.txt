ClassBuilder 3.0 - macOS (Apple Silicon / arm64)
================================================


INSTALL
-------
Drag ClassBuilder.app onto the Applications folder in this window.


FIRST LAUNCH - PLEASE READ
--------------------------
ClassBuilder is not signed with an Apple Developer certificate, so macOS
blocks it the first time. You may see:

    "ClassBuilder is damaged and can't be opened"
    "cannot be opened because the developer cannot be verified"

The download is NOT damaged. This is macOS' Gatekeeper policy for unsigned
applications. To allow it, run this once in Terminal after installing:

    xattr -dr com.apple.quarantine /Applications/ClassBuilder.app

Or, without Terminal: try to open the app once, let it be blocked, then go to
System Settings > Privacy & Security and click "Open Anyway".

Note: right-click > Open no longer works for this on macOS 15 (Sequoia) and
later - Apple removed that shortcut. Use one of the two routes above.


WHAT IS INCLUDED
----------------
The manual, an example model, and the compile-runtime are installed INSIDE the
application bundle, so they always travel with the app:

    /Applications/ClassBuilder.app/Contents/Resources/
        doc/ClassBuilder_Manual.pdf   the full manual (PDF)
        examples/Matrix.CBZ           example model
        runtime/include/              CB_*.h runtime headers
        runtime/value/                CbColor, CbGeometry, CbString, CbTime
        runtime/serialize/            CbArchive / CbZstdStream
        runtime/zstd/                 zstd headers + libzstd.a

The "runtime" is what you need to COMPILE the code that ClassBuilder generates
for you.

You do not have to go hunting for these. Use the Help menu inside ClassBuilder:

    Help > Open Manual           opens the PDF above
    Help > Show Runtime Files    opens the runtime folder in Finder
    Help > Show Example Model    opens the examples folder in Finder

If you prefer to browse there yourself, note that a .app is shown as a single
file by Finder - you must right-click ClassBuilder.app > "Show Package
Contents" first. From Terminal it is simply:

    open /Applications/ClassBuilder.app/Contents/Resources


OPENING MODELS
--------------
ClassBuilder models are .cbz files. The installer registers them, so a
double-click on a .cbz opens it in ClassBuilder. Start with the included
examples/Matrix.CBZ if you are new to the tool.


ABOUT THIS BUILD
----------------
Version          3.0
Platform         macOS, Apple Silicon (arm64) only - there is no Intel build
Self-contained   Qt and zstd are linked in; nothing else to install
Signing          ad-hoc (unsigned) - see "FIRST LAUNCH" above
