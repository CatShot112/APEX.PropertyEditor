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
bool shortcutEnabled = false;

bool showAbout = false;
bool showSearch = false;

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

void InitHashMap() {
    std::ifstream file("property_list_2.txt");
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
                        if (fs::file_size(currentFileName) > 204800) // 200 Kb
                            printf("[WAR]: Opening big file!\n");

                        rtpcFile.Clear();
                        ProcessRTPC(currentFileName);

                        showAll = false;
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

            ImGuiInputTextFlags isReadOnly = rtpcFile.Version == 3 ? ImGuiInputTextFlags_ReadOnly : 0;

            // U32
            if (node.props[i].Type == 1) {
                ImGui::InputInt("##value", (int*)&node.props[i].DataRaw);
            }
            // F32
            else if (node.props[i].Type == 2) {
                ImGui::InputFloat("##value", (float*)&node.props[i].DataRaw);
            }
            // Str
            else if (node.props[i].Type == 3) {
                ImGui::InputText("##value", &node.props[i].DataStr, ImGuiInputTextFlags_ReadOnly);
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
                ImGui::InputFloat3("##value", (float*)&node.props[i].DataFinal[0], "%.3f", isReadOnly);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat3("##value", (float*)&node.props[i].DataFinal[12], "%.3f", isReadOnly);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat3("##value", (float*)&node.props[i].DataFinal[24], "%.3f", isReadOnly);
            }
            // Mat4x4
            else if (node.props[i].Type == 8) {
                ImGui::InputFloat4("##value", (float*)&node.props[i].DataFinal[0], "%.3f", isReadOnly);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat4("##value", (float*)&node.props[i].DataFinal[16], "%.3f", isReadOnly);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat4("##value", (float*)&node.props[i].DataFinal[32], "%.3f", isReadOnly);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat4("##value", (float*)&node.props[i].DataFinal[48], "%.3f", isReadOnly);
            }
            // A[U32]
            else if (node.props[i].Type == 9) {
                if (node.props[i].DataFinal.GetSize())
                    ImGui::InputScalarN("##value", ImGuiDataType_U32, &node.props[i].DataFinal[0], (int)node.props[i].DataFinal.GetSize() / sizeof(u32), 0, 0, 0, isReadOnly);
                else
                    ImGui::Text("Empty array of u32.");
            }
            // A[F32]
            else if (node.props[i].Type == 10) {
                if (node.props[i].DataFinal.GetSize())
                    ImGui::InputScalarN("##value", ImGuiDataType_Float, &node.props[i].DataFinal[0], (int)node.props[i].DataFinal.GetSize() / sizeof(float), 0, 0, 0, isReadOnly);
                else
                    ImGui::Text("Empty array of f32.");
            }
            // A[U8]
            else if (node.props[i].Type == 11) {
                if (node.props[i].DataFinal.GetSize())
                    ImGui::InputScalarN("##value", ImGuiDataType_U8, &node.props[i].DataFinal[0], (int)node.props[i].DataFinal.GetSize() / sizeof(u8), 0, 0, 0, isReadOnly);
                else
                    ImGui::Text("Empty array of u8.");
            }
            // ObjID
            else if (node.props[i].Type == 13) {
                ImGui::InputScalar("##value", ImGuiDataType_U64, &node.props[i].DataFinal[0], 0, 0, 0, isReadOnly);
            }
            // Event
            else if (node.props[i].Type == 14) {
                if (node.props[i].DataFinal.GetSize())
                    ImGui::InputScalarN("##value", ImGuiDataType_U64, &node.props[i].DataFinal[0], (int)node.props[i].DataFinal.GetSize() / sizeof(u64), 0, 0, 0, isReadOnly);
                else
                    ImGui::Text("Empty event.");
            }

            // Show tooltips
            if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > 0.25f) {
                if (node.props[i].Type == 3) {
                    ImGui::SetTooltip("Strings cannot be modified yet.");
                }
                else if (node.props[i].IsShared) {
                    ImGui::SetTooltip("Shared values connot be modified yet.");
                }
                else if (rtpcFile.Version == 3 && node.props[i].Type != 1 && node.props[i].Type != 2) {
                    ImGui::SetTooltip("For now, only basic values can be modified in this version of RTPC (v3).");
                }
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
                    if (fs::file_size(currentFileName) > 204800) // 200 Kb
                        printf("[WAR]: Opening big file!\n");

                    rtpcFile.Clear();
                    ProcessRTPC(currentFileName);

                    showAll = true;
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
                            rtpcFile.Clear();
                            ProcessRTPC(recentFiles[i]);

                            showAll = false;
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
            if (ImGui::MenuItem("Colors", "", nullptr, false))
            {

            }

            if (ImGui::MenuItem("Settings", "", nullptr, false))
            {

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

            if (ImGui::MenuItem("Console", "", nullptr, false))
            {

            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void DrawAbout() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("About", &showAbout, flags))
    {
        if (ImGui::BeginTabBar("About"))
        {
            if (ImGui::BeginTabItem("Contributors")) {
                ImGui::BulletText("CatShot112");

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Libraries")) {
                ImGui::BulletText("ImGui");
                ImGui::BulletText("ImGui-SFML");
                ImGui::BulletText("SFML");

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("License")) {
                ImGui::Text("LICENSE");

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}

void DrawSearch() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Search", &showSearch, flags)) {
        ImGui::Checkbox("Enable", &searchOn);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##value", &searchStr);

        ImGui::End();
    }
}

int WINAPI WinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance, _In_ LPSTR cmdLine, _In_ int cmdShow) {
#ifndef _DEBUG
    InitHashMap();
#endif

    sf::RenderWindow window(sf::VideoMode(1280, 720), "APEX.PropertyEditor");
    sf::Clock clock;

    window.setVerticalSyncEnabled(false);
    window.setFramerateLimit(60);

    ImGui::SFML::Init(window);

    while (window.isOpen()) {
        sf::Event e;

        while (window.pollEvent(e)) {
            ImGui::SFML::ProcessEvent(window, e);

            if (e.type == sf::Event::Closed)
                window.close();
        }

        ImGui::SFML::Update(window, clock.restart());
        ImGui::DockSpaceOverViewport();

        DrawMainMenuBar(window);
        DrawPropertyEditor(nullptr);

        if (showAbout)
            DrawAbout();
        if (showSearch)
            DrawSearch();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}

#ifdef _DEBUG
int main(int argc, char** argv) {
    return WinMain(GetModuleHandleA(nullptr), nullptr, GetCommandLineA(), SW_SHOWNORMAL);
}
#endif
