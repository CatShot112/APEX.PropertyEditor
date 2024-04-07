#include "RtpcNode.hpp"
#include "../Structs.hpp"

#include <algorithm>

vector<string> writtenStrings;

unordered_map<string, u32> strings;
unordered_map<string, u32> vec2s;
unordered_map<string, u32> vec3s;
unordered_map<string, u32> vec4s;
unordered_map<string, u32> mat3x3s;
unordered_map<string, u32> mat4x4s;
unordered_map<string, u32> au32s;
unordered_map<string, u32> af32s;
unordered_map<string, u32> au8s;
unordered_map<string, u32> aobjids;
unordered_map<string, u32> aevents;

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
        if (props[i].Type == PTYPE_STR) {
            if (!strings.count(props[i].DataStr)) {
                //file.write(props[i].DataStr.c_str(), strlen(props[i].DataStr.c_str()) + 1);
                strings[props[i].DataStr] = props[i].DataRaw; // String -> offset
            }
            else {
                props[i].DataRaw = strings.at(props[i].DataStr); // Update offset to existing string.
            }
        }
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructStrings();
}
void RtpcNode::ConstructVec2() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_VEC2)
            continue;

        string vec2 = props[i].DataFinal.ToString();

        if (!vec2s.count(vec2))
            vec2s[vec2] = props[i].DataRaw;
        else
            props[i].DataRaw = vec2s.at(vec2);
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructVec2();
}
void RtpcNode::ConstructVec3() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_VEC3)
            continue;

        string vec3 = props[i].DataFinal.ToString();

        if (!vec3s.count(vec3))
            vec3s[vec3] = props[i].DataRaw;
        else
            props[i].DataRaw = vec3s.at(vec3);
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructVec3();
}
void RtpcNode::ConstructVec4() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_VEC4)
            continue;

        string vec4 = props[i].DataFinal.ToString();

        if (!vec4s.count(vec4))
            vec4s[vec4] = props[i].DataRaw;
        else
            props[i].DataRaw = vec4s.at(vec4);
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructVec4();
}
void RtpcNode::ConstructMat3x3() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_MAT3X3)
            continue;

        string mat3x3 = props[i].DataFinal.ToString();

        if (!mat3x3s.count(mat3x3))
            mat3x3s[mat3x3] = props[i].DataRaw;
        else
            props[i].DataRaw = mat3x3s.at(mat3x3);
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructMat3x3();
}
void RtpcNode::ConstructMat4x4() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_MAT4X4)
            continue;

        string mat4x4 = props[i].DataFinal.ToString();

        if (!mat4x4s.count(mat4x4))
            mat4x4s[mat4x4] = props[i].DataRaw;
        else
            props[i].DataRaw = mat4x4s.at(mat4x4);
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructMat4x4();
}
void RtpcNode::ConstructAU32() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_AU32)
            continue;

        string au32 = props[i].DataFinal.ToString();

        if (!au32s.count(au32))
            au32s[au32] = props[i].DataRaw;
        else
            props[i].DataRaw = au32s.at(au32);
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructAU32();
}
void RtpcNode::ConstructAF32() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_AF32)
            continue;

        string af32 = props[i].DataFinal.ToString();

        if (!af32s.count(af32))
            af32s[af32] = props[i].DataRaw;
        else
            props[i].DataRaw = af32s.at(af32);
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructAF32();
}
void RtpcNode::ConstructAU8() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_AU8)
            continue;

        string au8 = props[i].DataFinal.ToString();

        if (!au8s.count(au8))
            au8s[au8] = props[i].DataRaw;
        else
            props[i].DataRaw = au8s.at(au8);
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructAU8();
}
void RtpcNode::ConstructObjID() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_OBJID)
            continue;

        string objid = props[i].DataFinal.ToString();

        if (!aobjids.count(objid))
            aobjids[objid] = props[i].DataRaw;
        else
            props[i].DataRaw = aobjids.at(objid);
    }

    for (u16 i = 0; i < ChildCount; i++)
        childs[i].ConstructObjID();
}
void RtpcNode::ConstructEvent() {
    for (u16 i = 0; i < PropsCount; i++) {
        if (props[i].Type != PTYPE_EVENT)
            continue;

        string event = props[i].DataFinal.ToString();

        if (!aevents.count(event))
            aevents[event] = props[i].DataRaw;
        else
            props[i].DataRaw = aevents.at(event);
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

    // TODO: Figure out this garbage
    // 
    // 1. Write ONLY headers (like V2, with valid props count)
    // 
    // 2. Write all strings sorted and only one instance of each
    // 
    // 3. Write all vec2?
    // 4. Write all vec3?
    // 
    // Looks like props data other than u32, f32 are stored the same way like strings? (shared values between props)

    return true;
}

bool RtpcNode::Serialize_V3_Strings(std::ofstream& file) {
    vector<string> vStrings;
    
    // Construct vector of strings to write from map and sort it.
    for (auto& it : strings)
        vStrings.emplace_back(it.first);

    std::sort(vStrings.begin(), vStrings.end());

    // Write those strings to file.
    for (auto& s : vStrings) {
        file.write(s.c_str(), strlen(s.c_str()) + 1);

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    if (vec2s.size() || vec3s.size() || vec4s.size())
        WritePadding(file, 4);

    return true;
}
bool RtpcNode::Serialize_V3_Vec2(std::ofstream& file) {
    vector<std::pair<f32, f32>> vVec2s;

    // Construct vector of vec2 (pairs) to write from map and sort it.
    for (auto& it : vec2s) {
        DataBuf data = it.first;
        Vec2 vec2{};
        
        data.Read((char*)&vec2, sizeof(Vec2));
        vVec2s.emplace_back(std::make_pair(vec2.x, vec2.y));
    }

    std::sort(vVec2s.begin(), vVec2s.end());

    for (auto& v : vVec2s) {
        file.write((char*)&v, sizeof(Vec2));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    if (vec3s.size() || vec4s.size())
        WritePadding(file, 4);

    return true;
}
bool RtpcNode::Serialize_V3_Vec3(std::ofstream& file) {
    vector<std::tuple<f32, f32, f32>> vVec3s;

    for (auto& it : vec3s) {
        DataBuf data = it.first;
        Vec3 vec3{};

        data.Read((char*)&vec3, sizeof(Vec3));
        vVec3s.emplace_back(std::make_tuple(vec3.x, vec3.y, vec3.z));
    }

    std::sort(vVec3s.begin(), vVec3s.end());

    for (auto& v : vVec3s) {
        //file.write((char*)&v, sizeof(Vec3));

        file.write((char*)&std::get<0>(v), sizeof(f32));
        file.write((char*)&std::get<1>(v), sizeof(f32));
        file.write((char*)&std::get<2>(v), sizeof(f32));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    if (vec4s.size() || mat3x3s.size() || mat4x4s.size())
        WritePadding(file, 16);

    return true;
}
bool RtpcNode::Serialize_V3_Vec4(std::ofstream& file) {
    vector<std::tuple<f32, f32, f32, f32>> vVec4s;

    for (auto& it : vec4s) {
        DataBuf data = it.first;
        Vec4 vec4{};

        data.Read((char*)&vec4, sizeof(Vec4));
        vVec4s.emplace_back(std::make_tuple(vec4.x, vec4.y, vec4.z, vec4.w));
    }

    std::sort(vVec4s.begin(), vVec4s.end());

    for (auto& v : vVec4s) {
        //file.write((char*)&v, sizeof(Vec4)); // This saves in reverse order. ? WTF ?

        file.write((char*)&std::get<0>(v), sizeof(f32));
        file.write((char*)&std::get<1>(v), sizeof(f32));
        file.write((char*)&std::get<2>(v), sizeof(f32));
        file.write((char*)&std::get<3>(v), sizeof(f32));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    WritePadding(file, 4);

    return true;
}
bool RtpcNode::Serialize_V3_Mat3x3(std::ofstream& file) {
    typedef std::tuple<f32, f32, f32> tupl3;
    std::vector<std::tuple<tupl3, tupl3, tupl3>> vMat3x3s;

    for (auto& it : mat3x3s) {
        DataBuf data = it.first;
        Mat3x3 m;

        data.Read((char*)&m, sizeof(Mat3x3));
        vMat3x3s.emplace_back(std::make_tuple(std::make_tuple(m.a00, m.a01, m.a02), std::make_tuple(m.b00, m.b01, m.b02), std::make_tuple(m.c00, m.c01, m.c02)));
    }

    std::sort(vMat3x3s.begin(), vMat3x3s.end());

    for (auto& v : vMat3x3s) {
        file.write((char*)&std::get<0>(std::get<0>(v)), sizeof(f32));
        file.write((char*)&std::get<0>(std::get<1>(v)), sizeof(f32));
        file.write((char*)&std::get<0>(std::get<2>(v)), sizeof(f32));
        file.write((char*)&std::get<1>(std::get<0>(v)), sizeof(f32));
        file.write((char*)&std::get<1>(std::get<1>(v)), sizeof(f32));
        file.write((char*)&std::get<1>(std::get<2>(v)), sizeof(f32));
        file.write((char*)&std::get<2>(std::get<0>(v)), sizeof(f32));
        file.write((char*)&std::get<2>(std::get<1>(v)), sizeof(f32));
        file.write((char*)&std::get<2>(std::get<2>(v)), sizeof(f32));

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    WritePadding(file, 4);

    return true;
}
bool RtpcNode::Serialize_V3_Mat4x4(std::ofstream& file) {
    typedef std::tuple<f32, f32, f32, f32> tupl4;
    std::vector<std::tuple<tupl4, tupl4, tupl4, tupl4>> vMat4x4s;
    
    for (auto& it : mat4x4s) {
        DataBuf data = it.first;
        Mat4x4 m;
        
        data.Read((char*)&m, sizeof(Mat4x4));
        vMat4x4s.emplace_back(std::make_tuple(std::make_tuple(m.a00, m.a01, m.a02, m.a03), std::make_tuple(m.b00, m.b01, m.b02, m.b03), std::make_tuple(m.c00, m.c01, m.c02, m.c03), std::make_tuple(m.d00, m.d01, m.d02, m.d03)));
    }
    
    std::sort(vMat4x4s.begin(), vMat4x4s.end());
    
    for (auto& v : vMat4x4s) {
        file.write((char*)&std::get<0>(std::get<0>(v)), sizeof(f32));
        file.write((char*)&std::get<0>(std::get<1>(v)), sizeof(f32));
        file.write((char*)&std::get<0>(std::get<2>(v)), sizeof(f32));
        file.write((char*)&std::get<0>(std::get<3>(v)), sizeof(f32));
        file.write((char*)&std::get<1>(std::get<0>(v)), sizeof(f32));
        file.write((char*)&std::get<1>(std::get<1>(v)), sizeof(f32));
        file.write((char*)&std::get<1>(std::get<2>(v)), sizeof(f32));
        file.write((char*)&std::get<1>(std::get<3>(v)), sizeof(f32));
        file.write((char*)&std::get<2>(std::get<0>(v)), sizeof(f32));
        file.write((char*)&std::get<2>(std::get<1>(v)), sizeof(f32));
        file.write((char*)&std::get<2>(std::get<2>(v)), sizeof(f32));
        file.write((char*)&std::get<2>(std::get<3>(v)), sizeof(f32));
        file.write((char*)&std::get<3>(std::get<0>(v)), sizeof(f32));
        file.write((char*)&std::get<3>(std::get<1>(v)), sizeof(f32));
        file.write((char*)&std::get<3>(std::get<2>(v)), sizeof(f32));
        file.write((char*)&std::get<3>(std::get<3>(v)), sizeof(f32));
        
#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }
    
    WritePadding(file, 4);

    return true;
}
bool RtpcNode::Serialize_V3_AU32(std::ofstream& file) {

    return true;
}
bool RtpcNode::Serialize_V3_AF32(std::ofstream& file) {

    return true;
}
bool RtpcNode::Serialize_V3_AU8(std::ofstream& file) {

    return true;
}
bool RtpcNode::Serialize_V3_ObjID(std::ofstream& file) {

    return true;
}
bool RtpcNode::Serialize_V3_Event(std::ofstream& file) {

    return true;
}

/*bool RtpcNode::Serialize_V3_WriteHeaders(std::ofstream& file, bool writeSelf) {
    // First pass: Write headers + valid props count
    // Second pass: Write all strings sorted
    // Third pass: Write all vec2
    // Fourth pass: Write all 

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
#ifdef _DEBUG
    file.flush(); // DEBUG
#endif

    // Write child nodes
    for (u16 i = 0; i < ChildCount; i++)
        childs[i].Serialize_V3_WriteHeaders(file, false);

    return true;
}

bool RtpcNode::Serialize_V3_GetStrings(std::ofstream& file, RtpcNode& node) {
    // Get list of all strings
    for (u16 i = 0; i < node.PropsCount; i++) {
        if (node.props[i].Type == 3) {
            // Make sure we only write one instance of a string
            if (!std::count(writtenStrings.begin(), writtenStrings.end(), node.props[i].DataStr)) {
                writtenStrings.emplace_back(node.props[i].DataStr);
            }
        }
    }

    for (u16 i = 0; i < node.ChildCount; i++)
        Serialize_V3_GetStrings(file, node.childs[i]);

    return true;
}

bool RtpcNode::Serialize_V3_WriteStrings(std::ofstream& file) {
    std::sort(writtenStrings.begin(), writtenStrings.end());

    for (size_t i = 0; i < writtenStrings.size(); i++) {
        file.write(writtenStrings[i].c_str(), strlen(writtenStrings[i].c_str()) + 1);

#ifdef _DEBUG
        file.flush(); // DEBUG
#endif
    }

    WritePadding(file, 4);

    return true;
}*/
