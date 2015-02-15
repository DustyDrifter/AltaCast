Function .onInit
  SetOutPath $TEMP
  File /oname=spltmp.bmp "oddsock_logo.bmp"
  splash::show 2500 $TEMP\spltmp
  Delete $TEMP\spltmp.bmp
FunctionEnd
;
; This script will generate an installer that installs a Winamp plug-in.
; It also puts a license page on, for shits and giggles.
;
; This installer will automatically alert the user that installation was
; successful, and ask them whether or not they would like to make the 
; plug-in the default and run Winamp.
;
; The name of the installer
Name "EdCast DSP For Winamp"

; The file to write
OutFile "edcast_winamp_3.1.21.exe"


LicenseText "EdCast DSP is released under the GNU Public License"
LicenseData ..\COPYING

UninstallText "This will uninstall EdCast Winamp DSP. Hit next to continue."

; The default installation directory
InstallDir $PROGRAMFILES\Winamp

; The text to prompt the user to enter a directory
DirText "Please select your Winamp path below :"
; DirShow hide

; automatically close the installer when done.
; AutoCloseWindow true
; hide the "show details" box
ShowInstDetails show

ComponentText "This will install EdCast DSP Winamp"

; The stuff to install
Section "EdCast DSP plugin for Winamp"

  SectionIn 1
  SetOutPath $INSTDIR\Plugins
  File "Release\dsp_edcast.dll"
  File "..\doc\_tmphhp\OddcastV3.chm"
  SetOutPath $INSTDIR
  File "..\..\external\lib\pthreadVSE.dll"
  File "..\..\external\lib\libfaac.dll"

  WriteUninstaller "dsp_edcast-uninst.exe"
  ; prompt user, and if they select no, skip the following 3 instructions.
SectionEnd

Section "BASS Audio DLLs"
SectionIn 1 2
SetOutPath $INSTDIR
File "..\..\external\lib\bass.dll"
SectionEnd
Section "OggFLAC DLLs"
SectionIn 1 2
SetOutPath $INSTDIR
File "..\..\external\lib\libOggFLAC.dll"
File "..\..\external\lib\libFLAC.dll"
SectionEnd
Section "Vorbis 1.1 DLLs"
SectionIn 1 3
SetOutPath $INSTDIR
File "..\..\external\lib\ogg.dll"
File "..\..\external\lib\vorbis.dll"
SectionEnd
Section /o "LAME encoder dll (for MP3 encoding)"
SectionIn 1 5
MessageBox MB_YESNO|MB_ICONQUESTION \
             "In order to broadcast in MP3, you must download the lame encoder dll (lame_enc.dll).  Due to legal issues, this cannot be distributed by me.  A browser will now open to a location that has been known to have copies. Place the dll 'lame_enc.dll' in the main Winamp program directory." \
             IDNO NoLAME
ExecShell open http://www.rarewares.org/mp3.html
NoLAME:
SectionEnd
Section /o "AAC encoder dll (for AAC encoding)"
SectionIn 1 6
MessageBox MB_YESNO|MB_ICONQUESTION \
             "In order to broadcast in AAC, you must download the FAAC encoder dll (libfaac.dll).  Due to legal issues, this cannot be distributed by me.  A browser will now open to a location that has been known to have copies. Place the dll 'libfacc.dll' in the main Winamp program directory." \
             IDNO NoAAC
ExecShell open http://www.rarewares.org/aac.html
NoAAC:
SectionEnd

; special uninstall section.
Section "uninstall"
Delete "$INSTDIR\Plugins\dsp_edcast.dll"
MessageBox MB_OK "EdCast DSP for Winamp Removed" IDOK 0 ; skipped if file doesn't exist
SectionEnd
; eof
