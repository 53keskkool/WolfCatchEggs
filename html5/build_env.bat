:Set below to DEBUG=1 for debug mode builds - slower but way easier to see problems. Disables the ASYNC stuff as that doesn't seem to play
:well with the rest
SET DEBUG=0

SET USE_HTML5_CUSTOM_MAIN=1

SET SHARED=..\..\shared

SET APP=..\source

SET COMPPATH=%SHARED%\Entity
SET CLANMATH=%SHARED%\ClanLib-2.0\Sources\Core\Math
SET ZLIBPATH=%SHARED%\util\zlib
set PPATH=%SHARED%\Renderer\linearparticle\sources
set COMPPATH=%SHARED%\Entity
set PNGSRC=%SHARED%\Irrlicht\source\Irrlicht\libpng
set JPGSRC=%SHARED%\Irrlicht\source\Irrlicht\jpeglib
set LZMASRC=%SHARED%\Irrlicht\source\Irrlicht\lzma

set SRC= %SHARED%\PlatformSetup.cpp  %SHARED%\html5\HTML5Main.cpp %SHARED%\html5\HTML5Utils.cpp ^
%SHARED%\Audio\AudioManager.cpp %CLANMATH%\angle.cpp %CLANMATH%\mat3.cpp %CLANMATH%\mat4.cpp %CLANMATH%\rect.cpp %CLANMATH%\vec2.cpp %CLANMATH%\vec3.cpp ^
%CLANMATH%\vec4.cpp %SHARED%\Entity\Entity.cpp %SHARED%\Entity\Component.cpp %SHARED%\GUI\RTFont.cpp %SHARED%\Manager\Console.cpp ^
%SHARED%\Manager\GameTimer.cpp %SHARED%\Manager\MessageManager.cpp %SHARED%\Manager\ResourceManager.cpp %SHARED%\Manager\VariantDB.cpp %SHARED%\Math\rtPlane.cpp ^
%SHARED%\Math\rtRect.cpp %SHARED%\Renderer\RenderBatcher.cpp %SHARED%\Renderer\SoftSurface.cpp %SHARED%\Renderer\Surface.cpp %SHARED%\Renderer\SurfaceAnim.cpp ^
%SHARED%\util\CRandom.cpp %SHARED%\util\GLESUtils.cpp %SHARED%\util\MathUtils.cpp %SHARED%\util\MiscUtils.cpp %SHARED%\util\RenderUtils.cpp %SHARED%\util\ResourceUtils.cpp ^
%SHARED%\util\Variant.cpp %SHARED%\BaseApp.cpp %SHARED%\util\TextScanner.cpp %SHARED%\Entity\EntityUtils.cpp ^
%SHARED%\Audio\AudioManagerSDL.cpp %SHARED%\util\unzip\unzip.c %SHARED%\util\unzip\ioapi.c ^
%SHARED%\FileSystem\StreamingInstance.cpp %SHARED%\FileSystem\StreamingInstanceZip.cpp %SHARED%\FileSystem\StreamingInstanceFile.cpp %SHARED%\FileSystem\FileSystem.cpp ^
%SHARED%\FileSystem\FileSystemZip.cpp %SHARED%\FileSystem\FileManager.cpp %SHARED%\Renderer\JPGSurfaceLoader.cpp

REM **************************************** ENGINE COMPONENT SOURCE CODE FILES
set COMPONENT_SRC=%COMPPATH%\Button2DComponent.cpp %COMPPATH%\FilterInputComponent.cpp %COMPPATH%\FocusInputComponent.cpp %COMPPATH%\FocusRenderComponent.cpp %COMPPATH%\FocusUpdateComponent.cpp ^
%COMPPATH%\InputTextRenderComponent.cpp %COMPPATH%\InterpolateComponent.cpp %COMPPATH%\OverlayRenderComponent.cpp ^
%COMPPATH%\RectRenderComponent.cpp %COMPPATH%\ScrollBarRenderComponent.cpp %COMPPATH%\ScrollComponent.cpp %COMPPATH%\TextBoxRenderComponent.cpp ^
%COMPPATH%\TextRenderComponent.cpp %COMPPATH%\TyperComponent.cpp %COMPPATH%\UnderlineRenderComponent.cpp ^
%COMPPATH%\TouchHandlerComponent.cpp %COMPPATH%\CustomInputComponent.cpp %COMPPATH%\SelectButtonWithCustomInputComponent.cpp %COMPPATH%\SliderComponent.cpp %COMPPATH%\EmitVirtualKeyComponent.cpp ^
%COMPPATH%\RenderScissorComponent.cpp %COMPPATH%\ArcadeInputComponent.cpp

REM **************************************** JPEG SOURCE CODE FILES
set JPG_SRC=%JPGSRC%\jcapimin.c %JPGSRC%\jcapistd.c %JPGSRC%\jccoefct.c %JPGSRC%\jccolor.c %JPGSRC%\jcdctmgr.c %JPGSRC%\jchuff.c %JPGSRC%\jcinit.c %JPGSRC%\jcmainct.c ^
%JPGSRC%\jcmarker.c %JPGSRC%\jcmaster.c %JPGSRC%\jcomapi.c %JPGSRC%\jcparam.c %JPGSRC%\jcphuff.c %JPGSRC%\jcprepct.c %JPGSRC%\jcsample.c %JPGSRC%\jctrans.c ^
%JPGSRC%\jdapimin.c %JPGSRC%\jdapistd.c %JPGSRC%\jdatadst.c %JPGSRC%\jdatasrc.c %JPGSRC%\jdcoefct.c %JPGSRC%\jdcolor.c %JPGSRC%\jddctmgr.c ^
%JPGSRC%\jdhuff.c %JPGSRC%\jdinput.c %JPGSRC%\jdmainct.c %JPGSRC%\jdmarker.c %JPGSRC%\jdmaster.c %JPGSRC%\jdmerge.c %JPGSRC%\jdphuff.c %JPGSRC%\jdpostct.c ^
%JPGSRC%\jdsample.c %JPGSRC%\jdtrans.c %JPGSRC%\jerror.c %JPGSRC%\jfdctflt.c %JPGSRC%\jfdctfst.c %JPGSRC%\jfdctint.c %JPGSRC%\jidctflt.c %JPGSRC%\jidctfst.c ^
%JPGSRC%\jidctint.c %JPGSRC%\jidctred.c %JPGSRC%\jmemmgr.c %JPGSRC%\jmemnobs.c %JPGSRC%\jquant1.c %JPGSRC%\jquant2.c %JPGSRC%\jutils.c


REM **************************************** ZLIB SOURCE CODE FILES
set ZLIB_SRC=%ZLIBPATH%/deflate.c %ZLIBPATH%/gzio.c %ZLIBPATH%/infback.c %ZLIBPATH%/inffast.c %ZLIBPATH%/inflate.c %ZLIBPATH%/inftrees.c %ZLIBPATH%/trees.c %ZLIBPATH%/uncompr.c %ZLIBPATH%/zutil.c %ZLIBPATH%/adler32.c %ZLIBPATH%/compress.c %ZLIBPATH%/crc32.c

REM **************************************** APP SOURCE CODE FILES
set APP_SRC=%APP%\App.cpp %APP%\Component\EggManager.cpp %APP%\Component\EggComponent.cpp %APP%\Component\WolfComponent.cpp ^
%APP%\GUI\MainMenu.cpp %APP%\GUI\AboutMenu.cpp %APP%\GUI\GameMenu.cpp %APP%\GUI\GUIUtils.cpp %APP%\GUI\LanguageSelectMenu.cpp ^
%APP%\GUI\OptionsMenu.cpp %APP%\GUI\OverlapTestMenu.cpp %APP%\GUI\ThemeSelectMenu.cpp %APP%\GUI\GameOverMenu.cpp %APP%\GUI\PauseMenu.cpp ^
%APP%\GUI\PhoneWebMenu.cpp %APP%\GUI\TutorialMenu.cpp %APP%\GUI\GamemodeSelectMenu.cpp ^
%APP%\Managers\ThemeManager.cpp %APP%\Managers\TranslationManager.cpp
REM **************************************** END SOURCE

:unused so far: -s USE_GLFW=3 -s NO_EXIT_RUNTIME=1 -s FORCE_ALIGNED_MEMORY=1 -s EMTERPRETIFY=1  -s EMTERPRETIFY_ASYNC=1 -DRT_EMTERPRETER_ENABLED
:To skip font loading so it needs no resource files or zlib, add  -DC_NO_ZLIB
SET CUSTOM_FLAGS= -DHAS_SOCKLEN_T -DBOOST_ALL_NO_LIB -DPLATFORM_HTML5 -DRT_USE_SDL_AUDIO -DRT_JPG_SUPPORT -DC_GL_MODE -s LEGACY_GL_EMULATION=1 -Wno-switch -s WASM=1 -s TOTAL_MEMORY=16MB -Wno-deprecated-builtins -Wno-c++11-compat-deprecated-writable-strings -Wno-shift-negative-value -Wno-deprecated-non-prototype -s ALLOW_MEMORY_GROWTH=1 -sUSE_SDL

:unused:   -s FULL_ES2=1 --emrun

IF %USE_HTML5_CUSTOM_MAIN% EQU 1 (
:add this define so we'll manually call mainf from the html later instead of it being auto
SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -DRT_HTML5_USE_CUSTOM_MAIN -s EXPORTED_FUNCTIONS=['_mainf','_PROTON_SystemMessage','_PROTON_GUIMessage'] -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']
SET FINAL_EXTENSION=js
) else (
SET FINAL_EXTENSION=html
)

IF %DEBUG% EQU 0 (
echo Compiling in release mode
SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -O2 -DNDEBUG
) else (
echo Compiling in debug mode
SET CUSTOM_FLAGS=%CUSTOM_FLAGS% -D_DEBUG -s GL_UNSAFE_OPTS=0 -s WARN_ON_UNDEFINED_SYMBOLS=1 -s EXCEPTION_DEBUG=1 -s DEMANGLE_SUPPORT=1 -s ALIASING_FUNCTION_POINTERS=0
)

SET INCLUDE_DIRS=-I%SHARED% -I%APP% -I../../shared/util/boost -I../../shared/ClanLib-2.0/Sources -I../../shared/Network/enet/include ^
-I%ZLIBPATH%