!include "MUI2.nsh"
!include "x64.nsh"

; The name of the installer
Name "GPXSee"
; Program version
!define VERSION "2.20"

; The file to write
OutFile "GPXSee-${VERSION}_x64.exe"

; Required execution level 
RequestExecutionLevel admin

; The default installation directory
InstallDir "$PROGRAMFILES64\GPXSee"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\GPXSee" "Install_Dir"

; Registry key for uninstaller
!define REGENTRY "Software\Microsoft\Windows\CurrentVersion\Uninstall\GPXSee"
; GPX file type registry entry
!define REGGPX "GPXSee.gpx" 

; Start menu page configuration
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\GPXSee" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "GPXSee" 

Var StartMenuFolder

;--------------------------------

; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "licence.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------

; Languages
!insertmacro MUI_LANGUAGE "English"

Function .onInit  
  ${If} ${RunningX64}
    SetRegView 64
  ${Else}  
    MessageBox MB_OK "The 64b version of GPXSee can not be run on 32b systems."  
    Abort  
  ${EndIf}
FunctionEnd 

; The stuff to install
Section "GPXSee (required)" SEC_APP

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put the files there
  File "gpxsee.exe"
  File "maps.txt"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\GPXSee "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "${REGENTRY}" "DisplayName" "GPXSee"
  WriteRegStr HKLM "${REGENTRY}" "Publisher" "Martin Tuma"
  WriteRegStr HKLM "${REGENTRY}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "${REGENTRY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "${REGENTRY}" "NoModify" 1
  WriteRegDWORD HKLM "${REGENTRY}" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Create start menu entry and add links
  SetShellVarContext all
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application  
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\GPXSee.lnk" "$INSTDIR\gpxsee.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  ; Associate .gpx files
  WriteRegStr HKCR ".gpx" "" "${REGGPX}"
  WriteRegStr HKCR "${REGGPX}" ""  "GPS Exchange Format"
  WriteRegStr HKCR "${REGGPX}\DefaultIcon" "" "$INSTDIR\GPXSee.exe,1"
  WriteRegStr HKCR "${REGGPX}\shell\open\command" "" "$\"$INSTDIR\GPXSee.exe$\" $\"%1$\""
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

SectionEnd

Section "QT libs" SEC_QT

  File "Qt5Core.dll"
  File "Qt5Gui.dll"
  File "Qt5Widgets.dll"
  File "Qt5PrintSupport.dll"
  File "Qt5Network.dll"
  File "libGLESv2.dll"
  File /r "platforms"
  File /r "imageformats"
  File /r "printsupport"
 
SectionEnd

Section "MSVC runtime" SEC_MSVC

  DetailPrint "Checking whether Visual C++ 2015 Redistributable is already installed..."
  ReadRegDword $R0 HKLM "SOFTWARE\Wow6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Installed"
  StrCmp $R0 "1" 0 +3
  DetailPrint "Visual C++ 2015 Redistributable is already installed, skipping install."
  Goto done

  DetailPrint "Installing Visual C++ 2015 Redistributable..."
  SetOutPath $TEMP
  File "VC_redist.x64.exe"
  ExecWait '"$TEMP/VC_redist.x64.exe" /install /quiet /norestart'

  done:
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  SetRegView 64
  DeleteRegKey HKLM "${REGENTRY}"
  DeleteRegKey HKLM SOFTWARE\GPXSee

  ; Remove directories used
  RMDir /r "$INSTDIR"

  ; Remove Start menu entries
  SetShellVarContext all
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\*.*"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  
  ; Remove GPX file association
  DeleteRegKey HKCR "${REGGPX}"
  DeleteRegKey HKCR ".gpx"
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

SectionEnd

;-------------------------------

;Descriptions

;Language strings
LangString DESC_QT ${LANG_ENGLISH} \
  "QT Library. Unselct only if you have QT already installed!"
LangString DESC_MSVC ${LANG_ENGLISH} \
  "Visual C++ 2015 runtime components. Unselct only if you have the runtime already installed!"
LangString DESC_APP ${LANG_ENGLISH} \
  "GPXSee application"

;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_QT} $(DESC_QT)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_MSVC} $(DESC_MSVC) 
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_APP} $(DESC_APP)
!insertmacro MUI_FUNCTION_DESCRIPTION_END