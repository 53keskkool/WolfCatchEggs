# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("RTAndroidApp")


set(APP "../../../../../source")
set(SHARED "../../../../../../shared")
set(COMPPATH "${SHARED}/Entity")
set(CLANMATH "${SHARED}/ClanLib-2.0/Sources/Core/Math")

set(ZLIBPATH "${SHARED}/util/zlib")
set(PPATH "${SHARED}/Renderer/linearparticle/sources")
set(PNGSRC "${SHARED}/Irrlicht/source/Irrlicht/libpng")
set(JPGSRC "${SHARED}/Irrlicht/source/Irrlicht/jpeglib")
set(LZMASRC "${SHARED}/Irrlicht/source/Irrlicht/lzma")

include_directories("${SHARED}")
include_directories("${SHARED}/util/boost")
include_directories("${SHARED}/ClanLib-2.0/Sources")
include_directories("${APP}")
include_directories("${SHARED}/android/optional_src/libs")

add_definitions(-Wno-error=format-security -DANDROID_NDK -DBOOST_ALL_NO_LIB -DBUILD_ANDROID -DPLATFORM_ANDROID -DRT_JPG_SUPPORT)

message("Building ${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
else()
    add_definitions(-DNDEBUG)
endif()


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.



add_library( # Sets the name of the library.
        RTAndroidApp

        # Sets the library as a shared library.
        SHARED


        ${SHARED}/PlatformSetup.cpp ${SHARED}/Android/AndroidApp.cpp ${SHARED}/android/AndroidUtils.cpp ${SHARED}/Audio/AudioManager.cpp ${SHARED}/Audio/AudioManagerAndroid.cpp
        ${CLANMATH}/angle.cpp ${CLANMATH}/mat3.cpp ${CLANMATH}/mat4.cpp ${CLANMATH}/rect.cpp ${CLANMATH}/vec2.cpp ${CLANMATH}/vec3.cpp ${CLANMATH}/vec4.cpp
        ${SHARED}/Entity/Entity.cpp ${SHARED}/Entity/Component.cpp ${SHARED}/GUI/RTFont.cpp ${SHARED}/Manager/Console.cpp ${SHARED}/FileSystem/FileManager.cpp
        ${SHARED}/Manager/GameTimer.cpp ${SHARED}/Manager/MessageManager.cpp ${SHARED}/Manager/ResourceManager.cpp ${SHARED}/Manager/VariantDB.cpp ${SHARED}/Math/rtPlane.cpp
        ${SHARED}/Math/rtRect.cpp ${SHARED}/Renderer/RenderBatcher.cpp ${SHARED}/Renderer/SoftSurface.cpp ${SHARED}/Renderer/Surface.cpp ${SHARED}/Renderer/SurfaceAnim.cpp
        ${SHARED}/util/CRandom.cpp ${SHARED}/util/GLESUtils.cpp ${SHARED}/util/MathUtils.cpp ${SHARED}/util/MiscUtils.cpp ${SHARED}/util/RenderUtils.cpp ${SHARED}/util/ResourceUtils.cpp
        ${SHARED}/util/Variant.cpp ${SHARED}/BaseApp.cpp ${SHARED}/FileSystem/FileSystem.cpp ${SHARED}/Entity/EntityUtils.cpp
        ${SHARED}/FileSystem/FileSystemZip.cpp ${SHARED}/util/unzip/unzip.c ${SHARED}/util/unzip/ioapi.c ${SHARED}/FileSystem/StreamingInstance.cpp
        ${SHARED}/FileSystem/StreamingInstanceZip.cpp ${SHARED}/FileSystem/StreamingInstanceFile.cpp ${SHARED}/Renderer/JPGSurfaceLoader.cpp

        ${COMPPATH}/FocusInputComponent.cpp ${COMPPATH}/FocusUpdateComponent.cpp ${COMPPATH}/FocusRenderComponent.cpp ${COMPPATH}/CustomInputComponent.cpp
        ${COMPPATH}/ArcadeInputComponent.cpp ${COMPPATH}/RectRenderComponent.cpp ${COMPPATH}/UnderlineRenderComponent.cpp ${COMPPATH}/TextBoxRenderComponent.cpp
        ${COMPPATH}/ScrollComponent.cpp ${COMPPATH}/ScrollBarRenderComponent.cpp ${COMPPATH}/FilterInputComponent.cpp ${COMPPATH}/RenderScissorComponent.cpp
        ${COMPPATH}/TyperComponent.cpp ${COMPPATH}/EmitVirtualKeyComponent.cpp ${COMPPATH}/SliderComponent.cpp ${COMPPATH}/OverlayRenderComponent.cpp
        ${COMPPATH}/TouchHandlerComponent.cpp ${COMPPATH}/Button2DComponent.cpp ${COMPPATH}/InterpolateComponent.cpp ${COMPPATH}/TextRenderComponent.cpp
        ${COMPPATH}/InputTextRenderComponent.cpp ${COMPPATH}/SelectButtonWithCustomInputComponent.cpp

        ${JPGSRC}/jcapimin.c ${JPGSRC}/jcapistd.c ${JPGSRC}/jccoefct.c ${JPGSRC}/jccolor.c ${JPGSRC}/jcdctmgr.c ${JPGSRC}/jchuff.c ${JPGSRC}/jcinit.c ${JPGSRC}/jcmainct.c
        ${JPGSRC}/jcmarker.c ${JPGSRC}/jcmaster.c ${JPGSRC}/jcomapi.c ${JPGSRC}/jcparam.c ${JPGSRC}/jcphuff.c ${JPGSRC}/jcprepct.c ${JPGSRC}/jcsample.c ${JPGSRC}/jctrans.c
        ${JPGSRC}/jdapimin.c ${JPGSRC}/jdapistd.c ${JPGSRC}/jdatadst.c ${JPGSRC}/jdatasrc.c ${JPGSRC}/jdcoefct.c ${JPGSRC}/jdcolor.c ${JPGSRC}/jddctmgr.c
        ${JPGSRC}/jdhuff.c ${JPGSRC}/jdinput.c ${JPGSRC}/jdmainct.c ${JPGSRC}/jdmarker.c ${JPGSRC}/jdmaster.c ${JPGSRC}/jdmerge.c ${JPGSRC}/jdphuff.c ${JPGSRC}/jdpostct.c
        ${JPGSRC}/jdsample.c ${JPGSRC}/jdtrans.c ${JPGSRC}/jerror.c ${JPGSRC}/jfdctflt.c ${JPGSRC}/jfdctfst.c ${JPGSRC}/jfdctint.c ${JPGSRC}/jidctflt.c ${JPGSRC}/jidctfst.c
        ${JPGSRC}/jidctint.c ${JPGSRC}/jidctred.c ${JPGSRC}/jmemmgr.c ${JPGSRC}/jmemnobs.c ${JPGSRC}/jquant1.c ${JPGSRC}/jquant2.c ${JPGSRC}/jutils.c

        ${APP}/App.cpp

        ${APP}/Component/EggManager.cpp ${APP}/Component/EggComponent.cpp ${APP}/Component/WolfComponent.cpp

        ${APP}/GUI/AboutMenu.cpp ${APP}/GUI/GameMenu.cpp ${APP}/GUI/GUIUtils.cpp ${APP}/GUI/LanguageSelectMenu.cpp ${APP}/GUI/MainMenu.cpp ${APP}/GUI/OptionsMenu.cpp
        ${APP}/GUI/OverlapTestMenu.cpp ${APP}/GUI/ThemeSelectMenu.cpp ${APP}/GUI/GameOverMenu.cpp ${APP}/GUI/PauseMenu.cpp ${APP}/GUI/TutorialMenu.cpp
        ${APP}/GUI/GamemodeSelectMenu.cpp

        ${APP}/Managers/ThemeManager.cpp ${APP}/Managers/TranslationManager.cpp
        )

# if you  need to add antoher C++ library, you can do it in the format below, then don't forget to add
# the lib name to the target_link_libraries command below as well
#add_library(lib_crypto STATIC IMPORTED)
#set_target_properties(lib_crypto PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/${SHARED}/android/optional_src/libs/libopenssl/${ANDROID_ABI}/libcrypto.a)

#add_library(lib_fmod STATIC IMPORTED)
#set_target_properties(lib_fmod PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/${SHARED}/android/optional_src/libs/fmodstudio/api/core/lib/${ANDROID_ABI}/libfmod.so)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log
        )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

#Keep in mind the order below matters!  Low level? Then it goes to the right

target_link_libraries( # Specifies the target library.
        RTAndroidApp

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} dl GLESv1_CM z) #add lib_fmod or whatever after z if you need it