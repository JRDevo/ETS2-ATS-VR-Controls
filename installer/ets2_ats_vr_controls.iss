; ETS2 ATS VR Controls -- Inno Setup 6 script (the setup.exe of a release; the same pattern as E:\RemoteUSB's installer).
;
; Build: scripts\build_setup.ps1 (Release build first). It passes AppVersion (CMakeLists.txt) and StageDir (the SHIPPED
; config.ini, from git -- the root one may hold the VR menu's saved changes). Output: build\setup\ETS2-ATS-VR-Controls-Setup-v<ver>.exe
;
; Does what scripts\install.ps1 does, as an app with an "Installed apps" entry and an uninstaller:
; - ets2_ats_vr_controls.dll + XR_APILAYER_JRDEVO_ets2_ats_vr_controls.json -> {app} (default C:\Program Files\ETS2 ATS VR Controls,
;   read-only for users: the layer DLL is loaded into every OpenXR program, so no user may replace it).
; - config.ini -> C:\ProgramData\ETS2 ATS VR Controls (users may write: the VR menu saves there, the log goes to its logs\).
;   The DLL looks there when no config.ini is next to it (src/config.cpp ConfigPath). Kept on an update.
; - HKLM\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit\<{app}\json> = DWORD 0 (HKLM only: the games' own loader reads only
;   HKLM; never HKCU, the official loader then loads the layer twice).
; - HKLM\SOFTWARE\SCS Software\<game>\Plugins\ets2_ats_vr_controls = {app}\dll for ETS2 and ATS (driving).
; - Older registrations of the same json / dll file name (another folder, a zip install) are removed.
; The folder page (first install only; an update keeps the folder) may pick another {app}: every registry value points there.
; Refuses while eurotrucks2.exe / amtrucks.exe run. The uninstaller undoes all of it: the registry values (an emptied Plugins
; key goes too, and any per-user layer value), the files and {app}, the Start menu entry, every log (ProgramData\...\logs,
; {app}\logs, each user's %LOCALAPPDATA%\ETS2 ATS VR Controls\logs); it asks before it deletes config.ini (default: keep it).
; The icon: installer\ets2_ats_vr_controls.ico, made from docs\images\logo.png (16 .. 256 px).

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef StageDir
  #define StageDir "..\build\setup\stage"
#endif
#define AppName "ETS2 ATS VR Controls"
#define BuildDir "..\build\Release"
#define DllName "ets2_ats_vr_controls.dll"
#define JsonName "XR_APILAYER_JRDEVO_ets2_ats_vr_controls.json"
#define PluginValue "ets2_ats_vr_controls"
#define LayerKey "SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit"
#define DataDir "{commonappdata}\ETS2 ATS VR Controls"

[Setup]
AppId={{B3E61C52-7D0A-4F7E-9C1B-5A2E8D4F6A91}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher=JRDevo
AppPublisherURL=https://github.com/JRDevo/ETS2-ATS-VR-Controls
AppSupportURL=https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues
AppUpdatesURL=https://github.com/JRDevo/ETS2-ATS-VR-Controls/releases
VersionInfoVersion={#AppVersion}
DefaultDirName={autopf}\{#AppName}
DisableDirPage=auto
UsePreviousAppDir=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
CloseApplications=yes
RestartApplications=no
UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\ets2_ats_vr_controls.ico
SetupIconFile=ets2_ats_vr_controls.ico
LicenseFile=..\LICENSE.md
OutputDir=..\build\setup
OutputBaseFilename=ETS2-ATS-VR-Controls-Setup-v{#AppVersion}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Dirs]
Name: "{#DataDir}"; Permissions: users-modify; Flags: uninsneveruninstall

[Files]
Source: "{#BuildDir}\{#DllName}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete
Source: "{#BuildDir}\{#JsonName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\THIRD_PARTY.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "ets2_ats_vr_controls.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "settings_folder.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StageDir}\config.ini"; DestDir: "{#DataDir}"; Flags: onlyifdoesntexist uninsneveruninstall

[Icons]
Name: "{autoprograms}\{#AppName} settings and logs"; Filename: "{#DataDir}"; IconFilename: "{app}\ets2_ats_vr_controls.ico"; Comment: "config.ini and the logs folder"

[UninstallDelete]
Type: filesandordirs; Name: "{app}\logs"
Type: dirifempty; Name: "{app}"

[Registry]
Root: HKLM64; Subkey: "{#LayerKey}"; ValueType: dword; ValueName: "{app}\{#JsonName}"; ValueData: 0; Flags: uninsdeletevalue
Root: HKLM64; Subkey: "SOFTWARE\SCS Software\Euro Truck Simulator 2\Plugins"; ValueType: string; ValueName: "{#PluginValue}"; ValueData: "{app}\{#DllName}"; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKLM64; Subkey: "SOFTWARE\SCS Software\American Truck Simulator\Plugins"; ValueType: string; ValueName: "{#PluginValue}"; ValueData: "{app}\{#DllName}"; Flags: uninsdeletevalue uninsdeletekeyifempty

[Code]
const
  Games = 'Euro Truck Simulator 2|American Truck Simulator';

{ '' = no game runs, else its exe name. WMI; a failed query counts as "not running" (the file copy then still reports a DLL in use). }
function RunningGame: String;
var
  Locator, Service, Found: Variant;
  Exes: array of String;
  I: Integer;
begin
  Result := '';
  SetArrayLength(Exes, 2);
  Exes[0] := 'eurotrucks2.exe';
  Exes[1] := 'amtrucks.exe';
  try
    Locator := CreateOleObject('WbemScripting.SWbemLocator');
    Service := Locator.ConnectServer('.', 'root\CIMV2');
    for I := 0 to GetArrayLength(Exes) - 1 do
    begin
      Found := Service.ExecQuery('SELECT ProcessId FROM Win32_Process WHERE Name = ''' + Exes[I] + '''');
      if Found.Count > 0 then
      begin
        Result := Exes[I];
        Exit;
      end;
    end;
  except
    Log('process check failed: ' + GetExceptionMessage);
  end;
end;

function GameRunningMessage(const Exe: String): String;
begin
  Result := Exe + ' is running. Close the game first (the mod''s DLL is in use), then try again.';
end;

function SameName(const A, B: String): Boolean;
begin
  Result := CompareText(A, B) = 0;
end;

{ A junction / symlink (FILE_ATTRIBUTE_REPARSE_POINT). Users may modify the settings folder, so they could swap it (or its logs\)
  for a link to another folder: this admin setup then never writes, sets rights or deletes through it. }
function IsLink(const Path: String): Boolean;
var
  R: TFindRec;
begin
  Result := False;
  if FindFirst(Path, R) then
  begin
    Result := (R.Attributes and $400) <> 0;
    FindClose(R);
  end;
end;

{ Deletes Dir and what is in it; a link is only unlinked (its target stays). }
procedure SafeDelTree(const Dir: String);
begin
  if IsLink(Dir) then RemoveDir(Dir)
  else DelTree(Dir, True, True, True);
end;

{ Removes every OpenXR layer value of our json file name in Root, except Keep ('' = all). }
procedure RemoveLayerValuesIn(Root: Integer; const Key, Keep: String);
var
  Names: TArrayOfString;
  I: Integer;
begin
  if not RegGetValueNames(Root, Key, Names) then Exit;
  for I := 0 to GetArrayLength(Names) - 1 do
    if SameName(ExtractFileName(Names[I]), '{#JsonName}') and not SameName(Names[I], Keep) then
    begin
      RegDeleteValue(Root, Key, Names[I]);
      Log('removed layer registration: ' + Key + '\' + Names[I]);
    end;
end;

procedure RemoveLayerValues(Root: Integer; const Keep: String);
begin
  RemoveLayerValuesIn(Root, '{#LayerKey}', Keep);
end;

{ Per-user layer values (old versions; the official loader then loads the layer twice) of every signed-in user (HKEY_USERS). }
procedure RemoveAllUsersLayerValues;
var
  Sids: TArrayOfString;
  I: Integer;
begin
  if not RegGetSubkeyNames(HKU, '', Sids) then Exit;
  for I := 0 to GetArrayLength(Sids) - 1 do
    RemoveLayerValuesIn(HKU, Sids[I] + '\{#LayerKey}', '');
end;

{ Removes every game plugin value that points at a file of our DLL's name, except KeepName = KeepData ('' = all). }
procedure RemovePluginValues(const KeepData: String);
var
  Rest, Game, Key, Data: String;
  Names: TArrayOfString;
  I, P: Integer;
begin
  Rest := Games;
  while Rest <> '' do
  begin
    P := Pos('|', Rest);
    if P = 0 then begin Game := Rest; Rest := ''; end
    else begin Game := Copy(Rest, 1, P - 1); Rest := Copy(Rest, P + 1, MaxInt); end;
    Key := 'SOFTWARE\SCS Software\' + Game + '\Plugins';
    if RegGetValueNames(HKLM64, Key, Names) then
      for I := 0 to GetArrayLength(Names) - 1 do
        if RegQueryStringValue(HKLM64, Key, Names[I], Data) and SameName(ExtractFileName(Data), '{#DllName}') and
           not ((KeepData <> '') and SameName(Names[I], '{#PluginValue}') and SameName(Data, KeepData)) then
        begin
          RegDeleteValue(HKLM64, Key, Names[I]);
          Log('removed plugin entry (' + Game + '): ' + Names[I] + ' = ' + Data);
        end;
    if KeepData = '' then
      RegDeleteKeyIfEmpty(HKLM64, Key);
  end;
end;

{ A per-user layer entry (old versions) makes the official loader load the layer twice: removed as the user who started setup. }
procedure RemovePerUserLayer;
var
  Cmd: String;
  Code: Integer;
begin
  Cmd := '-NoProfile -ExecutionPolicy Bypass -Command "$k = ''HKCU:\{#LayerKey}''; if (Test-Path $k) { ' +
         '(Get-Item $k).GetValueNames() | Where-Object { [IO.Path]::GetFileName($_) -ieq ''{#JsonName}'' } | ' +
         'ForEach-Object { Remove-ItemProperty -Path $k -Name $_ } }"';
  if not ExecAsOriginalUser(ExpandConstant('{sys}\WindowsPowerShell\v1.0\powershell.exe'), Cmd, '', SW_HIDE,
                            ewWaitUntilTerminated, Code) then
    Log('per-user cleanup did not run: ' + SysErrorMessage(Code));
end;

function InitializeSetup: Boolean;
var
  Exe: String;
begin
  Exe := RunningGame;
  Result := Exe = '';
  if not Result then
    SuppressibleMsgBox(GameRunningMessage(Exe), mbError, MB_OK, IDOK);
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  Exe: String;
begin
  Result := '';
  Exe := RunningGame;                                  { (the game may have been started while the wizard was open) }
  if Exe <> '' then
    Result := GameRunningMessage(Exe)
  else if IsLink(ExpandConstant('{#DataDir}')) then
    Result := ExpandConstant('{#DataDir}') + ' is a link to another folder, not a folder. Remove it, then try again.';
end;

{ An install from the zip / install.ps1 had its config.ini next to its DLL. The setup moves that registration to its own folder, so it
  brings the settings along: the config.ini next to a registered copy (layer json, else game plugin; the setup's own folder
  too: a zip unpacked there before -- the DLL then reads only the settings folder). '' = none. }
function OldConfig: String;
var
  Names: TArrayOfString;
  I: Integer;
  Dir, Data: String;
begin
  Result := '';
  if RegGetValueNames(HKLM64, '{#LayerKey}', Names) then
    for I := 0 to GetArrayLength(Names) - 1 do
      if SameName(ExtractFileName(Names[I]), '{#JsonName}') then
      begin
        Dir := ExtractFileDir(Names[I]);
        if FileExists(Dir + '\config.ini') then
        begin
          Result := Dir + '\config.ini';
          Exit;
        end;
      end;
  SetArrayLength(Names, 2);
  Names[0] := 'American Truck Simulator';
  Names[1] := 'Euro Truck Simulator 2';
  for I := 0 to 1 do
    if RegQueryStringValue(HKLM64, 'SOFTWARE\SCS Software\' + Names[I] + '\Plugins', '{#PluginValue}', Data) then
    begin
      Dir := ExtractFileDir(Data);
      if FileExists(Dir + '\config.ini') then
      begin
        Result := Dir + '\config.ini';
        Exit;
      end;
    end;
end;

var
  KeepSettingsFrom: String;                            { ssInstall -> ssPostInstall: an older install's config.ini to bring along }

procedure CurStepChanged(CurStep: TSetupStep);
var
  Dst: String;
  Text: AnsiString;
begin
  if CurStep = ssInstall then
  begin
    KeepSettingsFrom := '';
    if not FileExists(ExpandConstant('{#DataDir}\config.ini')) then   { (a setup's own config.ini always wins) }
      KeepSettingsFrom := OldConfig;
  end;
  if CurStep = ssPostInstall then
  begin
    Dst := ExpandConstant('{#DataDir}\config.ini');
    if (KeepSettingsFrom <> '') and not IsLink(ExpandConstant('{#DataDir}')) and LoadStringFromFile(KeepSettingsFrom, Text) then
    begin                                              { (written into the file the setup just made: it keeps that file's rights) }
      if SaveStringToFile(Dst, Text, False) then Log('settings brought over from ' + KeepSettingsFrom)
      else Log('could not bring the settings over from ' + KeepSettingsFrom);
    end;
    RemoveLayerValues(HKLM64, ExpandConstant('{app}\{#JsonName}'));
    RemoveAllUsersLayerValues;
    RemovePerUserLayer;
    RemovePluginValues(ExpandConstant('{app}\{#DllName}'));
  end;
end;

{ ---- uninstall ---- }

{ The log's last fallback (no writable folder next to the DLL or in ProgramData): %LOCALAPPDATA%\ETS2 ATS VR Controls\logs of
  whichever user played. The uninstaller runs as admin, so every profile in ProfileList. }
procedure RemoveUserLogs;
var
  Keys: TArrayOfString;
  I: Integer;
  Base, Profile, Dir: String;
begin
  Base := 'SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList';
  if not RegGetSubkeyNames(HKLM64, Base, Keys) then Exit;
  for I := 0 to GetArrayLength(Keys) - 1 do
    if RegQueryStringValue(HKLM64, Base + '\' + Keys[I], 'ProfileImagePath', Profile) then
    begin
      StringChangeEx(Profile, '%SystemDrive%', GetEnv('SystemDrive'), True);
      Dir := Profile + '\AppData\Local\ETS2 ATS VR Controls';
      if DirExists(Dir) and not IsLink(Dir) then
      begin
        SafeDelTree(Dir + '\logs');
        if RemoveDir(Dir) then Log('removed ' + Dir);
      end;
    end;
end;

function InitializeUninstall: Boolean;
var
  Exe: String;
begin
  Exe := RunningGame;
  Result := Exe = '';
  if not Result then
    SuppressibleMsgBox(GameRunningMessage(Exe), mbError, MB_OK, IDOK);
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Dir: String;
begin
  if CurUninstallStep = usUninstall then
  begin
    RemoveLayerValues(HKLM64, '');
    RemoveAllUsersLayerValues;
    RemovePluginValues('');
  end;
  if CurUninstallStep = usPostUninstall then
  begin
    RemoveUserLogs;
    Dir := ExpandConstant('{#DataDir}');
    if IsLink(Dir) then
    begin                                              { (a user swapped the folder for a link: unlink it, delete nothing in it) }
      RemoveDir(Dir);
      Exit;
    end;
    SafeDelTree(Dir + '\logs');
    DeleteFile(Dir + '\config.ini.tmp');
    if FileExists(Dir + '\config.ini') and not UninstallSilent then
      if MsgBox('Also delete your settings (config.ini)?' + #13#10#13#10 + Dir + '\config.ini' + #13#10#13#10 +
                'No = keep it: a later install then has the same buttons and settings again.',
                mbConfirmation, MB_YESNO or MB_DEFBUTTON2) = IDYES then
        DeleteFile(Dir + '\config.ini');
    RemoveDir(Dir);                                    { (only when empty: a kept config.ini keeps the folder) }
  end;
end;
