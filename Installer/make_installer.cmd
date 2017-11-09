@echo off
echo Generating script
c:\Python27\python.exe %~dp0..\version.py -f %~dp0..\VERSION -f %~dp0..\BRANDING -o %~dp0installer.cmd installer.cmd.in
echo Generating installer
call %~dp0installer.cmd
echo Done
del %~dp0installer.cmd