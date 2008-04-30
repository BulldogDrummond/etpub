

XPStyle on
Var REINSTALL

Function un.onInit
	SetShellVarContext current
	ReadRegStr $R0 HKLM "SOFTWARE\etpub" "InstallPath"
	IfErrors err
		StrCpy $INSTDIR $R0
		Goto done
	err:
		MessageBox MB_OK "Could not find etpub installation to uninstall!"
		Abort
	done:
FunctionEnd

Function .onInit
	SetShellVarContext current
	ReadRegStr $R0 HKLM "SOFTWARE\etpub" "InstallPath"
	IfErrors etdefault
		StrCpy $INSTDIR $R0
		Goto dllcheck
	etdefault:
		ReadRegStr $R0 HKLM "SOFTWARE\Activision\Wolfenstein - Enemy Territory" "InstallPath"
		IfErrors dllcheck
			StrCpy $INSTDIR $R0

	dllcheck:
	IfFileExists "$EXEDIR\qagame_mp_x86.dll" done
		MessageBox MB_OK "Could not find qagame_mp_x86.dll, quitting."
		Abort
	done:
FunctionEnd

Function GetParent
 
   Exch $R0
   Push $R1
   Push $R2
   Push $R3
   
   StrCpy $R1 0
   StrLen $R2 $R0
   
   loop:
     IntOp $R1 $R1 + 1
     IntCmp $R1 $R2 get 0 get
     StrCpy $R3 $R0 1 -$R1
     StrCmp $R3 "\" get
     Goto loop
   
   get:
     StrCpy $R0 $R0 -$R1
     
     Pop $R3
     Pop $R2
     Pop $R1
     Exch $R0
     
FunctionEnd


!include "MUI.nsh"

!define MUI_ABORTWARNING

!define MUI_PAGE_CUSTOMFUNCTION_LEAVE CheckReinstall
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION "Select your Enemy Territory Directory"
!define MUI_DIRECTORYPAGE_TEXT_TOP "Please select your Enemy Territory directory.  This is the directory on your system that contains the file 'ET.exe'.  This installer will install all files into a directory named 'etpub' inside of the directroy you select."
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"


; The name of the installer
Name "etpub"


; The file to write
OutFile "setup.exe"

; The default installation directory
InstallDir ""

Section "etpub qagame DLL" qagame
  SectionIn RO
  CreateDirectory "$INSTDIR\etpub"
  SetOutPath "$INSTDIR"
  CopyFiles "$EXEDIR\qagame_mp_x86.dll" "$INSTDIR\etpub\" 1500
  WriteRegStr HKLM "Software\etpub" "InstallPath" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\etpub" "DisplayName" "etpub (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\etpub" "UninstallString" '"$INSTDIR\etpub\uninstall.exe"'
  CreateShortCut "$INSTDIR\etpub.lnk" "$INSTDIR\ET.exe" "+set dedicated 2 +set fs_game etpub +exec server.cfg"
  WriteUninstaller "$INSTDIR\etpub\uninstall.exe"
SectionEnd

Section "Example Configuration" cfg
  Push $EXEDIR
  Call GetParent
  Pop $R0
  CopyFiles "$R0\docs\Example\config\default.cfg" "$INSTDIR\etpub\" 16
  CopyFiles "$R0\docs\Example\config\server.cfg" "$INSTDIR\etpub\" 16
  CopyFiles "$R0\docs\Example\config\shrubbot.cfg" "$INSTDIR\etpub\" 16
  CopyFiles "$R0\docs\Example\config\settings.cfg" "$INSTDIR\etpub\" 16
  CopyFiles "$R0\docs\Example\config\mapvotecycle.cfg" "$INSTDIR\etpub\" 16
SectionEnd

Section "Start Menu Shortcuts" startmenu
  CreateDirectory "$SMPROGRAMS\etpub\"
  CreateShortCut "$SMPROGRAMS\etpub\Start etpub.lnk" "$INSTDIR\etpub.lnk" "" "etpub" 0
  CreateShortCut "$SMPROGRAMS\etpub\Edit server.cfg.lnk" "notepad.exe" "$INSTDIR\etpub\server.cfg" "" 0 SW_SHOWNORMAL "" "Edit server.cfg"
  CreateShortCut "$SMPROGRAMS\etpub\Edit default.cfg.lnk" "notepad.exe" "$INSTDIR\etpub\default.cfg" ""  0 SW_SHOWNORMAL "" "Edit default.cfg"
  CreateShortCut "$SMPROGRAMS\etpub\Edit shrubbot.cfg.lnk" "notepad.exe" "$INSTDIR\etpub\shrubbot.cfg" "" 0 SW_SHOWNORMAL "" "Edit shrubbot.cfg"
  CreateShortCut "$SMPROGRAMS\etpub\Edit settings.cfg.lnk" "notepad.exe" "$INSTDIR\etpub\settings.cfg" "" 0 SW_SHOWNORMAL "" "Edit settings.cfg"
  CreateShortCut "$SMPROGRAMS\etpub\Uninstall.lnk" "$INSTDIR\etpub\uninstall.exe" "" "" 0
SectionEnd

Section "Desktop Icon" desktop
  CreateShortCut "$DESKTOP\etpub.lnk" "$INSTDIR\etpub.lnk" "" "$INSTDIR\etpub.lnk"
SectionEnd

UninstallText "You are about to uninstall etpub from your computer.  If you do this, all files in your etpub directory WILL BE DELETED."

Section "Uninstall"
  ; remove registry keys

  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\etpub"
  DeleteRegKey HKLM "SOFTWARE\etpub"

  ; remove shortcuts, if any
  Delete "$SMPROGRAMS\etpub\*.*"
  Delete "$DESKTOP\etpub.lnk"
  Delete "$INSTDIR\etpub.lnk"

  ; remove directories used
  RMDir /r "$SMPROGRAMS\etpub"
  RMDir /r "$INSTDIR\etpub"
  RMDir "$SMPROGRAMS\etpub"
SectionEnd


LangString DESC_qagame ${LANG_ENGLISH} "Copies qagame DLL to the etpub directory inside your ET directory"
LangString DESC_cfg ${LANG_ENGLISH} "Installs example server.cfg, default.cfg, settings.cfg and shrubbot.cfg files into your etpub directory with the default settings."
LangString DESC_startmenu ${LANG_ENGLISH} "Installs links into your Programs menu."
LangString DESC_desktop ${LANG_ENGLISH} "Creates a etpub icon on your desktop."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${qagame} $(DESC_qagame)
  !insertmacro MUI_DESCRIPTION_TEXT ${cfg} $(DESC_cfg)
  !insertmacro MUI_DESCRIPTION_TEXT ${startmenu} $(DESC_startmenu)
  !insertmacro MUI_DESCRIPTION_TEXT ${desktop} $(DESC_desktop)
!insertmacro MUI_FUNCTION_DESCRIPTION_END


Function CheckReinstall
	IfFileExists "$INSTDIR\ET.exe" EndIf
		MessageBox MB_OK "$INSTDIR does not appear to contain a valid Enemy Territory installation."
		Abort
	EndIf:

	IfFileExists "$INSTDIR\etpub\*.*" reinst
		GoTo fresh
	reinst:
		MessageBox MB_YESNO "etpub is already installed in this directory.  Do you want to install over it?" IDYES doreinst
		Abort
	doreinst:
		StrCpy $REINSTALL "1"
		MessageBox MB_OK "Since you are re-installing, your etpub .cfg files will NOT be overwritten by default.$\r$\nHowever, IF you choose to install the 'Example Configuration' component,$\r$\nyour existing .cfg files WILL BE OVERWRITTEN." 
		SectionSetFlags ${cfg} 0
		Goto done 
	fresh:
		; required if not reinstalling
		;SectionSetFlags ${cfg} 17
	done:
FunctionEnd
