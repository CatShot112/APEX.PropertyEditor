#include "RtpcFile.hpp"

RtpcFile::RtpcFile() {
    Magic = 0;
    Version = 0;
}

void RtpcFile::Clear() {
    Magic = 0;
    Version = 0;

    mainNode.Clear();
}

bool RtpcFile::Deserialize(std::ifstream& file) {
    // Read header
    file.read((char*)&Magic, sizeof(u32));
    file.read((char*)&Version, sizeof(u32));

    if (Magic != 0x43505452) {
        printf("[ERRO]: RTPC file magic is wrong!\n");
        return false;
    }

    if (Version == 1 || Version == 2 || Version == 3) {
        printf("Opening file version: %d\n", Version);

        if (Version == 3)
            mainNode.Deserialize(file, true);
        else
            mainNode.Deserialize(file);
    }
    else {
        printf("[ERRO]: (Deserialize) RTPC file version not supported: %d\n", Version);
        return false;
    }

    return true;
}

bool RtpcFile::Serialize(std::ofstream& file) {
    // Write header
    file.write((char*)&Magic, sizeof(u32));
    file.write((char*)&Version, sizeof(u32));

    if (Version == 1) {
        return mainNode.Serialize_V1(file);
    }
    else if (Version == 2) {
        return mainNode.Serialize_V2(file);
    }
    else if (Version == 3) {
        // 1. Construct strings, and sort them.
        // 2. Construct vec2, vec3, vec4, ??? and sort them.
        // 
        // 3. Update offsets. (TODO, This should allow modification of strings, shared props and 'complex' types)
        // 
        // 4. Write ONLY headers (like V2, with valid props count).
        // 5. Write all constructed stuff in order (str, vec2, vec3, vec4, ???).

        mainNode.ClearWrite();

        mainNode.ConstructStrings();
        mainNode.ConstructVec2();
        mainNode.ConstructVec3();
        mainNode.ConstructVec4();
        mainNode.ConstructMat3x3();
        mainNode.ConstructMat4x4();
        mainNode.ConstructAU32();
        mainNode.ConstructAF32();
        mainNode.ConstructAU8();
        mainNode.ConstructObjID();
        mainNode.ConstructEvent();

        mainNode.Serialize_V3_Headers(file);

        mainNode.Serialize_V3_Strings(file);
        mainNode.Serialize_V3_Vec2(file);
        mainNode.Serialize_V3_Vec3(file);
        mainNode.Serialize_V3_Vec4(file);
        mainNode.Serialize_V3_Mat3x3(file);
        mainNode.Serialize_V3_Mat4x4(file);
        mainNode.Serialize_V3_AU32(file);
        mainNode.Serialize_V3_AF32(file);
        mainNode.Serialize_V3_AU8(file);
        mainNode.Serialize_V3_ObjID(file);
        mainNode.Serialize_V3_Event(file);

        return true;
    }

    printf("[ERRO]: (Serialize) RTPC file version not supported: %d\n", Version);

    return false;
}
