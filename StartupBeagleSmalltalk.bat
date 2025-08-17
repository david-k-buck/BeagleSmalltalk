@echo off
cd /d "%~dp0"

set "path=%path%;%~dp0%\lib"
START /B .\beagle.exe beagle.im

set "htmlFile=%~dp0BeagleUI.html"
set "htmlFile=file:///%htmlFile:\=/%"

"%ProgramFiles(x86)%\Microsoft\Edge\Application\msedge.exe" "%htmlFile%"
