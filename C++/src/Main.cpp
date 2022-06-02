#include "../headers/imgui/imgui.h"
#include "../headers/imgui/imgui-SFML.h"
#include "../headers/imgui/imgui_stdlib.h"
#include "../headers/jenkins/hashlittle.h"

#include "rtpc/RtpcFile.hpp"

// SFML includes
#include <SFML/Graphics.hpp>

// Windows includes
#include <Windows.h>
#include <ShObjIdl.h>

// Std library includes
#include <unordered_map>

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

// Global variables
RtpcFile rtpcFile;
std::wstring currentFileName;

bool searchOn = false;
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

void DehashNames(RtpcNode& node) {
    // Dehash current node name
    if (hashmap.count(node.HashedName))
        node.DehashedName = hashmap[node.HashedName];

    // Dehash current node prop names
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

void ShowNode(RtpcNode& node, bool showAll) {
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
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;

            // Try to find dehashed name for property
            if (node.props[i].DehashedName.length() > 0)
                ImGui::TreeNodeEx("", flags, "%s", node.props[i].DehashedName.c_str());
            else
                ImGui::TreeNodeEx("", flags, "%08X", node.props[i].HashedName);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text(propTypeNames[node.props[i].Type].c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-FLT_MIN);

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
                ImGui::InputFloat2("##value", (float*)&node.props[i].DataFinal[0]);
            }
            // Vec3
            else if (node.props[i].Type == 5) {
                ImGui::InputFloat3("##value", (float*)&node.props[i].DataFinal[0]);
            }
            // Vec4
            else if (node.props[i].Type == 6) {
                ImGui::InputFloat4("##value", (float*)&node.props[i].DataFinal[0]);
            }
            // Mat3x3
            else if (node.props[i].Type == 7) {
                ImGui::InputFloat3("##value", (float*)&node.props[i].DataFinal[0]);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat3("##value", (float*)&node.props[i].DataFinal[12]);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat3("##value", (float*)&node.props[i].DataFinal[24]);
            }
            // Mat4x4
            else if (node.props[i].Type == 8) {
                ImGui::InputFloat4("##value", (float*)&node.props[i].DataFinal[0]);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat4("##value", (float*)&node.props[i].DataFinal[16]);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat4("##value", (float*)&node.props[i].DataFinal[32]);
                ImGui::SetNextItemWidth(-FLT_MIN);

                ImGui::InputFloat4("##value", (float*)&node.props[i].DataFinal[48]);
            }
            // A[U32]
            else if (node.props[i].Type == 9) {
                if (node.props[i].DataFinal.GetSize())
                    ImGui::InputScalarN("##value", ImGuiDataType_U32, &node.props[i].DataFinal[0], (int)node.props[i].DataFinal.GetSize() / sizeof(u32));
                else
                    ImGui::Text("Empty array of u32.");
            }
            // A[F32]
            else if (node.props[i].Type == 10) {
                if (node.props[i].DataFinal.GetSize())
                    ImGui::InputScalarN("##value", ImGuiDataType_Float, &node.props[i].DataFinal[0], (int)node.props[i].DataFinal.GetSize() / sizeof(float));
                else
                    ImGui::Text("Empty array of f32.");
            }
            // A[U8]
            else if (node.props[i].Type == 11) {
                if (node.props[i].DataFinal.GetSize())
                    ImGui::InputScalarN("##value", ImGuiDataType_U8, &node.props[i].DataFinal[0], (int)node.props[i].DataFinal.GetSize() / sizeof(u8));
                else
                    ImGui::Text("Empty array of u8.");
            }
            // ObjID
            else if (node.props[i].Type == 13) {
                ImGui::InputScalar("##value", ImGuiDataType_U64, &node.props[i].DataFinal[0]);
            }
            // Event
            else if (node.props[i].Type == 14) {
                if (node.props[i].DataFinal.GetSize())
                    ImGui::InputScalarN("##value", ImGuiDataType_U64, &node.props[i].DataFinal[0], (int)node.props[i].DataFinal.GetSize() / sizeof(u64));
                else
                    ImGui::Text("Empty event.");
            }

            ImGui::NextColumn();
            ImGui::PopID();
        }

        for (int i = 0; i < node.ChildCount; i++)
            ShowNode(node.childs[i], showAll);

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void ShowPropertyEditor(bool* open, bool showAll) {
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Property editor", open)) {
        ImGui::End();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

    if (ImGui::BeginTable("split", 3, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable)) {
        ShowNode(rtpcFile.mainNode, showAll);

        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow) {
    InitHashMap();

    sf::RenderWindow window(sf::VideoMode(1280, 720), "APEX.PropertyEditor");
    sf::Clock clock;

    bool showAll = true;
    window.setVerticalSyncEnabled(true);

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

        ImGui::Begin("Load/Save File");
        {
            if (ImGui::Button("Load File")) {
                if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
                    IFileOpenDialog* pFileOpen = nullptr;

                    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen)))) {
                        if (SUCCEEDED(pFileOpen->Show(nullptr))) {
                            IShellItem* pItem = nullptr;

                            if (SUCCEEDED(pFileOpen->GetResult(&pItem))) {
                                PWSTR pszFilePath = nullptr;

                                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
                                    currentFileName = pszFilePath;

                                    rtpcFile.Clear();
                                    ProcessRTPC(currentFileName);
                                    showAll = true;

                                    CoTaskMemFree(pszFilePath);
                                }

                                pItem->Release();
                            }
                        }

                        pFileOpen->Release();
                    }

                    CoUninitialize();
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Reload File")) {
                rtpcFile.Clear();
                ProcessRTPC(currentFileName);
                showAll = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Save File")) {
                writtenStrings.clear();
                std::ofstream file(currentFileName, std::ios::binary);
                rtpcFile.Serialize(file);
                file.close();
            }

            ImGui::SameLine();

            if (ImGui::Button("Save File As")) {
                if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
                    IFileSaveDialog* pFileSave = nullptr;

                    if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave)))) {
                        if (SUCCEEDED(pFileSave->Show(nullptr))) {
                            IShellItem* pItem = nullptr;

                            if (SUCCEEDED(pFileSave->GetResult(&pItem))) {
                                PWSTR pszFilePath = nullptr;

                                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
                                    currentFileName = pszFilePath;
                                    writtenStrings.clear();

                                    std::ofstream file(currentFileName, std::ios::binary);
                                    rtpcFile.Serialize(file);
                                    file.close();

                                    CoTaskMemFree(pszFilePath);
                                }

                                pItem->Release();
                            }
                        }

                        pFileSave->Release();

                    }

                    CoUninitialize();
                }
            }

            ImGui::SameLine();

            ImGui::Checkbox("Enable Prop Search", &searchOn);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText("##value", &searchStr);
        }
        ImGui::End();

        ShowPropertyEditor(nullptr, showAll);
        if (showAll)
            showAll = false;

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
