# APEX.PropertyEditor
This project is using [SFML](https://github.com/SFML/SFML), [Dear ImGui](https://github.com/ocornut/imgui) and [ImGui-SFML](https://github.com/eliasdaler/imgui-sfml).

## Info
This program allows You to view and modify RTPC files extracted from games that are running on APEX Engine (theHunter: COTW, GenerationZero, possibly others (not tested :)). You can extract RTPC files using [DECA](https://github.com/kk49/deca).

Program can read all three versions of RTPC files, but writing modified version of it works only on version 2. Support for other versions will be added later.

## Releases
Compiled binaries can be found [here](https://github.com/CatShot112/APEX.PropertyEditor/releases).

## Build
You will need to download compiled SFML binaries from [here]() and put libraries in `C++/external/lib/x64/`. Then just open solution in Visual Studio 2022 and compile.

## Todo
- [ ] Implement serializer for RTPC version 1 and 3.
- [ ] Implement search properties function.
- [ ] Add ability to edit strings.

## Credits
[kk49](https://github.com/kk49) - For creating [((DE)C)onstructor for (A)pps (DECA)](https://github.com/kk49/deca)
