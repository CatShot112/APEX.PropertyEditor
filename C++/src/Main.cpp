// TODO: Add 'Log' class.

// https://github.com/tim42/gibbed-justcause3-tools-fork/blob/master/Gibbed.JustCause3.ConvertProperty/Program.cs :)

#include "../headers/imgui/imgui.h"
#include "../headers/imgui/imgui_internal.h"
#include "../headers/imgui/imgui-SFML.h"
#include "../headers/imgui/imgui_stdlib.h"
#include "../headers/jenkins/hashlittle.h"

#include "Rtpc/RtpcFile.hpp"
#include "FileSystem/FileSystem.hpp"

// SFML includes
#include <SFML/Graphics.hpp>

// Windows includes
#include <Windows.h>

// Std library includes
#include <unordered_map>
#include <filesystem>

#ifdef _DEBUG
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-graphics-d.lib")
#else
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-graphics.lib")
#endif

#pragma comment(lib, "opengl32.lib")

namespace fs = std::filesystem;
using Kbd = sf::Keyboard;

// Global variables
RtpcFile rtpcFile;
std::string currentFileName;

bool searchOn = false;
bool showAll = false;
bool fileOpened = false;
bool fileBig = false;
bool shortcutEnabled = false;

bool showAbout = false;
bool showSearch = false;
bool showSettings = false;
bool showStyleEditor = false;
bool showLog = false;

std::string searchStr;

std::unordered_map<u32, string> hashmap;
std::unordered_map<u8, string> propTypeNames = {
    {0, "none"},
    {1, "u32"},
    {2, "f32"},
    {3, "str"},
    {4, "vec2"},
    {5, "vec3"},
    {6, "vec4"},
    {7, "mat3x3"},
    {8, "mat4x4"},
    {9, "a[u32]"},
    {10, "a[f32]"},
    {11, "a[u8]"},
    {12, "d12"},
    {13, "objid"},
    {14, "event"}
};

std::vector<string> recentFiles;

sf::RenderWindow window;

namespace Settings {
    static int fpsLimitType = 0;
    static unsigned int fpsLimitFocused = 60;
    static unsigned int fpsLimitBackground = 15;

    // Restart required
    static int loggingType = 0;
    static std::string logPath = fs::current_path().generic_string() + "/Log.txt";

    void Apply() {
        if (fpsLimitType == 0) {
            window.setVerticalSyncEnabled(true);
            window.setFramerateLimit(0);
        }
        else if (fpsLimitType == 1) {
            window.setVerticalSyncEnabled(false);
            window.setFramerateLimit(fpsLimitFocused);
        }
    }
}

void LoadRecentFiles() {
    std::ifstream file(".history", std::ios::in);
    if (!file.is_open()) {
        printf("[WAR]: File history doesn't exist!\n");
        return;
    }

    string line;

    while (std::getline(file, line)) {
        recentFiles.emplace_back(line);
    }

    file.close();
}
void SaveRecentFiles() {
    std::ofstream file(".history", std::ios::out);
    if (!file.is_open()) {
        printf("[WAR]: Failed to open file to write!\n");
        return;
    }

    for (auto& s : recentFiles) {
        file << s << std::endl;
    }

    file.close();
}

void InitHashMap() {
    std::ifstream file("property_list.txt");
    if (!file.is_open())
        return;

    string line;
    while (std::getline(file, line)) {
        if (line.empty() || line.size() <= 0 || line[0] == ' ')
            continue;

        hashmap[hashlittle(line.c_str(), strlen(line.c_str()), 0)] = line;
    }
}

void DehashNames(RtpcNode& node) {
    // Dehash current node name
    if (hashmap.count(node.HashedName))
        node.DehashedName = hashmap[node.HashedName];

    // Dehash current node props names
    for (u16 i = 0; i < node.PropsCount; i++) {
        if (hashmap.count(node.props[i].HashedName))
            node.props[i].DehashedName = hashmap[node.props[i].HashedName];
    }

    // Dehash child nodes names
    for (u16 i = 0; i < node.ChildCount; i++)
        DehashNames(node.childs[i]);
}

void ProcessRTPC(std::wstring fileName) {
    std::ifstream file(fileName, std::ios::binary);

    if (!file.is_open()) {
        printf("[ERRO]: Failed to open RTPC file!\n");
        return;
    }

    if (!rtpcFile.Deserialize(file)) {
        printf("[ERRO]: Failed to process RTPC file!\n");
        file.close();
        return;
    }

    file.close();

    DehashNames(rtpcFile.mainNode);
}

void ProcessRTPC(std::string fileName) {
    std::ifstream file(fileName, std::ios::binary);

    if (!file.is_open()) {
        printf("[ERRO]: Failed to open RTPC file!\n");
        return;
    }

    if (!rtpcFile.Deserialize(file)) {
        printf("[ERRO]: Failed to process RTPC file!\n");
        file.close();
        return;
    }

    file.close();

    DehashNames(rtpcFile.mainNode);
}

void ProcessShortcuts() {
    if (ImGui::IsWindowFocused()) {
        if (!shortcutEnabled) {
            shortcutEnabled = true;

            // File menu
            {
                // Open
                if (Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::O)) {
                    if (FileSystem::OpenFileDialog(currentFileName)) {
                        // TODO: Block execution and let the user decide if to proceed.
                        if (fs::file_size(currentFileName) > 204800) { // 200 Kb
                            printf("[WAR]: Opening big file!\n");
                            fileBig = true;
                        }

                        rtpcFile.Clear();
                        ProcessRTPC(currentFileName);

                        showAll = (fileBig) ? false : true;
                        fileOpened = true;
                    }
                }
                // Save
                else if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::S)) {
                    writtenStrings.clear();

                    std::ofstream file(currentFileName, std::ios::binary);
                    rtpcFile.Serialize(file);
                    file.close();
                }
                // Save As..
                else if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::LShift) && Kbd::isKeyPressed(Kbd::S)) {
                    if (FileSystem::SaveFileDialog(currentFileName)) {
                        writtenStrings.clear();

                        std::ofstream file(currentFileName, std::ios::binary);
                        rtpcFile.Serialize(file);
                        file.close();
                    }
                }
                // Reload
                else if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::R)) {
                    rtpcFile.Clear();
                    ProcessRTPC(currentFileName);

                    showAll = false;
                    fileOpened = true;
                }
                // Rename
                else if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::LShift) && Kbd::isKeyPressed(Kbd::R)) {
                    // TODO: Open rename file dialog.
                }
                // Close
                else if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::Q)) {
                    rtpcFile.Clear();

                    showAll = false;
                    fileOpened = false;
                }
            }

            // Edit menu
            {
                // Undo
                if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::Z)) {
                    // Not needed? :D
                }
                // Redo
                else if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::Y)) {
                    // Not needed? :D
                }
                // Cut
                else if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::X)) {
                    // Not needed? :D
                }
                // Copy
                else if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::C)) {
                    // Not needed? :D
                }
                // Paste
                else if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::V)) {
                    // Not needed? :D
                }
                // Search
                else if (fileOpened && Kbd::isKeyPressed(Kbd::LControl) && Kbd::isKeyPressed(Kbd::F)) {
                    showSearch = !showSearch;
                }
            }

            shortcutEnabled = false;
        }
    }
}

// GUI
void DrawNode(RtpcNode& node) {
    ImGui::PushID(&node);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();

    if (showAll)
        ImGui::SetNextItemOpen(true);

    bool nodeOpen = false;

    // Try to find dehashed name for node
    if (node.DehashedName.length() > 0)
        nodeOpen = ImGui::TreeNode("", "%s", node.DehashedName.c_str());
    else
        nodeOpen = ImGui::TreeNode("", "%08X", node.HashedName);

    ImGui::TableSetColumnIndex(2);

    if (nodeOpen) {
        for (int i = 0; i < node.PropsCount; i++) {
            // Skip null props
            if (node.props[i].HashedName == 0)
                continue;

            if (searchOn && node.props[i].DehashedName.find(searchStr) == std::string::npos)
                continue;

            ImGui::PushID(&node.props[i]);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

            // Try to find dehashed name for property
            if (node.props[i].DehashedName.length() > 0)
                ImGui::TreeNodeEx("", flags, "%s", node.props[i].DehashedName.c_str());
            else
                ImGui::TreeNodeEx("", flags, "%08X", node.props[i].HashedName);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", propTypeNames[node.props[i].Type].c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-FLT_MIN);

            //ImGuiInputTextFlags isReadOnly = rtpcFile.Version == 3 ? ImGuiInputTextFlags_ReadOnly : 0;
            ImGuiInputTextFlags isReadOnly = 0;

            // U32
            if (node.props[i].Type == 1) {
                ImGui::InputScalar("##value", ImGuiDataType_U32, (int*)&node.props[i].DataRaw);
            }
            // F32
            else if (node.props[i].Type == 2) {
                ImGui::InputFloat("##value", (float*)&node.props[i].DataRaw);
            }
            // Str
            else if (node.props[i].Type == 3) {
                ImGui::InputText("##value", &node.props[i].DataStr, isReadOnly);
            }
            // Vec2
            else if (node.props[i].Type == 4) {
                ImGui::InputFloat2("##value", (float*)&node.props[i].DataFinal[0], "%.3f", isReadOnly);
            }
            // Vec3
            else if (node.props[i].Type == 5) {
                ImGui::InputFloat3("##value", (float*)&node.props[i].DataFinal[0], "%.3f", isReadOnly);
            }
            // Vec4
            else if (node.props[i].Type == 6) {
                ImGui::InputFloat4("##value", (float*)&node.props[i].DataFinal[0], "%.3f", isReadOnly);
            }
            // Mat3x3
            else if (node.props[i].Type == 7) {
                ImGui::InputFloat3("##value0", (float*)&node.props[i].DataFinal[0], "%.3f", isReadOnly);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat3("##value1", (float*)&node.props[i].DataFinal[12], "%.3f", isReadOnly);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat3("##value2", (float*)&node.props[i].DataFinal[24], "%.3f", isReadOnly);
            }
            // Mat4x4
            else if (node.props[i].Type == 8) {
                ImGui::InputFloat4("##value0", (float*)&node.props[i].DataFinal[0], "%.3f", isReadOnly);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat4("##value1", (float*)&node.props[i].DataFinal[16], "%.3f", isReadOnly);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat4("##value2", (float*)&node.props[i].DataFinal[32], "%.3f", isReadOnly);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat4("##value3", (float*)&node.props[i].DataFinal[48], "%.3f", isReadOnly);
            }
            // A[U32] TODO: Auto calculate number of columns.
            else if (node.props[i].Type == 9) {
                if (node.props[i].DataFinal.GetSize()) {
                    int num = (int)node.props[i].DataFinal.GetSize() / sizeof(u32);

                    if (num <= 8) {
                        ImGui::InputScalarN("##value", ImGuiDataType_U32, &node.props[i].DataFinal[0], num, 0, 0, 0, isReadOnly);
                    }
                    else {
                        int a = num / 8;
                        int b = num % 8;

                        for (int ii = 0; ii < a; ii++) {
                            ImGui::InputScalarN(string("##value" + std::to_string(ii)).c_str(), ImGuiDataType_U32, &node.props[i].DataFinal[ii * sizeof(u32) * 8], 8, 0, 0, 0, isReadOnly);
                            ImGui::SetNextItemWidth(-FLT_MIN);
                        }

                        ImGui::InputScalarN("##valueLast", ImGuiDataType_U32, &node.props[i].DataFinal[a * sizeof(u32) * 8], b, 0, 0, 0, isReadOnly);
                        ImGui::Separator();
                    }
                }
                else {
                    ImGui::Text("Empty array of u32.");
                }
            }
            // A[F32]
            else if (node.props[i].Type == 10) {
                if (node.props[i].DataFinal.GetSize()) {
                    int num = (int)node.props[i].DataFinal.GetSize() / sizeof(f32);

                    if (num <= 8) {
                        ImGui::InputScalarN("##value", ImGuiDataType_Float, &node.props[i].DataFinal[0], num, 0, 0, 0, isReadOnly);
                    }
                    else {
                        int a = num / 8;
                        int b = num % 8;

                        for (int ii = 0; ii < a; ii++) {
                            ImGui::InputScalarN(string("##value" + std::to_string(ii)).c_str(), ImGuiDataType_Float, &node.props[i].DataFinal[ii * sizeof(f32) * 8], 8, 0, 0, 0, isReadOnly);
                            ImGui::SetNextItemWidth(-FLT_MIN);
                        }

                        ImGui::InputScalarN("##valueLast", ImGuiDataType_Float, &node.props[i].DataFinal[a * sizeof(f32) * 8], b, 0, 0, 0, isReadOnly);
                    }

                    ImGui::Separator();
                }
                else {
                    ImGui::Text("Empty array of f32.");
                }
            }
            // A[U8]
            else if (node.props[i].Type == 11) {
                if (node.props[i].DataFinal.GetSize()) {
                    int num = (int)node.props[i].DataFinal.GetSize() / sizeof(u32);

                    if (num <= 16) {
                        ImGui::InputScalarN("##value", ImGuiDataType_U8, &node.props[i].DataFinal[0], num, 0, 0, 0, isReadOnly);
                    }
                    else {
                        int a = num / 16;
                        int b = num % 16;

                        for (int ii = 0; ii < a; ii++) {
                            ImGui::InputScalarN(string("##value" + std::to_string(ii)).c_str(), ImGuiDataType_U8, &node.props[i].DataFinal[ii * sizeof(u8) * 16], 16, 0, 0, 0, isReadOnly);
                            ImGui::SetNextItemWidth(-FLT_MIN);
                        }

                        ImGui::InputScalarN("##valueLast", ImGuiDataType_U8, &node.props[i].DataFinal[a * sizeof(u8) * 16], b, 0, 0, 0, isReadOnly);
                    }

                    ImGui::Separator();
                }
                else {
                    ImGui::Text("Empty array of u8.");
                }
            }
            // ObjID
            else if (node.props[i].Type == 13) {
                ImGui::InputScalar("##value", ImGuiDataType_U64, &node.props[i].DataFinal[0], 0, 0, 0, isReadOnly);
            }
            // Event (aka. a[u64])
            else if (node.props[i].Type == 14) {
                if (node.props[i].DataFinal.GetSize()) {
                    int num = (int)node.props[i].DataFinal.GetSize() / sizeof(u64);

                    if (num <= 6) {
                        ImGui::InputScalarN("##value", ImGuiDataType_U64, &node.props[i].DataFinal[0], num, 0, 0, 0, isReadOnly);
                    }
                    else {
                        int a = num / 6;
                        int b = num % 6;

                        for (int ii = 0; ii < a; ii++) {
                            ImGui::InputScalarN(string("##value" + std::to_string(ii)).c_str(), ImGuiDataType_U64, &node.props[i].DataFinal[ii * sizeof(u64) * 6], 6, 0, 0, 0, isReadOnly);
                            ImGui::SetNextItemWidth(-FLT_MIN);
                        }

                        ImGui::InputScalarN("##valueLast", ImGuiDataType_U64, &node.props[i].DataFinal[a * sizeof(u64) * 6], b, 0, 0, 0, isReadOnly);
                    }

                    ImGui::Separator();
                }
                else {
                    ImGui::Text("Empty event.");
                }
            }

            // Show tooltips
            if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.25f) {

            }

            ImGui::NextColumn();
            ImGui::PopID();
        }

        for (int i = 0; i < node.ChildCount; i++)
            DrawNode(node.childs[i]);

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void DrawPropertyEditor(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Property editor", open)) {
        ImGui::End();
        return;
    }

    ProcessShortcuts();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

    if (ImGui::BeginTable("split", 3, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable)) {
        DrawNode(rtpcFile.mainNode);

        ImGui::EndTable();
    }

    if (showAll)
        showAll = false;

    ImGui::PopStyleVar();
    ImGui::End();
}

void DrawMainMenuBar(sf::RenderWindow& window) {
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "CTRL+O"))
            {
                if (FileSystem::OpenFileDialog(currentFileName)) {
                    // TODO: Block execution and let the user decide if to proceed.
                    if (fs::file_size(currentFileName) > 204800) { // 200 Kb
                        printf("[WAR]: Opening big file!\n");
                        fileBig = true;
                    }

                    rtpcFile.Clear();
                    ProcessRTPC(currentFileName);

                    if (std::find(recentFiles.begin(), recentFiles.end(), currentFileName) == recentFiles.end())
                        recentFiles.emplace_back(currentFileName);

                    showAll = (fileBig) ? false : true;
                    fileOpened = true;
                }
            }

            if (ImGui::BeginMenu("Open Recent"))
            {
                if (recentFiles.size() <= 0) {
                    ImGui::Text("No History Yet..");
                }
                else {
                    for (size_t i = 0; i < recentFiles.size(); i++) {
                        if (i >= 10)
                            break;

                        if (ImGui::MenuItem(recentFiles[i].c_str())) {
                            if (fs::file_size(recentFiles[i]) > 204800) {
                                printf("[WAR]: Opening big file!\n");
                                fileBig = true;
                            }

                            rtpcFile.Clear();
                            ProcessRTPC(recentFiles[i]);

                            showAll = (fileBig) ? false : true;
                            fileOpened = true;
                        }
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save", "CTRL+S", nullptr, fileOpened))
            {
                writtenStrings.clear();

                std::ofstream file(currentFileName, std::ios::binary);
                rtpcFile.Serialize(file);
                file.close();
            }

            if (ImGui::MenuItem("Save As..", "CTRL+SHIFT+S", nullptr, fileOpened))
            {
                if (FileSystem::SaveFileDialog(currentFileName)) {
                    writtenStrings.clear();

                    std::ofstream file(currentFileName, std::ios::binary);
                    rtpcFile.Serialize(file);
                    file.close();
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Reload", "CTRL+R", nullptr, fileOpened))
            {
                rtpcFile.Clear();
                ProcessRTPC(currentFileName);

                showAll = false;
                fileOpened = true;
            }

            if (ImGui::MenuItem("Rename", "CTRL+SHIFT+R", nullptr, false))
            {

            }

            if (ImGui::MenuItem("Close", "CTRL+Q", nullptr, fileOpened))
            {
                rtpcFile.Clear();

                showAll = false;
                fileOpened = false;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                window.close();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTLR+Z", nullptr, fileOpened))
            {

            }

            if (ImGui::MenuItem("Redo", "CTRL+Y", nullptr, fileOpened))
            {

            }

            ImGui::Separator();

            if (ImGui::MenuItem("Cut", "CTRL+X", nullptr, fileOpened))
            {

            }

            if (ImGui::MenuItem("Copy", "CTRL+C", nullptr, fileOpened))
            {

            }

            if (ImGui::MenuItem("Paste", "CTRL+V", nullptr, fileOpened))
            {

            }

            ImGui::Separator();

            if (ImGui::MenuItem("Search", "CTRL+F", nullptr, fileOpened))
            {
                showSearch = true;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Options"))
        {
            if (ImGui::MenuItem("Settings", "", nullptr, true))
            {
                showSettings = true;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                showAbout = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Log", "", nullptr, true))
            {
                showLog = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void DrawAbout() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;

    if (ImGui::Begin("About", &showAbout, flags))
    {
        if (ImGui::BeginTabBar("About"))
        {
            if (ImGui::BeginTabItem("Contributors")) {
                ImGui::BulletText("CatShot112");

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Libraries")) {
                ImGui::TextLinkOpenURL("ImGui", "https://github.com/ocornut/imgui");
                ImGui::TextLinkOpenURL("ImGui-SFML", "https://github.com/SFML/imgui-sfml");
                ImGui::TextLinkOpenURL("SFML", "https://github.com/SFML/SFML");

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("License")) {
                ImGui::Text("LICENSE");

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }

        ImGui::End();
    }

void DrawSearch() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;

    if (ImGui::Begin("Search", &showSearch, flags)) {
        ImGui::Checkbox("Enable", &searchOn);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##value", &searchStr);
    }

    ImGui::End();
}

void TextCentered(std::string text) {
    auto widthW = ImGui::GetWindowSize().x;
    auto widthT = ImGui::CalcTextSize(text.c_str()).x;

    ImGui::SetCursorPosX((widthW - widthT) * 0.5f);
    ImGui::Text(text.c_str());
}

void DrawSettings() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Settings", &showSettings, flags)) {
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::Button("Apply Settings"))
            Settings::Apply();

        TextCentered("Fps Limit Type");
        ImGui::Separator();
        ImGui::RadioButton("V-SYNC", &Settings::fpsLimitType, 0);
        ImGui::RadioButton("FPS Limit", &Settings::fpsLimitType, 1);

        if (Settings::fpsLimitType == 0)
            ImGui::BeginDisabled();
        ImGui::InputScalar("Limit Focused", ImGuiDataType_U32, &Settings::fpsLimitFocused);
        ImGui::InputScalar("Limit Background", ImGuiDataType_U32, &Settings::fpsLimitBackground);
        if (Settings::fpsLimitType == 0)
            ImGui::EndDisabled();

        ImGui::Separator();

        TextCentered("Logging");
        ImGui::Separator();
        ImGui::RadioButton("Normal", &Settings::loggingType, 1);
        ImGui::SameLine();
        ImGui::RadioButton("In-Memory only", &Settings::loggingType, 2);
        ImGui::SetItemTooltip("!!!WARNING!!!\n\nLog will be visible under Help->Log but WON'T BE SAVED TO DISK!!\nDATA WILL BE LOST UNLESS MANUALLY SAVED!!");
        ImGui::SameLine();
        ImGui::RadioButton("Disabled", &Settings::loggingType, 0);
        if (Settings::loggingType != 1)
            ImGui::BeginDisabled();
        ImGui::Text("Log Path");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##", &Settings::logPath);
        if (Settings::loggingType != 1)
            ImGui::EndDisabled();

        if (ImGui::Button("ImGui Style Editor"))
            showStyleEditor = true;
    }

    ImGui::End();
}

void DrawLog() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Log", &showLog, flags)) {
        ImGui::Text("Framerate: %0.3f", ImGui::GetIO().Framerate);
        
    }

    ImGui::End();
}

void DrawStyleEditor() {
    if (ImGui::Begin("Dear ImGui Style Editor", &showStyleEditor))
        ImGui::ShowStyleEditor();

        ImGui::End();
    }

// https://gist.github.com/FRex/3f7b8d1ad1289a2117553ff3702f04af

LONG_PTR originalWndProc = 0;

LRESULT CALLBACK WndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_DROPFILES) {
        POINT point{};
        HDROP hDrop = reinterpret_cast<HDROP>(wParam);

        DragQueryPoint(hDrop, &point);

        auto filesCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
        if (filesCount > 1) {
            printf("Only 1 file can be opened at once!\n");
            DragFinish(hDrop);
            return CallWindowProc(reinterpret_cast<WNDPROC>(originalWndProc), handle, message, wParam, lParam);
        }

        auto bufSize = DragQueryFile(hDrop, 0, nullptr, 0) + 1;
        std::string buf;
        buf.resize(bufSize);

        if (DragQueryFile(hDrop, 0, &buf[0], bufSize)) {
            currentFileName = buf;

            if (!fs::is_directory(currentFileName)) {
                if (fs::file_size(currentFileName) > 204800) { // 200 Kb
                    printf("[WAR]: Opening big file!\n");
                    fileBig = true;
                }

                rtpcFile.Clear();
                ProcessRTPC(currentFileName);

                showAll = (fileBig) ? false : true;
                fileOpened = true;
            }
            else {
                printf("[ERR]: Dragged folder!\n");
            }
        }

        DragFinish(hDrop);
    }

            return CallWindowProc(reinterpret_cast<WNDPROC>(originalWndProc), handle, message, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance, _In_ LPSTR cmdLine, _In_ int cmdShow) {
#ifndef _DEBUG
    InitHashMap();
#endif

    //InitHashMap();
    LoadRecentFiles();

    sf::RenderWindow window(sf::VideoMode(1280, 720), "APEX.PropertyEditor");
    sf::Clock clock;

    window.create(sf::VideoMode(1280, 720), "APEX.PropertyEditor");
    window.setVerticalSyncEnabled(false);
    window.setFramerateLimit(60);

    DragAcceptFiles(window.getSystemHandle(), true);
    originalWndProc = SetWindowLongPtr(window.getSystemHandle(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));

    ImGui::SFML::Init(window);

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    while (window.isOpen()) {
        sf::Event e;

        while (window.pollEvent(e)) {
            ImGui::SFML::ProcessEvent(window, e);

            if (e.type == sf::Event::Closed)
                window.close();

            if (e.type == sf::Event::GainedFocus)
                if (Settings::fpsLimitType == 1)
                    window.setFramerateLimit(Settings::fpsLimitFocused);

            if (e.type == sf::Event::LostFocus)
                if (Settings::fpsLimitType == 1)
                    window.setFramerateLimit(Settings::fpsLimitBackground);
        }

        ImGui::SFML::Update(window, clock.restart());
        ImGui::DockSpaceOverViewport();

        DrawMainMenuBar(window);
        DrawPropertyEditor(nullptr);
        //ImGui::ShowDemoWindow();

        // Edit
        if (showSearch)
            DrawSearch();

        // Options
        if (showSettings)
            DrawSettings();
        if (showStyleEditor)
            DrawStyleEditor();

        // Help
        if (showAbout)
            DrawAbout();
        if (showLog)
            DrawLog();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    SaveRecentFiles();
    ImGui::SFML::Shutdown();

    return 0;
}

#ifdef _DEBUG
int main(int argc, char** argv) {
    return WinMain(GetModuleHandleA(nullptr), nullptr, GetCommandLineA(), SW_SHOWNORMAL);
}
#endif
