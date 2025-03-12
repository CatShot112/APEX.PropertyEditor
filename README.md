# APEX.PropertyEditor
This project is using [SFML](https://github.com/SFML/SFML), [Dear ImGui](https://github.com/ocornut/imgui) and [ImGui-SFML](https://github.com/eliasdaler/imgui-sfml).

## Info
This program allows You to view and modify RTPC files extracted from games that are running on APEX Engine (theHunter: COTW, GenerationZero, possibly others (not tested :)). You can extract RTPC files using [DECA](https://github.com/kk49/deca).

Program can read all three versions of RTPC files, but writing modified version of it works only on version 1 and 2. Support for other versions will be added later.

Please make sure that modified files are serialized properly by comparing them with originals using some hex editor. If they don't match (except modified stuff), please report an issue with game name and extracted file path and name, so I can investigate.

## Releases
Compiled binaries can be found [here](https://github.com/CatShot112/APEX.PropertyEditor/releases).

## Build
You will need to download compiled SFML binaries from [here](https://github.com/SFML/SFML/releases/tag/2.6.2) and put libraries in `C++/external/lib/x64/`. Then just open solution in Visual Studio 2022 and compile.

## Known issues
- After opening big file or file with a lot of nodes/props the program is slow AF.
- Program can produce corrupted files when saving.
- ???

## Todo
- [ ] Implement serializer for RTPC version 3 (WIP).
- [ ] Implement search properties function.
- [ ] Add ability to edit strings and shared values (WIP).
- [ ] Everything else :P

## Credits
[kk49](https://github.com/kk49) - For creating [((DE)C)onstructor for (A)pps (DECA)](https://github.com/kk49/deca)
