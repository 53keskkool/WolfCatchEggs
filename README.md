# WolfCatchEggs
A game where you catch stuff falling at you. This game was made as a part of a research work.


# How to build?
First of all, you need to get [Proton SDK](https://github.com/SethRobinson/proton) and put this repository into it. Then you need to go to `media` folder and run `update_media.bat`. 
After that everything is platform specific.
## Windows
1. Go to `windows_vc` folder and open `WolfCatchEggs.sln` in Visual Studio.
2. Choose build type and architecture you need (usually `Release_GL` and `x64` are fine).
3. Click `Build` and `Build Solution` or `Start Without Debugging` (Ctrl+F5).
4. If everything goes right, you will find built executable at `bin`.

## Android
1. Open folder named `AndroidGradle` in Android Studio.
2. Add `RTappID` to your `local.properties` file. (You can see an example at `local.properties_removethispart_`)
3. Click `Build` and `Make Project` (or Ctrl+F9).
4. If everything goes right, you will find built APK at `AndroidGradle/app/build/outputs/apk`.

## HTML5
1. [Install Emscripten](https://emscripten.org/docs/getting_started/downloads.html#installation-instructions-using-the-emsdk-recommended).
2. Edit file `base_setup.bat` in root of Proton SDK. (You just need to set `EMSCRIPTEN_ROOT`)
3. Go to `html5` and run `build_release.bat`.
4. Put resulting files (`WolfCatchEggs.*` and `WebLoaderData`) on your HTTP server.

## Linux / OS X / iOS / PS Vita
I didn't make these work. Maybe I will work on these later.