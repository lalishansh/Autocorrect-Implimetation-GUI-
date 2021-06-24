@echo off
pushd ..\

set premakeFilesLocation=.\#Scripts\premakeFilesForVendors\
set vendorProjectsLoc=.\~vendor\
set vendorProjectsWithPremake=(imgui glfw)
set files=(premake5.lua)
set user_selecting_option=0
:: Auto-select Option
IF %1.==. GOTO Selection
IF %1%==A GOTO OptionA
IF %1%==B GOTO OptionB

:Selection
set user_selecting_option=1
echo "Choose direction to copy files"
echo "A: premakeAndOtherFilesForVendor/(PROJECTS) -> ~vendor/(PROJECTS)"
echo "B: ~vendor/(PROJECTS) -> premakeAndOtherFilesForVendor/(PROJECTS)"

CHOICE /C AB /M "Option "
IF ERRORLEVEL 2 GOTO OptionB
IF ERRORLEVEL 1 GOTO OptionA

:OptionA
IF %user_selecting_option%==0 ECHO ------------
IF %user_selecting_option%==0 ECHO  #Auto-Selected Option A
IF %user_selecting_option%==0 ECHO ------------
set sourceDIR=%premakeFilesLocation%
set destDIR=%vendorProjectsLoc%
GOTO End

:OptionB
IF %user_selecting_option%==0 ECHO #Auto-Selected Option B
set sourceDIR=%vendorProjectsLoc%
set destDIR=%premakeFilesLocation%
GOTO End

:End
FOR %%p IN %vendorProjectsWithPremake% DO (
    FOR %%f IN %files% DO (

        :: [/d] Use the command with /d option and a specific date, in MM-DD-YYYY format, to copy files changed on or after that date. You can also use this option without specifying a specific date to copy only those files in source that are newer than the same files that already exist in destination. This is helpful when using xcopy to perform regular file backups.
        :: [/s] Use this option to copy directories, subdirectories, and the files contained within them, in addition to the files in the root of source. Empty folders will not be recreated.
        :: [/y] Use this option to stop the command from prompting you about overwriting files from source that already exist in destination.
        :: [/c] This option forces xcopy to continue even if it encounters an error.
        IF %user_selecting_option%==1 (XCOPY /s /c "%sourceDIR%%%p\%%f" "%destDIR%%%p\") ELSE (XCOPY /d /s /y /c "%sourceDIR%%%p\%%f" "%destDIR%%%p\")
    )
)
IF %user_selecting_option%==1 PAUSE
popd