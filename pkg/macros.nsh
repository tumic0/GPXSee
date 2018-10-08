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
    CreateDirectory "$INSTDIR\translations"
    File /oname=translations\gpxsee_${CODE}.qm translations\gpxsee_${CODE}.qm
    !if /FileExists translations\qt_${CODE}.qm
      File /oname=translations\qt_${CODE}.qm translations\qt_${CODE}.qm
    !endif
  SectionEnd
!macroend
