@echo off
chcp 936 >nul
echo 编译 ControllerOS OOBE

cl /c main.c /Fo:main.obj
cl /c oobe.c /Fo:oobe.obj
cl /c chooses.c /Fo:chooses.obj
rc resource.rc
link main.obj oobe.obj chooses.obj resource.res /OUT:oobe.exe /SUBSYSTEM:WINDOWS /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64" user32.lib gdi32.lib advapi32.lib shell32.lib kernel32.lib

echo 编译完成
pause