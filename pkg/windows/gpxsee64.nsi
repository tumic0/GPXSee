!include "MUI2.nsh"
!include "x64.nsh"
!include "WinVer.nsh"


; Macros

; File association
!macro FILE_ASSOCIATION_ADD EXT DESC ICON
  WriteRegStr HKCR ".${EXT}" "" "GPXSee.${EXT}"
  WriteRegStr HKCR "GPXSee.${EXT}" ""  "${DESC}"
  WriteRegStr HKCR "GPXSee.${EXT}\DefaultIcon" "" "$INSTDIR\GPXSee.exe,${ICON}"
  WriteRegStr HKCR "GPXSee.${EXT}\shell\open\command" "" "$\"$INSTDIR\GPXSee.exe$\" $\"%1$\""
!macroend

!macro FILE_ASSOCIATION_REMOVE EXT
  DeleteRegKey HKCR "GPXSee.${EXT}"
  DeleteRegKey HKCR ".${EXT}"
!macroend

; Translations
!macro LOCALIZATION LANG CODE
  Section "${LANG}"
    IfFileExists "$INSTDIR\translations" +2 0
      CreateDirectory "$INSTDIR\translations" 
    File /oname=translations\gpxsee_${CODE}.qm translations\gpxsee_${CODE}.qm
    !if /FileExists translations\qt_${CODE}.qm
      File /oname=translations\qt_${CODE}.qm translations\qt_${CODE}.qm
    !endif
  SectionEnd
!macroend

;--------------------------------

Unicode true

; The name of the installer
Name "GPXSee"
; Program version
!define VERSION "13.34"

; The file to write
OutFile "GPXSee-${VERSION}_x64.exe"
; Compression method
SetCompressor /SOLID lzma

; Required execution level
RequestExecutionLevel admin

; Don't let the OS scale(blur) the installer GUI
ManifestDPIAware true

; The default installation directory
InstallDir "$PROGRAMFILES64\GPXSee"

; Installer executable info
VIProductVersion "${VERSION}.0.0"
VIAddVersionKey "ProductVersion" ${VERSION}
VIAddVersionKey "FileVersion" "${VERSION}.0.0"
VIAddVersionKey "ProductName" "GPXSee"
VIAddVersionKey "LegalCopyright" "Copyright (c) 2015-2025 Martin Tůma"
VIAddVersionKey "FileDescription" "GPXSee installer (x64)"

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
!ifdef QT6
  ${IfNot} ${AtLeastWin10}
    MessageBox MB_OK "GPXSee can only be installed on Windows 10 or later."
    Abort
  ${EndIf}
!else
  ${IfNot} ${AtLeastWin7}
    MessageBox MB_OK "GPXSee can only be installed on Windows 7 or later."
    Abort
  ${EndIf}
!endif

  ${If} ${RunningX64}
    SetRegView 64
  ${Else}
    MessageBox MB_OK "The 64b version of GPXSee can not be run on 32b systems."
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
  File /r "CRS"
  File /r "symbols"

  ; Create the uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
  ; Create start menu entry and add links
  SetShellVarContext all
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\GPXSee.lnk" "$INSTDIR\gpxsee.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  ; Write the installation path into the registry
  DetailPrint "Registering application..."
  WriteRegStr HKLM SOFTWARE\GPXSee "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "${REGENTRY}" "DisplayName" "GPXSee (x64)"
  WriteRegStr HKLM "${REGENTRY}" "Publisher" "Martin Tůma"
  WriteRegStr HKLM "${REGENTRY}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "${REGENTRY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "${REGENTRY}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
  WriteRegStr HKLM "${REGENTRY}" "DisplayIcon" '"$INSTDIR\gpxsee.exe"'
  WriteRegStr HKLM "${REGENTRY}" "InstallLocation" '"$INSTDIR"'
  WriteRegStr HKLM "${REGENTRY}" "URLInfoAbout" "https://www.gpxsee.org"
  WriteRegDWORD HKLM "${REGENTRY}" "NoModify" 1
  WriteRegDWORD HKLM "${REGENTRY}" "NoRepair" 1

  ; Associate file formats
  DetailPrint "Associating file types..."
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
  !insertmacro FILE_ASSOCIATION_ADD "gpx" "GPS Exchange Format" 11
  !insertmacro FILE_ASSOCIATION_ADD "map" "OziExplorer Map File" 12
  !insertmacro FILE_ASSOCIATION_ADD "gmi" "GPS Tuner Map Calibration File" 12
  !insertmacro FILE_ASSOCIATION_ADD "mbtiles" "MBTiles Map File" 13
  !insertmacro FILE_ASSOCIATION_ADD "rmap" "TwoNav Raster Map File" 14
  !insertmacro FILE_ASSOCIATION_ADD "tba" "TrekBuddy Atlas" 15
  !insertmacro FILE_ASSOCIATION_ADD "aqm" "AlpineQuest Map File" 16
  !insertmacro FILE_ASSOCIATION_ADD "sqlite" "Osmdroid SQLite Map File" 17
  !insertmacro FILE_ASSOCIATION_ADD "sqlitedb" "RMaps SQLite Map File" 17
  !insertmacro FILE_ASSOCIATION_ADD "ov2" "TomTom POI File" 18
  !insertmacro FILE_ASSOCIATION_ADD "itn" "TomTom Route File" 19
  !insertmacro FILE_ASSOCIATION_ADD "wld" "ESRI World File" 20
  !insertmacro FILE_ASSOCIATION_ADD "jgw" "ESRI World File" 20
  !insertmacro FILE_ASSOCIATION_ADD "gfw" "ESRI World File" 20
  !insertmacro FILE_ASSOCIATION_ADD "pgw" "ESRI World File" 20
  !insertmacro FILE_ASSOCIATION_ADD "tfw" "ESRI World File" 20
  !insertmacro FILE_ASSOCIATION_ADD "omd" "ONmove Log File" 21
  !insertmacro FILE_ASSOCIATION_ADD "tcx" "Training Center XML" 22
  !insertmacro FILE_ASSOCIATION_ADD "ghp" "ONmove Log File" 23
  !insertmacro FILE_ASSOCIATION_ADD "qct" "QuickChart Map File" 24
  !insertmacro FILE_ASSOCIATION_ADD "trk" "TwoNav Track File" 25
  !insertmacro FILE_ASSOCIATION_ADD "gemf" "GEMF Map File" 26
  !insertmacro FILE_ASSOCIATION_ADD "000" "IHO S-57 Electronic Navigation Chart" 27
  !insertmacro FILE_ASSOCIATION_ADD "031" "IHO S-57 Electronic Navigation Catalogue" 28
  !insertmacro FILE_ASSOCIATION_ADD "kml" "Keyhole Markup Language" 29
  !insertmacro FILE_ASSOCIATION_ADD "kmz" "KML geographic compressed data" 29
  !insertmacro FILE_ASSOCIATION_ADD "fit" "Flexible and Interoperable Data Transfer" 30
  !insertmacro FILE_ASSOCIATION_ADD "igc" "Flight Recorder Data Format" 31
  !insertmacro FILE_ASSOCIATION_ADD "nmea" "NMEA 0183 Data" 32
  !insertmacro FILE_ASSOCIATION_ADD "plt" "OziExplorer Track File" 33
  !insertmacro FILE_ASSOCIATION_ADD "rte" "OziExplorer Route File" 34

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
  WriteRegStr HKCR ".ov2\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".itn\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".csv\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".json\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".jpg\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".jpeg\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".img\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".jnx\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".kap\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".map\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".gmi\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".mbtiles\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".rmap\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".rtmap\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".tar\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".tba\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".tif\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".tiff\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".xml\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".kmz\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".aqm\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".sqlite\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".sqlitedb\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".wld\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".jgw\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".gfw\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".pgw\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".tfw\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".omd\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".ghp\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".qct\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".trk\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".gemf\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".000\OpenWithList" "GPXSee.exe" ""
  WriteRegStr HKCR ".031\OpenWithList" "GPXSee.exe" ""

  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

SectionEnd

Section "Qt framework" SEC_QT

  SectionIn RO

!ifdef QT6
  File "Qt6Concurrent.dll"
  File "Qt6Core.dll"
  File "Qt6Gui.dll"
  File "Qt6Network.dll"
  File "Qt6OpenGL.dll"
  File "Qt6OpenGLWidgets.dll"
  File "Qt6PrintSupport.dll"
  File "Qt6Sql.dll"
  File "Qt6Svg.dll"
  File "Qt6Widgets.dll"
  File "Qt6Positioning.dll"
  File "Qt6SerialPort.dll"
  File /r "tls"
!else
  File "Qt5Core.dll"
  File "Qt5Gui.dll"
  File "Qt5Widgets.dll"
  File "Qt5PrintSupport.dll"
  File "Qt5Network.dll"
  File "Qt5Sql.dll"
  File "Qt5Svg.dll"
  File "Qt5Concurrent.dll"
  File "Qt5Positioning.dll"
  File "Qt5SerialPort.dll" 
  File /r "printsupport"
!endif
  File /r "platforms"
  File /r "iconengines"
  File /r "imageformats"
  File /r "styles"
  File /r "sqldrivers"
  File /r "position"

SectionEnd

Section "MSVC runtime" SEC_MSVC

  SectionIn RO

  SetOutPath $TEMP
  File "vc_redist.x64.exe"
  ExecWait '"$TEMP\vc_redist.x64.exe" /install /quiet /norestart'
  SetOutPath $INSTDIR

SectionEnd

!ifdef ICU
Section "ICU" SEC_ICU

  SectionIn RO

  File "icudt*.dll"
  File "icuin*.dll"
  File "icuuc*.dll"

SectionEnd
!endif

!ifdef OPENSSL
Section "OpenSSL" SEC_OPENSSL

  SectionIn RO

  File "libcrypto-*-x64.dll"
  File "libssl-*-x64.dll"

SectionEnd
!endif

!ifdef ANGLE
Section "ANGLE" SEC_ANGLE

  File "libGLESv2.dll"
  File "libEGL.dll"
  File "D3DCompiler_47.dll"

SectionEnd
!endif

SectionGroup "Localization" SEC_LOCALIZATION
  !insertmacro LOCALIZATION "Catalan" "ca"
  !insertmacro LOCALIZATION "Chinese (Simplified)" "zh_CN"
  !insertmacro LOCALIZATION "Czech" "cs"
  !insertmacro LOCALIZATION "Danish" "da"
  !insertmacro LOCALIZATION "English" "en"
  !insertmacro LOCALIZATION "Esperanto" "eo"
  !insertmacro LOCALIZATION "Finnish" "fi"
  !insertmacro LOCALIZATION "French" "fr"
  !insertmacro LOCALIZATION "German" "de"
  !insertmacro LOCALIZATION "Hungarian" "hu"
  !insertmacro LOCALIZATION "Italian" "it"
  !insertmacro LOCALIZATION "Korean" "ko"
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
  !insertmacro FILE_ASSOCIATION_REMOVE "gmi"
  !insertmacro FILE_ASSOCIATION_REMOVE "mbtiles"
  !insertmacro FILE_ASSOCIATION_REMOVE "rmap"
  !insertmacro FILE_ASSOCIATION_REMOVE "tba"
  !insertmacro FILE_ASSOCIATION_REMOVE "kmz"
  !insertmacro FILE_ASSOCIATION_REMOVE "aqm"
  !insertmacro FILE_ASSOCIATION_REMOVE "sqlite"
  !insertmacro FILE_ASSOCIATION_REMOVE "sqlitedb"
  !insertmacro FILE_ASSOCIATION_REMOVE "ov2"
  !insertmacro FILE_ASSOCIATION_REMOVE "itn"
  !insertmacro FILE_ASSOCIATION_REMOVE "wld"
  !insertmacro FILE_ASSOCIATION_REMOVE "jgw"
  !insertmacro FILE_ASSOCIATION_REMOVE "gfw"
  !insertmacro FILE_ASSOCIATION_REMOVE "pgw"
  !insertmacro FILE_ASSOCIATION_REMOVE "tfw"
  !insertmacro FILE_ASSOCIATION_REMOVE "omd"
  !insertmacro FILE_ASSOCIATION_REMOVE "ghp"
  !insertmacro FILE_ASSOCIATION_REMOVE "qct"
  !insertmacro FILE_ASSOCIATION_REMOVE "trk"
  !insertmacro FILE_ASSOCIATION_REMOVE "gemf"
  !insertmacro FILE_ASSOCIATION_REMOVE "000"
  !insertmacro FILE_ASSOCIATION_REMOVE "031"

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
  DeleteRegValue HKCR ".ov2\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".itn\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".csv\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".json\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".jpg\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".jpeg\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".img\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".jnx\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".kap\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".map\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".gmi\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".mbtiles\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".rmap\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".rtmap\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".tar\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".tba\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".tif\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".tiff\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".xml\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".kmz\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".aqm\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".sqlite\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".sqlitedb\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".wld\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".jgw\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".gfw\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".pgw\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".tfw\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".omd\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".ghp\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".qct\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".trk\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".gemf\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".000\OpenWithList" "GPXSee.exe"
  DeleteRegValue HKCR ".031\OpenWithList" "GPXSee.exe"
  DeleteRegKey HKCR "Applications\GPXSee.exe"   
  
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

SectionEnd

;-------------------------------

; Descriptions

; Language strings
!ifdef QT6
LangString DESC_QT ${LANG_ENGLISH} \
  "Qt6 cross-platform application framework."
!else
LangString DESC_QT ${LANG_ENGLISH} \
  "Qt5 cross-platform application framework."
!endif
LangString DESC_MSVC ${LANG_ENGLISH} \
  "Microsoft Visual C++ runtime. If already installed, will be skipped."
!ifdef ICU
LangString DESC_ICU ${LANG_ENGLISH} \
  "ICU library. Required for character set/encoding conversions."
!endif
!ifdef OPENSSL
LangString DESC_OPENSSL ${LANG_ENGLISH} \
  "OpenSSL library. Qt SSL/TLS backend for HTTPS."
!endif
!ifdef ANGLE
LangString DESC_ANGLE ${LANG_ENGLISH} \
  "ANGLE (OpenGL via Direct3D). Enables OpenGL on systems without native OpenGL drivers."
!endif
LangString DESC_APP ${LANG_ENGLISH} \
  "GPXSee application"
LangString DESC_LOCALIZATION ${LANG_ENGLISH} \
  "Localization"

; Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_QT} $(DESC_QT)
!ifdef ICU
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_ICU} $(DESC_ICU)
!endif
!ifdef OPENSSL
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_OPENSSL} $(DESC_OPENSSL)
!endif
!ifdef ANGLE
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_ANGLE} $(DESC_ANGLE)
!endif
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_MSVC} $(DESC_MSVC) 
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_APP} $(DESC_APP)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_LOCALIZATION} $(DESC_LOCALIZATION)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
