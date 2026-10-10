; Installeur Windows de Key Detector (VST3) - compilé par Inno Setup.
#define AppName "Key Detector"
#define AppVersion "1.0.0"

[Setup]
AppId={{B7C2D6E4-3F1A-4C58-9E21-6A0D5C9F7A11}
AppName={#AppName} (VST3)
AppVersion={#AppVersion}
AppPublisher=KeyDetectorLab
CreateAppDir=no
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=Output
OutputBaseFilename=KeyDetector-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
InfoAfterFile=apres-installation.txt
UninstallDisplayName={#AppName} (VST3)

[Files]
Source: "..\build\KeyDetector_artefacts\Release\VST3\Key Detector.vst3\*"; \
  DestDir: "{commoncf64}\VST3\Key Detector.vst3"; \
  Flags: ignoreversion recursesubdirs createallsubdirs
