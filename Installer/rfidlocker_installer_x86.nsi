# define name of installer
OutFile "RFIDLockerSetup_x86.exe"
 
# plugins
!include UAC.nsh 
 
# define installation directory
InstallDir "$PROGRAMFILES32\RfidLocker"
 
# For removing Start Menu shortcut in Windows 7
RequestExecutionLevel user
 
# start default section
Section
    InitPluginsDir

    Call UacInit
 
    # set the installation directory as the destination for the following actions
    SetOutPath $INSTDIR
 
    # create directory
    CreateDirectory $INSTDIR

    # create the file
    File RfidLocker.exe
    
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "RFID Locker" '"$INSTDIR\RfidLocker.exe"'
    
    # create the uninstaller
    WriteUninstaller "$INSTDIR\RfidUninstall.exe"

    # create a shortcut named "new shortcut" in the start menu programs directory
    # point the new shortcut at the program uninstaller
    CreateDirectory "$SMPROGRAMS\RFID Locker"
    CreateShortCut "$SMPROGRAMS\RFID Locker\RfidLocker.lnk" "$INSTDIR\RfidLocker.exe"
    CreateShortCut "$SMPROGRAMS\RFID Locker\Uninstall.lnk" "$INSTDIR\RfidUninstall.exe"
SectionEnd
 
# uninstaller section start
Section "uninstall"
    InitPluginsDir
    
    Call un.UacInit
 
    DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "RFID Locker"
 
    Delete "$INSTDIR\RfidUninstall.exe"
    Delete "$INSTDIR\RfidLocker.exe"
    RMDir $INSTDIR
    
    Delete "$SMPROGRAMS\RFID Locker\RFIDLocker.lnk"
    Delete "$SMPROGRAMS\RFID Locker\Uninstall.lnk"
    RMDir "$SMPROGRAMS\RFID Locker"
 
# uninstaller section end
SectionEnd

; Initialize the UAC, return if already admin and quit if we are launching a new EXE
!macro UACINIT un
  Function ${un}UacInit
  uac_tryagain:
  !insertmacro UAC_RunElevated
  ${If} $0 = 0
  	${If} $1 = 1  ;	we are the outer process, the inner process has done its work, we are done
		Quit
	${EndIf}
	${If} $3 <> 0  ;  	we are admin, let the show go on
		Return
	${EndIf}
	${If} $1 = 3 ; RunAs completed successfully, but with a non-admin user
	  MessageBox mb_YesNo|mb_IconExclamation|mb_TopMost|mb_SetForeground "The Price Finder installer requires admin privileges, try again" /SD IDNO IDYES uac_tryagain IDNO 0
	${EndIf}
  ${Else}
	; Installer success?
	Delete "$TEMP\pfinder.tmp"	;Delete our tmp file
	Quit
  ${EndIf}
  FunctionEnd
!macroend

!insertmacro UACINIT ""
!insertmacro UACINIT "un."