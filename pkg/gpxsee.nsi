!include "MUI2.nsh"
!include "x64.nsh"
!include "WinVer.nsh"

; The name of the installer
Name "GPXSee"
; Program version
!define VERSION "4.16"

; The file to write
OutFile "GPXSee-${VERSION}.exe"
; Compression method
SetCompressor /SOLID lzma

; Required execution level 
RequestExecutionLevel admin

; The default installation directory
InstallDir "$PROGRAMFILES\GPXSee"

; Installer executable info
VIProductVersion "${VERSION}.0.0"
VIAddVersionKey "ProductVersion" ${VERSION}
VIAddVersionKey "FileVersion" "${VERSION}.0.0"
VIAddVersionKey "ProductName" "GPXSee"
VIAddVersionKey "LegalCopyright" "GPXSee project"
VIAddVersionKey "FileDescription" "GPXSee installer"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\GPXSee" "Install_Dir"

; Registry key for uninstaller
!define REGENTRY "Software\Microsoft\Windows\CurrentVersion\Uninstall\GPXSee"
; File types registry entries
!define REGGPX "GPXSee.gpx"
!define REGTCX "GPXSee.tcx"
!define REGKML "GPXSee.kml"
!define REGFIT "GPXSee.fit"
!define REGIGC "GPXSee.igc"
!define REGNMEA "GPXSee.nmea"

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
  ${IfNot} ${AtLeastWin7}
    MessageBox MB_OK "GPXSee can only be installed on Windows 7 or later."
    Abort
  ${EndIf}
FunctionEnd 

; The stuff to install
Section "GPXSee" SEC_APP

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put the files there
  File "gpxsee.exe"
  File "maps.txt"
  File "ellipsoids.csv"
  File "datums.csv"

  ; Create start menu entry and add links
  SetShellVarContext all
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application  
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\GPXSee.lnk" "$INSTDIR\gpxsee.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  ; Create the uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe" 

  ; Write the installation path into the registry
  DetailPrint "Registering application..."
  WriteRegStr HKLM SOFTWARE\GPXSee "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "${REGENTRY}" "DisplayName" "GPXSee"
  WriteRegStr HKLM "${REGENTRY}" "Publisher" "Martin Tuma"
  WriteRegStr HKLM "${REGENTRY}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "${REGENTRY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "${REGENTRY}" "NoModify" 1
  WriteRegDWORD HKLM "${REGENTRY}" "NoRepair" 1

  ; Associate file formats
  DetailPrint "Associating file types..."
  WriteRegStr HKCR ".gpx" "" "${REGGPX}"
  WriteRegStr HKCR "${REGGPX}" ""  "GPS Exchange Format"
  WriteRegStr HKCR "${REGGPX}\DefaultIcon" "" "$INSTDIR\GPXSee.exe,1"
  WriteRegStr HKCR "${REGGPX}\shell\open\command" "" "$\"$INSTDIR\GPXSee.exe$\" $\"%1$\""
  WriteRegStr HKCR ".tcx" "" "${REGTCX}"
  WriteRegStr HKCR "${REGTCX}" ""  "Training Center XML"
  WriteRegStr HKCR "${REGTCX}\DefaultIcon" "" "$INSTDIR\GPXSee.exe,2"
  WriteRegStr HKCR "${REGTCX}\shell\open\command" "" "$\"$INSTDIR\GPXSee.exe$\" $\"%1$\""
  WriteRegStr HKCR ".kml" "" "${REGKML}"
  WriteRegStr HKCR "${REGKML}" ""  "Keyhole Markup Language"
  WriteRegStr HKCR "${REGKML}\DefaultIcon" "" "$INSTDIR\GPXSee.exe,3"
  WriteRegStr HKCR "${REGKML}\shell\open\command" "" "$\"$INSTDIR\GPXSee.exe$\" $\"%1$\""
  WriteRegStr HKCR ".fit" "" "${REGFIT}"
  WriteRegStr HKCR "${REGFIT}" ""  "Flexible and Interoperable Data Transfer"
  WriteRegStr HKCR "${REGFIT}\DefaultIcon" "" "$INSTDIR\GPXSee.exe,4"
  WriteRegStr HKCR "${REGFIT}\shell\open\command" "" "$\"$INSTDIR\GPXSee.exe$\" $\"%1$\""
  WriteRegStr HKCR ".igc" "" "${REGIGC}"
  WriteRegStr HKCR "${REGIGC}" ""  "Flight Recorder Data Format"
  WriteRegStr HKCR "${REGIGC}\DefaultIcon" "" "$INSTDIR\GPXSee.exe,5"
  WriteRegStr HKCR "${REGIGC}\shell\open\command" "" "$\"$INSTDIR\GPXSee.exe$\" $\"%1$\""
  WriteRegStr HKCR ".nmea" "" "${REGNMEA}"
  WriteRegStr HKCR "${REGNMEA}" ""  "NMEA 0183 data"
  WriteRegStr HKCR "${REGNMEA}\DefaultIcon" "" "$INSTDIR\GPXSee.exe,6"
  WriteRegStr HKCR "${REGNMEA}\shell\open\command" "" "$\"$INSTDIR\GPXSee.exe$\" $\"%1$\""

  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

SectionEnd

Section "QT framework" SEC_QT

  SectionIn RO

  File "Qt5Core.dll"
  File "Qt5Gui.dll"
  File "Qt5Widgets.dll"
  File "Qt5PrintSupport.dll"
  File "Qt5Network.dll"
  File /r "platforms"
  File /r "imageformats"
  File /r "printsupport"
 
SectionEnd

Section "MSVC runtime" SEC_MSVC

  SectionIn RO

  DetailPrint "Checking whether Visual C++ 2015 Redistributable is already installed..."
  ${If} ${RunningX64}
    ReadRegDword $R0 HKLM "SOFTWARE\Wow6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x86" "Installed"
  ${Else}
    ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86" "Installed"
  ${EndIf}

  StrCmp $R0 "1" 0 +3
  DetailPrint "Visual C++ 2015 Redistributable is already installed, skipping install."
  Goto done

  DetailPrint "Installing Visual C++ 2015 Redistributable..."
  SetOutPath $TEMP
  File "vcredist_x86.exe"
  ExecWait '"$TEMP\vcredist_x86.exe" /install /quiet /norestart'
  SetOutPath $INSTDIR

  done:
SectionEnd

Section "OpenSSL" SEC_OPENSSL

  File "libeay32.dll"
  File "ssleay32.dll"

SectionEnd

Section "ANGLE" SEC_ANGLE

  File "libGLESv2.dll"
  File "libEGL.dll"
  File "D3DCompiler_47.dll"

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
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
  DeleteRegKey HKCR "${REGTCX}"
  DeleteRegKey HKCR ".tcx"
  DeleteRegKey HKCR "${REGKML}"
  DeleteRegKey HKCR ".kml"
  DeleteRegKey HKCR "${REGFIT}"
  DeleteRegKey HKCR ".fit"
  DeleteRegKey HKCR "${REGIGC}"
  DeleteRegKey HKCR ".igc"
  DeleteRegKey HKCR "${REGNMEA}"
  DeleteRegKey HKCR ".nmea"
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'  

SectionEnd

;-------------------------------

;Descriptions

;Language strings
LangString DESC_QT ${LANG_ENGLISH} \
  "QT cross-platform application framework."
LangString DESC_MSVC ${LANG_ENGLISH} \
  "Visual C++ 2015 runtime components. If already installed, will be skipped."
LangString DESC_OPENSSL ${LANG_ENGLISH} \
  "OpenSSL library. Required for HTTPS to work."
LangString DESC_ANGLE ${LANG_ENGLISH} \
  "ANGLE (OpenGL via Direct3D). Enables OpenGL on systems without native OpenGL drivers."
LangString DESC_APP ${LANG_ENGLISH} \
  "GPXSee application"

;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_QT} $(DESC_QT)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_OPENSSL} $(DESC_OPENSSL)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_ANGLE} $(DESC_ANGLE)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_MSVC} $(DESC_MSVC) 
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_APP} $(DESC_APP)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
