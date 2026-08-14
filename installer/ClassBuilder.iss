; ClassBuilder installer -- Inno Setup script.
; Build:  ISCC.exe ClassBuilder.iss   -> output\ClassBuilderSetup.exe
; This .iss lives in <repo>\installer, so ".." is the repo root.
; Prerequisite: the full-static x64 Release exe must be built first
;   cmake --build --preset x64-release

#define MyAppName      "ClassBuilder"
#define MyAppVersion   "2.3"
#define MyAppPublisher "Jimmy Venema"
#define MyAppExe       "ClassBuilder.exe"
#define RepoRoot       ".."
#define BuildOut       RepoRoot + "\out\build\x64\bin\Release"

[Setup]
AppId={{5CAD5932-DD9E-4332-9B7C-ABE8E6B880EA}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\{#MyAppExe}
SetupIconFile={#RepoRoot}\res\ClassBuilder.ico
OutputDir=output
OutputBaseFilename=ClassBuilderSetup
Compression=lzma2/max
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
; Broadcast SHCNE_ASSOCCHANGED after install/uninstall so Explorer picks up the
; .cbz association without a re-login.
ChangesAssociations=yes
; We intentionally clear a stale per-user (HKCU) .cbz shadow in admin mode; this
; targets the installing user's profile (the normal single-user case).
UsedUserAreasWarning=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Files]
Source: "{#BuildOut}\{#MyAppExe}";                              DestDir: "{app}";                    Flags: ignoreversion
Source: "{#RepoRoot}\res\ClassBuilder.ico";                    DestDir: "{app}";                    Flags: ignoreversion
Source: "{#RepoRoot}\res\ClassBuilderDoc.ico";                 DestDir: "{app}";                    Flags: ignoreversion
Source: "{#RepoRoot}\docs\manual\ClassBuilder_Manual.pdf";     DestDir: "{app}\doc";                Flags: ignoreversion
Source: "{#RepoRoot}\models\manual\Matrix.CBZ";                DestDir: "{app}\examples";           Flags: ignoreversion
; --- runtime the user needs to compile generated code ---
Source: "{#RepoRoot}\include\*";                               DestDir: "{app}\runtime\include";    Flags: ignoreversion recursesubdirs
Source: "{#RepoRoot}\value\*";                                 DestDir: "{app}\runtime\value";      Flags: ignoreversion recursesubdirs
Source: "{#RepoRoot}\serialize\*";                             DestDir: "{app}\runtime\serialize";  Flags: ignoreversion recursesubdirs
Source: "{#RepoRoot}\third_party\zstd\include\*";              DestDir: "{app}\runtime\zstd\include";     Flags: ignoreversion
Source: "{#RepoRoot}\third_party\zstd\lib\x64\libzstd_static.lib";    DestDir: "{app}\runtime\zstd\lib\x64";    Flags: ignoreversion
Source: "{#RepoRoot}\third_party\zstd\lib-mt\x64\libzstd_static.lib"; DestDir: "{app}\runtime\zstd\lib-mt\x64"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}";                Filename: "{app}\{#MyAppExe}";          IconFilename: "{app}\ClassBuilder.ico"
Name: "{group}\{#MyAppName} Manual (PDF)";   Filename: "{app}\doc\ClassBuilder_Manual.pdf"
Name: "{group}\Uninstall {#MyAppName}";      Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}";          Filename: "{app}\{#MyAppExe}";          IconFilename: "{app}\ClassBuilder.ico"; Tasks: desktopicon

[Registry]
; .cbz file association -> ClassBuilder (double-click opens a model)
Root: HKA; Subkey: "Software\Classes\.cbz";                                    ValueType: string; ValueName: ""; ValueData: "ClassBuilder.Model"; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\ClassBuilder.Model";                      ValueType: string; ValueName: ""; ValueData: "ClassBuilder Model";  Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\ClassBuilder.Model\DefaultIcon";          ValueType: string; ValueName: ""; ValueData: "{app}\ClassBuilderDoc.ico"
Root: HKA; Subkey: "Software\Classes\ClassBuilder.Model\shell\open\command";   ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExe}"" ""%1"""
; All-users install writes HKLM, but a stale PER-USER (HKCU) .cbz association --
; e.g. from an older ClassBuilder -- shadows it (HKCU\Software\Classes wins over
; HKLM). Clear that stale per-user shadow so the machine-wide association applies.
Root: HKCU; Subkey: "Software\Classes\.cbz";               Flags: deletekey; Check: IsAdminInstallMode
Root: HKCU; Subkey: "Software\Classes\ClassBuilder.Model"; Flags: deletekey; Check: IsAdminInstallMode

[Run]
Filename: "{app}\{#MyAppExe}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent
