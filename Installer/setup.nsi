Unicode true

!define /ifndef PRODUCT "PraeTor"

!define /ifndef VER_MAJOR 0
!define /ifndef VER_MINOR 1
!define /ifndef VER_REVISION 0
!define /ifndef VER_BUILD 0

!define /ifndef VERSION "${VER_MAJOR}.${VER_MINOR}.${VER_REVISION}.${VER_BUILD}"

!define /ifndef OUTFILE "out\${PRODUCT}-${VERSION}-setup.exe"

!define DATA_ROOT "$LOCALAPPDATA\${PRODUCT}\Data"

Name "${PRODUCT}"
Caption "${PRODUCT} ${VERSION} Setup"

OutFile "${OUTFILE}"

SetCompressor /SOLID lzma

; The default installation directory
InstallDir "$LOCALAPPDATA\${PRODUCT}"

RequestExecutionLevel user
ShowInstDetails show
SetOverwrite off

;--------------------------------
;Header Files

!include "MUI2.nsh"
!include "StrRep.nsh"
!include "ReplaceInFile.nsh"

;--------------------------------
;Language Selection Dialog Settings

;Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU" 
!define MUI_LANGDLL_REGISTRY_KEY "Software\${PRODUCT}" 
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;--------------------------------
;Interface Settings

!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_NODESC 

;--------------------------------
; Pages

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English" ;first language is the default language
!insertmacro MUI_LANGUAGE "Russian"

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
;Reserve Files
  
  ;If you are using solid compression, files that are required before
  ;the actual installation should be stored first in the data block,
  ;because this will make your installer start faster.
  
!insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
; The stuff to install

Section "Install files"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR"
  File "..\Release\TorController.exe"

  ; Web UI files
  SetOutPath "$INSTDIR\UI"
  File /r "x:\EasyTorUI\build\*.*"

  ; Tor files
  SetOutPath "$INSTDIR\Tor"
  File /r "EtApps\tor-win32-0.3.1.7\*.*"

  ; Privoxy files
  SetOutPath "$INSTDIR\Privoxy"
  File /r "EtApps\privoxy-3.0.26\*.*"
  
  CreateDirectory "${DATA_ROOT}\Tor"
  CreateDirectory "${DATA_ROOT}\Privoxy"

  ; Config template
  SetOutPath "${DATA_ROOT}"
  File /oname=torcontroller.xml "..\TorController\torcontroller.xml.template"

  ; Replace %_DATA_ROOT_%
  !insertmacro _ReplaceInFile "${DATA_ROOT}\torcontroller.xml" "%_DATA_ROOT_%" "${DATA_ROOT}"
  Delete "${DATA_ROOT}\torcontroller.xml.old"

  ; Replace %_INSTALL_ROOT_%
  !insertmacro _ReplaceInFile "${DATA_ROOT}\torcontroller.xml" "%_INSTALL_ROOT_%" "$INSTDIR"
  Delete "${DATA_ROOT}\torcontroller.xml.old"
  
  ; Write the installation path into the registry
  WriteRegStr HKCU "SOFTWARE\${PRODUCT}" "Install_Dir" "$INSTDIR"
  WriteRegStr HKCU "SOFTWARE\${PRODUCT}" "Data_Dir" "${DATA_ROOT}"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "DisplayName" "${PRODUCT}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  ReadRegStr $1 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86" "Installed"
  StrCmp $1 1 installed

  ;not installed, so run the installer
  SetOutPath "$INSTDIR" 
  File "c:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Redist\MSVC\14.11.25325\vcredist_x86.exe" 	
  ExecWait '"$INSTDIR\vcredist_x86.exe" /install /passive /norestart'
installed:
  
SectionEnd

;--------------------------------
;Installer Functions

Function .onInit

  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

;--------------------------------
; Optional section (can be disabled by the user)

;Language strings
LangString UninstallShortcut ${LANG_ENGLISH} "Uninstall"
LangString UninstallShortcut ${LANG_RUSSIAN} "Uninstall"

LangString RunShortcut ${LANG_ENGLISH} "Start ${PRODUCT}"
LangString RunShortcut ${LANG_RUSSIAN} "Start ${PRODUCT}"

Section "Start Menu Shortcuts"
  ; Create shortcuts
  CreateDirectory "$SMPROGRAMS\${PRODUCT}"
  CreateShortcut "$SMPROGRAMS\${PRODUCT}\$(UninstallShortcut) ${PRODUCT}.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\${PRODUCT}\$(RunShortcut) ${PRODUCT}.lnk" "$INSTDIR\TorController.exe" "--config=$\"${DATA_ROOT}\torcontroller.xml$\"" "$INSTDIR\TorController.exe" 0
  WriteIniStr "$SMPROGRAMS\${PRODUCT}\${PRODUCT} Control Panel.url" "InternetShortcut" "URL" "http://127.0.0.1:30000/ui/index.html"
SectionEnd

;--------------------------------
; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}"
  DeleteRegKey HKCU "SOFTWARE\${PRODUCT}"

  ; Remove files and uninstaller
  Delete "$INSTDIR\TorController.exe"
  Delete "$INSTDIR\uninstall.exe"
  RMDir /r "$INSTDIR\Tor"
  RMDir /r "$INSTDIR\Privoxy"
  Delete "${DATA_ROOT}\torcontroller.xml"
  RMDir /r "${DATA_ROOT}"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${PRODUCT}\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\${PRODUCT}"
  RMDir "$INSTDIR"

SectionEnd

;--------------------------------
;Uninstaller Functions

Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd