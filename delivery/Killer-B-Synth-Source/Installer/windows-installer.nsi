!include "MUI2.nsh"

!ifndef OUTPUT_EXE
  !error "OUTPUT_EXE define is required"
!endif

!ifndef VST3_BUNDLE
  !error "VST3_BUNDLE define is required"
!endif

Unicode True
Name "Killer B Synth"
OutFile "${OUTPUT_EXE}"
InstallDir "$PROGRAMFILES64\Common Files\VST3"
RequestExecutionLevel admin
SetCompressor /SOLID lzma
SetCompressorDictSize 64

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "Install"
  SetRegView 64
  RMDir /r "$INSTDIR\Killer B Synth.vst3"
  SetOutPath "$INSTDIR"
  File /r "${VST3_BUNDLE}"
SectionEnd
