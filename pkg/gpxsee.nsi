!include "MUI2.nsh"
!include "x64.nsh"
!include "WinVer.nsh"
!include "macros.nsh"


; The name of the installer
Name "GPXSee"
; Program version
!define VERSION "7.38"

; The file to write
OutFile "GPXSee-${VERSION}.exe"
; Compression method
SetCompressor /SOLID lzma

; Required execution level
RequestExecutionLevel admin

; Don't let the OS scale(blur) the installer GUI
ManifestDPIAware true

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

  ; Set output path to the installation directory
  SetOutPath $INSTDIR

  ; Put the files there
  File "gpxsee.exe"
  File /r "maps"
  File /r "csv"

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
  !insertmacro FILE_ASSOCIATION_ADD "gpx" "GPS Exchange Format" 11
  !insertmacro FILE_ASSOCIATION_ADD "tcx" "Training Center XML" 16
  !insertmacro FILE_ASSOCIATION_ADD "kml" "Keyhole Markup Language" 17
  !insertmacro FILE_ASSOCIATION_ADD "fit" "Flexible and Interoperable Data Transfer" 18
  !insertmacro FILE_ASSOCIATION_ADD "igc" "Flight Recorder Data Format" 19
  !insertmacro FILE_ASSOCIATION_ADD "nmea" "NMEA 0183 Data" 20
  !insertmacro FILE_ASSOCIATION_ADD "plt" "OziExplorer Track Point File" 21
  !insertmacro FILE_ASSOCIATION_ADD "rte" "OziExplorer Route File" 22
  !insertmacro FILE_ASSOCIATION_ADD "wpt" "OziExplorer Waypoint File" 1
  !insertmacro FILE_ASSOCIATION_ADD "loc" "Geocaching.com Waypoint File" 2
  !insertmacro FILE_ASSOCIATION_ADD "slf" "Sigma Log File" 3
  !insertmacro FILE_ASSOCIATION_ADD "geojson" "GeoJSON" 4
  !insertmacro FILE_ASSOCIATION_ADD "cup" "SeeYou CUP File" 5
  !insertmacro FILE_ASSOCIATION_ADD "gpi" "Garmin POI File" 6
  !insertmacro FILE_ASSOCIATION_ADD "sml" "Suunto Markup Language" 7
  !insertmacro FILE_ASSOCIATION_ADD "img" "Garmin IMG Map" 8
  !insertmacro FILE_ASSOCIATION_ADD "jnx" "Garmin JNX Map" 9
  !insertmacro FILE_ASSOCIATION_ADD "kap" "BSB Nautical Chart" 10
  !insertmacro FILE_ASSOCIATION_ADD "map" "OziExplorer Map File" 12
  !insertmacro FILE_ASSOCIATION_ADD "mbtiles" "MBTiles Map File" 13
  !insertmacro FILE_ASSOCIATION_ADD "rmap" "TwoNav Raster Map File" 14
  !insertmacro FILE_ASSOCIATION_ADD "tba" "TrekBuddy Atlas" 15

  WriteRegStr HKCR "Applications\GPXSee.exe\shell\open\command" "" "$\"$INSTDIR\GPXSee.exe$\" $\"%1$\""
  WriteRegStr HKCR ".gpx\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".tcx\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".kml\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".fit\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".igc\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".nmea\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".plt\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".rte\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".wpt\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".loc\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".slf\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".geojson\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".cup\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".gpi\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".sml\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".csv\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".json\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".jpg\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".jpeg\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".img\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".jnx\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".kap\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".map\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".mbtiles\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".rmap\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".rtmap\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".tar\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".tba\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".tif\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".tiff\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".xml\OpenWithList" "GPXSee.exe" ""  
  
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

SectionEnd

Section "QT framework" SEC_QT

  SectionIn RO

  File "Qt5Core.dll"
  File "Qt5Gui.dll"
  File "Qt5Widgets.dll"
  File "Qt5PrintSupport.dll"
  File "Qt5Network.dll"
  File "Qt5Sql.dll"
  File "Qt5Concurrent.dll"
  File /r "platforms"
  File /r "imageformats"
  File /r "printsupport"
  File /r "styles"
  File /r "sqldrivers"

SectionEnd

Section "MSVC runtime" SEC_MSVC

  SectionIn RO

  SetOutPath $TEMP
  File "vc_redist.x86.exe"
  ExecWait '"$TEMP\vc_redist.x86.exe" /install /quiet /norestart'
  SetOutPath $INSTDIR    

SectionEnd

Section "OpenSSL" SEC_OPENSSL

  File "libcrypto-1_1.dll"
  File "libssl-1_1.dll"

SectionEnd

Section "ANGLE" SEC_ANGLE

  File "libGLESv2.dll"
  File "libEGL.dll"
  File "D3DCompiler_47.dll"

SectionEnd

SectionGroup "Localization" SEC_LOCALIZATION
  !insertmacro LOCALIZATION "Czech" "cs"
  !insertmacro LOCALIZATION "Danish" "da"
  !insertmacro LOCALIZATION "Finnish" "fi"
  !insertmacro LOCALIZATION "French" "fr"
  !insertmacro LOCALIZATION "German" "de"
  !insertmacro LOCALIZATION "Hungarian" "hu"
  !insertmacro LOCALIZATION "Italian" "it"
  !insertmacro LOCALIZATION "Norwegian" "nb"
  !insertmacro LOCALIZATION "Polish" "pl"
  !insertmacro LOCALIZATION "Portuguese (Brazil)" "pt_BR"
  !insertmacro LOCALIZATION "Russian" "ru"
  !insertmacro LOCALIZATION "Spanish" "es"
  !insertmacro LOCALIZATION "Swedish" "sv"
  !insertmacro LOCALIZATION "Turkish" "tr"
  !insertmacro LOCALIZATION "Ukrainian" "uk"
SectionGroupEnd

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

  ; Remove file associations
  !insertmacro FILE_ASSOCIATION_REMOVE "gpx"
  !insertmacro FILE_ASSOCIATION_REMOVE "tcx"
  !insertmacro FILE_ASSOCIATION_REMOVE "kml"
  !insertmacro FILE_ASSOCIATION_REMOVE "fit"
  !insertmacro FILE_ASSOCIATION_REMOVE "igc"
  !insertmacro FILE_ASSOCIATION_REMOVE "nmea"
  !insertmacro FILE_ASSOCIATION_REMOVE "plt"
  !insertmacro FILE_ASSOCIATION_REMOVE "rte"
  !insertmacro FILE_ASSOCIATION_REMOVE "wpt"
  !insertmacro FILE_ASSOCIATION_REMOVE "loc"
  !insertmacro FILE_ASSOCIATION_REMOVE "slf"
  !insertmacro FILE_ASSOCIATION_REMOVE "geojson"
  !insertmacro FILE_ASSOCIATION_REMOVE "cup"
  !insertmacro FILE_ASSOCIATION_REMOVE "gpi"
  !insertmacro FILE_ASSOCIATION_REMOVE "sml"
  !insertmacro FILE_ASSOCIATION_REMOVE "img"
  !insertmacro FILE_ASSOCIATION_REMOVE "jnx"
  !insertmacro FILE_ASSOCIATION_REMOVE "kap"
  !insertmacro FILE_ASSOCIATION_REMOVE "map"
  !insertmacro FILE_ASSOCIATION_REMOVE "mbtiles"
  !insertmacro FILE_ASSOCIATION_REMOVE "rmap"
  !insertmacro FILE_ASSOCIATION_REMOVE "tba"

  DeleteRegValue HKCR ".gpx\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".tcx\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".kml\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".fit\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".igc\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".nmea\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".plt\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".rte\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".wpt\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".loc\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".slf\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".geojson\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".cup\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".gpi\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".sml\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".csv\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".json\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".jpg\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".jpeg\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".img\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".jnx\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".kap\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".map\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".mbtiles\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".rmap\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".rtmap\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".tar\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".tba\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".tif\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".tiff\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".xml\OpenWithList" "GPXSee.exe"
  DeleteRegKey HKCR "Applications\GPXSee.exe"
  
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

SectionEnd

;-------------------------------

;Descriptions

;Language strings
LangString DESC_QT ${LANG_ENGLISH} \
  "QT cross-platform application framework."
LangString DESC_MSVC ${LANG_ENGLISH} \
  "Microsoft Visual C++ 2017 runtime. If already installed, will be skipped."
LangString DESC_OPENSSL ${LANG_ENGLISH} \
  "OpenSSL library. Required for HTTPS to work."
LangString DESC_ANGLE ${LANG_ENGLISH} \
  "ANGLE (OpenGL via Direct3D). Enables OpenGL on systems without native OpenGL drivers."
LangString DESC_APP ${LANG_ENGLISH} \
  "GPXSee application"
LangString DESC_LOCALIZATION ${LANG_ENGLISH} \
  "Localization"

;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_QT} $(DESC_QT)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_OPENSSL} $(DESC_OPENSSL)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_ANGLE} $(DESC_ANGLE)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_MSVC} $(DESC_MSVC) 
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_APP} $(DESC_APP)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_LOCALIZATION} $(DESC_LOCALIZATION)
!insertmacro MUI_FUNCTION_DESCRIPTION_END