Unicode true

!define /ifndef PRODUCT "EasyTor"

!define /ifndef VER_MAJOR 0
!define /ifndef VER_MINOR 1

!ifdef VER_MAJOR & VER_MINOR
  !define /ifndef VER_REVISION 0
  !define /ifndef VER_BUILD 0
!endif

!define /ifndef VERSION "${VER_MAJOR}.${VER_MINOR}.${VER_REVISION}.${VER_BUILD}"

!define /ifndef OUTFILE "out\${PRODUCT}-${VERSION}-setup.exe"

Name "${PRODUCT}"
Caption "${PRODUCT} ${VERSION} Setup"

OutFile "${OUTFILE}"

SetCompressor /SOLID lzma

; The default installation directory
InstallDir "$LOCALAPPDATA\${PRODUCT}"

RequestExecutionLevel user
ShowInstDetails show
SetOverwrite off


LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"

;--------------------------------
;Version Information

  VIProductVersion ${VERSION}
  VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${PRODUCT}"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "A test comment"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Fake company"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "Test Application is a trademark of Fake company"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© Fake company"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Test Application"
  VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" ${VERSION}

;--------------------------------
; Pages

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------
; The stuff to install
Section "Install files"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\Release\TorController.exe"
  
  ; Write the installation path into the registry
  WriteRegStr HKCU "SOFTWARE\${PRODUCT}" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayName" "${PRODUCT}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
/*
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Example2"
  CreateShortcut "$SMPROGRAMS\Example2\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\Example2\Example2 (MakeNSISW).lnk" "$INSTDIR\example2.nsi" "" "$INSTDIR\example2.nsi" 0
  
SectionEnd
*/
;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"
  DeleteRegKey HKCU "SOFTWARE\${PRODUCT}"

  ; Remove files and uninstaller
  Delete $INSTDIR\TorController.exe
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${PRODUCT}\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\${PRODUCT}"
  RMDir "$INSTDIR"

SectionEnd
