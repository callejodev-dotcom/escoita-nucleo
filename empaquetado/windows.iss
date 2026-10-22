; Instalador dun plugin de Escoita (ou dunha serie de plugins) para Windows (Inno Setup 6.5 ou
; posterior), en galego. Compílase desde a CI cos datos de produto.env:
;   iscc /DNome="Barullo Tune" /DCurto=BarulloTune /DAppId=3D8E1F6A-… /DAppVersion=0.3.0
;        /DArtDir=<carpeta> /DOutDir=<saída> [/DHaiClap] [/DHaiApp] windows.iss
; <carpeta> ten VST3\*.vst3, CLAP\*.clap e Standalone\<Nome>.exe: todo o que haxa instálase.

#ifndef Nome
  #error Falta /DNome
#endif
#ifndef Curto
  #error Falta /DCurto
#endif
#ifndef AppId
  #error Falta /DAppId (un GUID propio de cada produto, sen chaves)
#endif
#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef ArtDir
  #error Falta /DArtDir
#endif
#ifndef OutDir
  #define OutDir "dist"
#endif
; /DHaiClap: hai CLAP; /DHaiApp: hai aplicación independente (Standalone\<Nome>.exe)

[Setup]
AppId={{{#AppId}}
AppName={#Nome}
AppVersion={#AppVersion}
AppPublisher=Escoita
AppPublisherURL=https://www.escoita.link
DefaultDirName={autopf}\{#Nome}
DefaultGroupName={#Nome}
DisableProgramGroupPage=yes
OutputDir={#OutDir}
OutputBaseFilename={#Curto}-{#AppVersion}-windows-setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
UninstallDisplayName={#Nome}
ShowLanguageDialog=no

[Languages]
; en galego (tradución non oficial de Inno Setup, de MdL: Galician.isl)
Name: "gl"; MessagesFile: "Galician.isl"

[Types]
#ifdef HaiClap
  #ifdef HaiApp
Name: "full"; Description: "Todo (VST3, CLAP e aplicación)"
  #else
Name: "full"; Description: "Todo (VST3 e CLAP)"
  #endif
#else
  #ifdef HaiApp
Name: "full"; Description: "Todo (VST3 e aplicación)"
  #else
Name: "full"; Description: "Todo (VST3)"
  #endif
#endif
Name: "custom"; Description: "Escoller"; Flags: iscustom

[Components]
Name: "vst3"; Description: "Plugin VST3 (para o DAW)"; Types: full custom
#ifdef HaiClap
Name: "clap"; Description: "Plugin CLAP (para o DAW)"; Types: full custom
#endif
#ifdef HaiApp
Name: "app";  Description: "Aplicación independente"; Types: full custom
#endif

[Files]
; VST3: na carpeta estándar C:\Program Files\Common Files\VST3 (cada bundle coa súa carpeta)
Source: "{#ArtDir}\VST3\*"; DestDir: "{commoncf64}\VST3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
#ifdef HaiClap
Source: "{#ArtDir}\CLAP\*.clap"; DestDir: "{commoncf64}\CLAP"; Components: clap; Flags: ignoreversion
#endif
#ifdef HaiApp
Source: "{#ArtDir}\Standalone\{#Nome}.exe"; DestDir: "{app}"; Components: app; Flags: ignoreversion
#endif

#ifdef HaiApp
[Icons]
Name: "{group}\{#Nome}"; Filename: "{app}\{#Nome}.exe"; Components: app
Name: "{autodesktop}\{#Nome}"; Filename: "{app}\{#Nome}.exe"; Components: app; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Icona no escritorio"; Components: app; Flags: unchecked

[Run]
Filename: "{app}\{#Nome}.exe"; Description: "Abrir {#Nome}"; Components: app; Flags: nowait postinstall skipifsilent
#endif
