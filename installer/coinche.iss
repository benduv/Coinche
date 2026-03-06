; ============================================
; Coinche - Script Inno Setup
; ============================================
; Pour compiler : lancer deploy.bat
; Ou ouvrir ce fichier dans Inno Setup IDE

#define AppName      "Coinche"
#define AppVersion   "1.0.0"
#define AppPublisher "Benjamin Duverlie"
#define AppURL       "https://nebuludik.fr"
#define AppExeName   "coinche.exe"

[Setup]
AppId={{A3F7B2C1-4D8E-4F2A-9B1C-0E5D3F7A8B2C}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
AllowNoIcons=yes
; Icone de l'installateur (optionnel - decommenter si vous avez un .ico)
; SetupIconFile=..\resources\coinche.ico
LicenseFile=
OutputDir=output
OutputBaseFilename=CoinchSetup-{#AppVersion}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
; Necessite les droits admin pour installer dans Program Files
PrivilegesRequired=admin
; Architecture 64-bit uniquement
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[Tasks]
Name: "desktopicon"; Description: "Cr&éer une icône sur le Bureau"; GroupDescription: "Icônes supplémentaires :"; Flags: unchecked

[Files]
; Tout le contenu du dossier deploy (exe + DLLs Qt + DLLs MinGW + plugins QML)
Source: "deploy\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
; Raccourci dans le menu Démarrer
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExeName}"
; Raccourci sur le Bureau (optionnel)
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
; Lancer l'app a la fin de l'installation (optionnel)
Filename: "{app}\{#AppExeName}"; Description: "Lancer {#AppName}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Nettoyer les fichiers crees par l'app (logs, db locale) a la desinstallation
Type: filesandordirs; Name: "{commonappdata}\{#AppName}"
