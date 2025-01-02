[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{68EAE32F-C73D-4232-B8E0-09F49F0B86B9}}
AppName=Remote Desktop with Email Service
AppVersion=1.0
; Uncomment the following line to define a custom installation directory
; DefaultDirName={pf}\Remote Desktop with Email Service
DefaultDirName={localappdata}\Remote Desktop with Email Service
DefaultGroupName=Remote Desktop with Email Service
OutputDir=.
OutputBaseFilename=RemoteDesktopWithEmailServiceInstaller
Compression=lzma
SolidCompression=yes
SetupIconFile=RemoteDesktopWithEmailServiceIcon.ico 

[Files]
; Replace "build\*" with the path to your build folder
Source: "..\build\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs
Source: "check_gcc_version.bat"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Icons]
Name: "{group}\Remote Desktop Client"; Filename: "{app}\client.exe"; IconFilename: "{app}\RemoteDesktopWithEmailServiceIcon.ico"
Name: "{group}\Remote Desktop Server"; Filename: "{app}\server.exe"; IconFilename: "{app}\RemoteDesktopWithEmailServiceIcon.ico"
; Uncomment the following lines if you want to create desktop shortcuts
Name: "{commondesktop}\Remote Desktop Client"; Filename: "{app}\client.exe"; IconFilename: "{app}\RemoteDesktopWithEmailServiceIcon.ico"
Name: "{commondesktop}\Remote Desktop Server"; Filename: "{app}\server.exe"; IconFilename: "{app}\RemoteDesktopWithEmailServiceIcon.ico"

[Run]
Filename: "{app}\client.exe"; Description: "{cm:LaunchProgram,Remote Desktop Client}"; Flags: nowait postinstall skipifsilent
Filename: "{app}\server.exe"; Description: "{cm:LaunchProgram,Remote Desktop Server}"; Flags: nowait postinstall skipifsilent

