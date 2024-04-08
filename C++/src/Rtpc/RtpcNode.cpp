#include "RtpcNode.hpp"

#include "../Structs.hpp"
#include "../../headers/jenkins/hashlittle.h"

#include <algorithm>
#include <map>

vector<string> writtenStrings;

std::map<string, vector<RtpcProp*>> strings;
std::map<string, vector<RtpcProp*>> vec2s;
std::map<string, vector<RtpcProp*>> vec3s;
std::map<string, vector<RtpcProp*>> vec4s;
std::map<string, vector<RtpcProp*>> mat3x3s;
std::map<string, vector<RtpcProp*>> mat4x4s;
std::map<string, vector<RtpcProp*>> au32s;
std::map<string, vector<RtpcProp*>> af32s;
std::map<string, vector<RtpcProp*>> au8s;
std::map<string, vector<RtpcProp*>> aobjids;
std::map<string, vector<RtpcProp*>> aevents;

RtpcNode::RtpcNode() {
    HashedName = 0;
    DataOffset = 0;
    PropsCount = 0;
    ChildCount = 0;

    props.reserve(256);
    childs.reserve(128);
}

void RtpcNode::Clear() {
    HashedName = 0;
    DataOffset = 0;
    PropsCount = 0;
    ChildCount = 0;

    props.clear();
    childs.clear();
}

void RtpcNode::ClearWrite() {
    strings.clear();
    vec2s.clear();
    vec3s.clear();
    vec4s.clear();
    mat3x3s.clear();
    mat4x4s.clear();
    au32s.clear();
    af32s.clear();
    au8s.clear();
    aobjids.clear();
    aevents.clear();
}

void RtpcNode::WritePadding(std::ofstream& file, int alignTo) {
    std::streamoff pos = file.tellp();
    int add = (alignTo - (pos % alignTo)) % alignTo;

    for (int p = 0; p < add; p++)
        file.write("\x50", 1);

#ifdef _DEBUG
    file.flush(); // DEBUG
#endif
}

bool RtpcNode::Deserialize(std::ifstream& file, bool handleShared) {
    // Read header
    file.read((char*)&HashedName, sizeof(u32));
    file.read((char*)&DataOffset, sizeof(u32));
    file.read((char*)&PropsCount, sizeof(u16));
    file.read((char*)&ChildCount, sizeof(u16));

    // Save current read position and go to node data
    std::streamoff oPos = file.tellg();
    file.seekg(DataOffset);

    // Read props headers
    for (u16 i = 0; i < PropsCount; i++) {
        RtpcProp prop{};

        prop.Deserialize(file);
        props.emplace_back(prop);
    }

    // 4-byte align after props headers
    std::streamoff pos = file.tellg();
    file.seekg(pos + (4 - (pos % 4)) % 4);

    // Read child nodes headers
    for (u16 i = 0; i < ChildCount; i++) {
        RtpcNode child;

        child.Deserialize(file);
        childs.emplace_back(child);
    }

    // Go to saved read position
    file.seekg(oPos);

    return true;
}

bool RtpcNode::Serialize_V1(std::ofstream& file, bool writeSelf) {
    // Write header
    if (writeSelf) {
        file.write((char*)&HashedName, sizeof(u32));
        file.write((char*)&DataOffset, sizeof(u32));
        file.write((char*)&PropsCount, sizeof(u16));
        file.write((char*)&ChildCount, sizeof(u16));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    // Write props headers
    for (u16 i = 0; i < PropsCount; i++) {
        file.write((char*)&props[i].HashedName, sizeof(u32));
        file.write((char*)&props[i].DataRaw, sizeof(u32));
        file.write((char*)&props[i].Type, sizeof(u8));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    // 4-byte align after props headers
    if (PropsCount)
        WritePadding(file, 4);

    // Write childs headers
    for (u16 i = 0; i < ChildCount; i++) {
        file.write((char*)&childs[i].HashedName, sizeof(u32));
        file.write((char*)&childs[i].DataOffset, sizeof(u32));
        file.write((char*)&childs[i].PropsCount, sizeof(u16));
        file.write((char*)&childs[i].ChildCount, sizeof(u16));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    // Write props data
    for (u16 i = 0; i < PropsCount; i++) {
        switch (props[i].Type) {
        case 4: // Vec2
            // Prob need align to 4
            file.write((char*)&props[i].DataFinal[0], sizeof(float) * 2);
            break;
        case 5: // Vec3
            // Prob need align to 4
            file.write((char*)&props[i].DataFinal[0], sizeof(float) * 3);
            break;
        case 6: // Vec4
            WritePadding(file, 16); // Align to 16 bytes before writing
            file.write((char*)&props[i].DataFinal[0], sizeof(float) * 4);
            break;
        case 7: // Mat3x3
            WritePadding(file, 16); // Align to 16 bytes before writing
            file.write((char*)&props[i].DataFinal[0], sizeof(float) * 3 * 3);
            break;
        case 8: // Mat4x4
            WritePadding(file, 16); // Align to 16 bytes before writing
            file.write((char*)&props[i].DataFinal[0], sizeof(float) * 4 * 4);
            break;
        case 13: // ObjID
            WritePadding(file, 4); // Align to 5 bytes before writing
            file.write((char*)&props[i].DataFinal[0], sizeof(u64));
            break;
        case 9: // A[U32]
        {
            WritePadding(file, 4); // Align to 4 bytes before writing
            u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(u32);
            file.write((char*)&arraySize, sizeof(u32));

            if (arraySize)
                file.write((char*)&props[i].DataFinal[0], sizeof(u32) * arraySize);

            break;
        }
        case 10: // A[F32]
        {
            // Prob need align to 4
            u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(float);
            file.write((char*)&arraySize, sizeof(u32));

            if (arraySize)
                file.write((char*)&props[i].DataFinal[0], sizeof(float) * arraySize);

            break;
        }
        case 11: // A[U8]
        {
            // Prob need align to 4
            u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(u8);
            file.write((char*)&arraySize, sizeof(u32));

            if (arraySize)
                file.write((char*)&props[i].DataFinal[0], sizeof(u8) * arraySize);

            break;
        }
        case 14: // Event
        {
            // Prob need align to 4
            u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(u64);
            file.write((char*)&arraySize, sizeof(u32));

            if (arraySize)
                file.write((char*)&props[i].DataFinal[0], sizeof(u64) * arraySize);

            break;
        }
        case 3: // Str
        {
            // Make sure we only write one instance of a string
            if (!std::count(writtenStrings.begin(), writtenStrings.end(), props[i].DataStr)) {
                file.write(props[i].DataStr.c_str(), strlen(props[i].DataStr.c_str()) + 1);
                writtenStrings.emplace_back(props[i].DataStr);

#ifdef _DEBUG
                file.flush(); // DEBUG
#endif
            }

            // We need to align strings to 4 bytes before other data type (except none, u32, f32), and if string is last property
            // TODO: Comment this properly :)
            if (i == PropsCount - 1)
                WritePadding(file, 4);

            if (i + 1 <= PropsCount - 1) {
                bool pad = true;

                if (props[i + 1].Type == 0 || props[i + 1].Type == 1 || props[i + 1].Type == 2) {
                    for (u16 j = i + 1; j < PropsCount - 1; j++) {
                        if (props[j + 1].Type == 3)
                            pad = false;
                    }
                }

                if (pad && props[i + 1].Type != 3)
                    WritePadding(file, 4);
            }

            break;
        }
        default:
            break;
        }

        file.flush();
    }

    // Write child nodes
    for (u16 i = 0; i < ChildCount; i++)
        childs[i].Serialize_V1(file, false);

    return true;
}
bool RtpcNode::Serialize_V2(std::ofstream& file, bool writeSelf) {
    // Write header
    if (writeSelf) {
        file.write((char*)&HashedName, sizeof(u32));
        file.write((char*)&DataOffset, sizeof(u32));
        file.write((char*)&PropsCount, sizeof(u16));
        file.write((char*)&ChildCount, sizeof(u16));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    // Write props headers and count valid props
    u32 cnt = 0;

    for (u16 i = 0; i < PropsCount; i++) {
        file.write((char*)&props[i].HashedName, sizeof(u32));
        file.write((char*)&props[i].DataRaw, sizeof(u32));
        file.write((char*)&props[i].Type, sizeof(u8));

        if (props[i].Type != 0)
            cnt++;

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    // 4-byte align after props headers
    if (PropsCount)
        WritePadding(file, 4);

    // Write childs headers
    for (u16 i = 0; i < ChildCount; i++) {
        file.write((char*)&childs[i].HashedName, sizeof(u32));
        file.write((char*)&childs[i].DataOffset, sizeof(u32));
        file.write((char*)&childs[i].PropsCount, sizeof(u16));
        file.write((char*)&childs[i].ChildCount, sizeof(u16));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    // Write number of valid props
    file.write((char*)&cnt, sizeof(u32));

    // Write props data
    for (u16 i = 0; i < PropsCount; i++) {
        switch (props[i].Type) {
        case 4: // Vec2
            file.write((char*)&props[i].DataFinal[0], sizeof(float) * 2);
            break;
        case 5: // Vec3
            file.write((char*)&props[i].DataFinal[0], sizeof(float) * 3);
            break;
        case 6: // Vec4
            file.write((char*)&props[i].DataFinal[0], sizeof(float) * 4);
            break;
        case 7: // Mat3x3
            file.write((char*)&props[i].DataFinal[0], sizeof(float) * 3 * 3);
            break;
        case 8: // Mat4x4
            file.write((char*)&props[i].DataFinal[0], sizeof(float) * 4 * 4);
            break;
        case 13: // ObjID
            file.write((char*)&props[i].DataFinal[0], sizeof(u64));
            break;
        case 9: // A[U32]
        {
            u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(u32);
            file.write((char*)&arraySize, sizeof(u32));

            if (arraySize)
                file.write((char*)&props[i].DataFinal[0], sizeof(u32) * arraySize);

            break;
        }
        case 10: // A[F32]
        {
            u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(float);
            file.write((char*)&arraySize, sizeof(u32));

            if (arraySize)
                file.write((char*)&props[i].DataFinal[0], sizeof(float) * arraySize);

            break;
        }
        case 11: // A[U8]
        {
            u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(u8);
            file.write((char*)&arraySize, sizeof(u32));

            if (arraySize)
                file.write((char*)&props[i].DataFinal[0], sizeof(u8) * arraySize);

            break;
        }
        case 14: // Event
        {
            u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(u64);
            file.write((char*)&arraySize, sizeof(u32));

            if (arraySize)
                file.write((char*)&props[i].DataFinal[0], sizeof(u64) * arraySize);

            break;
        }
        case 3: // Str
        {
            // Make sure we only write one instance of a string
            if (!std::count(writtenStrings.begin(), writtenStrings.end(), props[i].DataStr)) {
                file.write(props[i].DataStr.c_str(), strlen(props[i].DataStr.c_str()) + 1);
                writtenStrings.emplace_back(props[i].DataStr);

#ifdef _DEBUG
                file.flush(); // DEBUG
#endif
            }

            // We need to align strings to 4 bytes before other data type (except none), and if string is last property
            // TODO: Comment this properly :)
            if (i == PropsCount - 1)
                WritePadding(file, 4);

            if (i + 1 <= PropsCount - 1) {
                bool pad = true;

                if (props[i + 1].Type == 0) {
                    for (u16 j = i + 1; j < PropsCount - 1; j++) {
                        if (props[j + 1].Type == 3)
                            pad = false;
                    }
                }

                if (pad && props[i + 1].Type != 3)
                    WritePadding(file, 4);
            }

            break;
        }
        default:
            break;
        }

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    // Write child nodes
    for (u16 i = 0; i < ChildCount; i++)
        childs[i].Serialize_V2(file, false);

    return true;
}

void RtpcNode::ConstructStrings() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_STR)
            continue;

        bool found = false;

        // String found.
        if (strings.count(props[i].DataStr)) {
            for (auto& v : strings[props[i].DataStr]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }
        }

        // String not found.
        if (!found)
            strings[props[i].DataStr].emplace_back(&props[i]);
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructStrings();
}
void RtpcNode::ConstructVec2() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_VEC2)
            continue;

        string data = props[i].DataFinal.ToString();
        bool found = false;

        if (vec2s.count(data)) {
            for (auto& v : vec2s[data]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }

            if (!found)
                vec2s[data].emplace_back(&props[i]);
        }
        else {
            vec2s[data].emplace_back(&props[i]);
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructVec2();
}
void RtpcNode::ConstructVec3() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_VEC3)
            continue;

        string data = props[i].DataFinal.ToString();
        bool found = false;

        if (vec3s.count(data)) {
            for (auto& v : vec3s[data]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }

            if (!found)
                vec3s[data].emplace_back(&props[i]);
        }
        else {
            vec3s[data].emplace_back(&props[i]);
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructVec3();
}
void RtpcNode::ConstructVec4() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_VEC4)
            continue;

        string data = props[i].DataFinal.ToString();
        bool found = false;

        if (vec4s.count(data)) {
            for (auto& v : vec4s[data]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }

            if (!found)
                vec4s[data].emplace_back(&props[i]);
        }
        else {
            vec4s[data].emplace_back(&props[i]);
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructVec4();
}
void RtpcNode::ConstructMat3x3() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_MAT3X3)
            continue;

        string data = props[i].DataFinal.ToString();
        bool found = false;

        if (mat3x3s.count(data)) {
            for (auto& v : mat3x3s[data]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }

            if (!found)
                mat3x3s[data].emplace_back(&props[i]);
        }
        else {
            mat3x3s[data].emplace_back(&props[i]);
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructMat3x3();
}
void RtpcNode::ConstructMat4x4() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_MAT4X4)
            continue;

        string data = props[i].DataFinal.ToString();
        bool found = false;

        if (mat4x4s.count(data)) {
            for (auto& v : mat4x4s[data]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }

            if (!found)
                mat4x4s[data].emplace_back(&props[i]);
        }
        else {
            mat4x4s[data].emplace_back(&props[i]);
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructMat4x4();
}

void RtpcNode::ConstructAU32() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_AU32)
            continue;

        string data;

        if (props[i].DataFinal.GetSize() <= 0) {
            data = "";
        }
        else {
            data = props[i].DataFinal.ToString();
        }

        bool found = false;

        if (au32s.count(data)) {
            for (auto& v : au32s[data]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }

            if (!found)
                au32s[data].emplace_back(&props[i]);
        }
        else {
            au32s[data].emplace_back(&props[i]);
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructAU32();
}
void RtpcNode::ConstructAF32() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_AF32)
            continue;

        string data = props[i].DataFinal.ToString();
        bool found = false;

        if (af32s.count(data)) {
            for (auto& v : af32s[data]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }

            if (!found)
                af32s[data].emplace_back(&props[i]);
        }
        else {
            af32s[data].emplace_back(&props[i]);
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructAF32();
}
void RtpcNode::ConstructAU8() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_AU8)
            continue;

        string data = props[i].DataFinal.ToString();
        bool found = false;

        if (au8s.count(data)) {
            for (auto& v : au8s[data]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }

            if (!found)
                au8s[data].emplace_back(&props[i]);
        }
        else {
            au8s[data].emplace_back(&props[i]);
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructAU8();
}
void RtpcNode::ConstructObjID() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_OBJID)
            continue;

        string data = props[i].DataFinal.ToString();
        bool found = false;

        if (aobjids.count(data)) {
            for (auto& v : aobjids[data]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }

            if (!found)
                aobjids[data].emplace_back(&props[i]);
        }
        else {
            aobjids[data].emplace_back(&props[i]);
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructObjID();
}
void RtpcNode::ConstructEvent() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_EVENT)
            continue;

        string data = props[i].DataFinal.ToString();
        bool found = false;

        if (aevents.count(data)) {
            for (auto& v : aevents[data]) {
                if (v == &props[i]) {
                    found = true;
                    break;
                }
            }

            if (!found)
                aevents[data].emplace_back(&props[i]);
        }
        else {
            aevents[data].emplace_back(&props[i]);
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructEvent();
}

bool RtpcNode::Serialize_V3_Headers(std::ofstream& file, bool writeSelf) {
    if (writeSelf) {
        file.write((char*)&HashedName, sizeof(u32));
        file.write((char*)&DataOffset, sizeof(u32));
        file.write((char*)&PropsCount, sizeof(u16));
        file.write((char*)&ChildCount, sizeof(u16));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    u32 cnt = 0;
    for (u16 i = 0; i < PropsCount; i++) {
        props[i].Offset = (u32)file.tellp() + sizeof(u32);
        file.write((char*)&props[i].HashedName, sizeof(u32));
        file.write((char*)&props[i].DataRaw, sizeof(u32));
        file.write((char*)&props[i].Type, sizeof(u8));

        if (props[i].Type != 0)
            cnt++;

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    if (PropsCount)
        WritePadding(file, 4);

#ifdef _DEBUG
    file.flush(); // DEBUG
#endif

    for (u16 i = 0; i < ChildCount; i++) {
        file.write((char*)&childs[i].HashedName, sizeof(u32));
        file.write((char*)&childs[i].DataOffset, sizeof(u32));
        file.write((char*)&childs[i].PropsCount, sizeof(u16));
        file.write((char*)&childs[i].ChildCount, sizeof(u16));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    file.write((char*)&cnt, sizeof(u32));

#ifdef _DEBUG
    file.flush(); // DEBUG
#endif

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].Serialize_V3_Headers(file, false);

    return true;
}

void UpdateOffsets(vector<RtpcProp*>& vec, std::ofstream& file) {
    if (vec.size() > 1) {
        u32 offset = (u32)file.tellp();

        for (auto& n : vec) {
            if (n->DataRaw == offset)
                continue;

            n->DataRaw = offset;

            auto oPos = file.tellp();
            file.seekp(n->Offset);
            file.write((char*)&n->DataRaw, sizeof(u32)); file.flush();
            file.seekp(oPos);
        }
    }
}

void RtpcNode::FixupStrings(std::ofstream& file) {
    for (auto& s : strings) {
        /*if (s.second.size() > 1) {
            auto offset = file.tellp();

            for (auto& n : s.second) {
                if (n->DataRaw == (u32)offset)
                    continue;

                n->DataRaw = (u32)offset;

                auto oPos = file.tellp();
                file.seekp(n->Offset);
                file.write((char*)&n->DataRaw, sizeof(u32)); file.flush();
                file.seekp(oPos);
            }
        }*/

        UpdateOffsets(s.second, file);

        file.write(s.first.c_str(), strlen(s.first.c_str()) + 1); file.flush();
    }

    if (vec2s.size() || vec3s.size() || vec4s.size())
        WritePadding(file, 4);
}
void RtpcNode::FixupVec2(std::ofstream& file) {
    for (auto& s : vec2s) {
        UpdateOffsets(s.second, file);

        file.write(s.first.c_str(), sizeof(f32) * 2); file.flush();
    }

    if (vec3s.size() || vec4s.size())
        WritePadding(file, 4);
}
void RtpcNode::FixupVec3(std::ofstream& file) {
    for (auto& s : vec3s) {
        UpdateOffsets(s.second, file);

        file.write(s.first.c_str(), sizeof(f32) * 3); file.flush();
    }

    // TODO: Check if valid.
    if (vec4s.size() || mat3x3s.size() || mat4x4s.size())
        WritePadding(file, 16);
}
void RtpcNode::FixupVec4(std::ofstream& file) {
    for (auto& s : vec4s) {
        UpdateOffsets(s.second, file);

        file.write(s.first.c_str(), sizeof(f32) * 4); file.flush();
    }

    WritePadding(file, 4);
}
void RtpcNode::FixupMat3x3(std::ofstream& file) {
    for (auto& s : mat3x3s) {
        UpdateOffsets(s.second, file);

        file.write(s.first.c_str(), sizeof(f32) * 3 * 3); file.flush();
    }

    WritePadding(file, 4);
}
void RtpcNode::FixupMat4x4(std::ofstream& file) {
    for (auto& s : mat4x4s) {
        UpdateOffsets(s.second, file);

        file.write(s.first.c_str(), sizeof(f32) * 4 * 4); file.flush();
    }

    WritePadding(file, 4);
}

// From this point on it's fucked. Values are sorted incorrectly. TODO: FIXME (check which values are occuring first when constructing map?)
// 1. Sorted based on array length. Longest arrays first.
// 2. If arrays are the same length, sorted HIGH -> LOW (reversed map). ????
// 
// Ok, I don't know how tf this is sorted. IM stupid xD [08.04.2024]
void RtpcNode::FixupAU32(std::ofstream& file) {
    // Sort them first.
    vector<string> vs;
    for (auto& a : au32s)
        vs.emplace_back(a.first);

    std::sort(vs.begin(), vs.end(),
        [](const string& lhs, const string& rhs) {
            auto s1 = lhs.length();
            auto s2 = rhs.length();

            return s1 == s2 ? lhs > rhs : s1 > s2;
        });

    for (auto& s : au32s) {
        int len = (int)s.first.length();

        if (len <= 0) {
            file.write((char*)&len, sizeof(u32)); file.flush();
            continue;
        }

        UpdateOffsets(s.second, file);

        len /= sizeof(u32);

        file.write((char*)&len, sizeof(u32)); file.flush();
        file.write(s.first.c_str(), s.first.length()); file.flush();
    }

    WritePadding(file, 4);
}
void RtpcNode::FixupAF32(std::ofstream& file) {
    for (auto& s : af32s) {
        int len = (int)s.first.length();

        if (len <= 0) {
            file.write((char*)&len, sizeof(u32)); file.flush();
            continue;
        }

        UpdateOffsets(s.second, file);

        len /= sizeof(f32);

        file.write((char*)&len, sizeof(u32)); file.flush();
        file.write(s.first.c_str(), s.first.length()); file.flush();
    }

    WritePadding(file, 4);
}
void RtpcNode::FixupAU8(std::ofstream& file) {
    for (auto& s : au8s) {
        int len = (int)s.first.length();

        if (len <= 0) {
            file.write((char*)&len, sizeof(u32)); file.flush();
            continue;
        }

        UpdateOffsets(s.second, file);

        len /= sizeof(u8);

        file.write((char*)&len, sizeof(u32)); file.flush();
        file.write(s.first.c_str(), s.first.length()); file.flush();
    }

    WritePadding(file, 4);
}
void RtpcNode::FixupObjID(std::ofstream& file) {
    for (auto& s : aobjids) {
        int len = (int)s.first.length();

        if (len <= 0) {
            file.write((char*)&len, sizeof(u32)); file.flush();
            continue;
        }

        UpdateOffsets(s.second, file);

        len /= sizeof(u64);

        file.write((char*)&len, sizeof(u32)); file.flush();
        file.write(s.first.c_str(), s.first.length()); file.flush();
    }

    WritePadding(file, 4);
}
void RtpcNode::FixupEvent(std::ofstream& file) {
    for (auto& s : aevents) {
        int len = (int)s.first.length();

        if (len <= 0) {
            file.write((char*)&len, sizeof(u32)); file.flush();
            continue;
        }

        UpdateOffsets(s.second, file);

        len /= sizeof(u64);

        file.write((char*)&len, sizeof(u32)); file.flush();
        file.write(s.first.c_str(), s.first.length()); file.flush();
    }

    WritePadding(file, 4);
}
