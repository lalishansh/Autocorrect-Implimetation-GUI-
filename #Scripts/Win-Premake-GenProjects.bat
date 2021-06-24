@echo off
ECHO Premake Automatic Project Generation For Visual Studio 2019 will now start
ECHO if There's a prompt for CHOICE to be made, select A (most times, especially 1st time project generation)
CALL premakeFileSetupForVendors.bat A

ECHO --x------x------x--
pushd ..\
call ~buildSys\premake-binary\premake5.exe vs2019
popd
PAUSE