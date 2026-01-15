This is a patch on `app.dlc.DlcService::isAvailableDLC(app.dlc.DlcProductId.ID)` to cache the results.
And a patch on `app.GUIManager::isNewBenefit()` to always return false. (This should make the many exclamation point checks the DLC cat (Support Desk) does much faster. Basically no DLC will appear as a new exclamation point.)

Here's a native plugin (put in `reframework\plugins\`).<br>
This will just do a very simple cache of the function result so it doesn't have to call over and over.<br>
This will work with your current DLC without granting you all the DLC in the world.<br>

PS: If it isn't obvs. to you, using this means you have to restart the game on purchasing DLC for the new stuff to be found.

# Compiling
GLHF, this isn't super easy.
- Clone https://github.com/Synthlight/Base-Dll-Proxy so it's at the same dir level as this project. (i.e., `../Base-Dll-Proxy` from this project needs to work.)
- Build safetyhook. Here's how I did it:
  - Make some new dir. I'm gonna call it `safetyhook-zydis` as that's what I did, but it doesn't matter what or where. (For best results, use a path with no spaces.)
  - In `safetyhook-zydis`:
  - Clone https://github.com/cursey/safetyhook as `safetyhook`.
  - Clone https://github.com/zyantific/zydis as `zydis`.
  - Create a `CMakeLists.txt` with this contents:

        ```cmake
        cmake_minimum_required(VERSION 3.15)

        add_subdirectory(zydis)
        add_subdirectory(safetyhook)
        ```
  - Run:
    - `cmake -B build -G "Visual Studio 17 2022" -A x64`
    - `cmake --build build --config Debug --target ALL_BUILD`
    - `cmake --build build --config Release --target ALL_BUILD`
  - Make note of where the output `lib` files wind up. (Well, where the `build` folder is, specifically, as the rest shouldn't change.)
- Edit `DLC-Check-Performance-Patch.vcxproj`
  - Replace all occurences of `R:\Code\safetyhook-zydis\build` with wherever your output build dir stuff is.
  - Alternatively, cahnge all the safetyhook/zydis debug/release references to wherever your respective `lib` files wound up.
- Open `DLC-Check-Performance-Patch.sln` in Visual Studio:
  - Edit the project post-build step as I have this set to copy into the MHWilds dir, which will not match yours unless you have it installed in a Steam lib on the `O:` drive.
- Build debug or release.