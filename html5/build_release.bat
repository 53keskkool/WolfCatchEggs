set CURPATH=%cd%
cd ..
call app_info_setup.bat
:um, why does the emsdk_env.bat not fully work unless I'm in the emscripten dir?  Whatever, we'll move there and then back
cd %EMSCRIPTEN_ROOT%
call emsdk_env.bat
:Move back to original directory
cd %CURPATH%

where /q emsdk_env.bat

if ERRORLEVEL 1 (
    ECHO You need the environmental EMSCRIPTEN_ROOT set.  This should be set in setup_base.bat in proton's main dir, then called from app_info_setup.bat.
     pause
     exit
)

call build_env.bat

del %APP_NAME%.js*
del %APP_NAME%.wasm*
del %APP_NAME%.data

:compile some libs into a separate thing, otherwise our list of files is too long and breaks stuff
call emcc %CUSTOM_FLAGS% %INCLUDE_DIRS% ^
%ZLIB_SRC% %JPG_SRC% -r -o temp.o

call emcc %CUSTOM_FLAGS% %INCLUDE_DIRS% ^
%APP_SRC% %SRC% %COMPONENT_SRC% temp.o ^
--preload-file ../bin/interface@interface/ --preload-file ../bin/audio@audio/ --preload-file ../bin/game@game/ -lidbfs.js --js-library %SHARED%\html5\SharedJSLIB.js -o %APP_NAME%.%FINAL_EXTENSION%

del temp.o

REM Make sure the file compiled ok
if not exist %APP_NAME%.js (
echo Compile failed.
IF NOT "%1" == "nopause" pause
exit
)

IF "%1" == "nopause" (
echo no pause wanted
) else (
echo Compile complete.
pause
)
