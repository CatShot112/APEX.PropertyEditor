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

// Constructed stuff:
//   Each string/big property can occur only once.
//   In order to write all of that correctly, we must iterate through all properties of all nodes and check
//   if it's data already exists in array. If so, update offset to point to existing data.
//
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
        // Write headers to temporary buffer instead of directly to file. Then get it's size (it shouldn't ever change).

        // 1. Construct strings, and sort them.
        // 
        // String must know it's owner/owners in order to update their offsets. This also applies to other data types.
        // 
        // 2. Construct vec2, vec3, vec4, ??? and sort them.
        // 
        // 3. Update offsets. (TODO, This should allow modification of strings, shared props and 'complex' types)
        // 
        // 4. Write ONLY headers (like V2, with valid props count and updated offsets).
        // 5. Write all constructed stuff in order (str, vec2, vec3, vec4, ???).
        // 
        // Size of headers doesn't change, so we can calculate where real data will be written?

        mainNode.ClearWrite();

        mainNode.Serialize_V3_Headers(file);

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

        mainNode.FixupStrings(file);
        mainNode.FixupVec2(file);
        mainNode.FixupVec3(file);
        mainNode.FixupVec4(file);
        mainNode.FixupMat3x3(file);
        mainNode.FixupMat4x4(file);
        mainNode.FixupAU32(file);
        mainNode.FixupAF32(file);
        mainNode.FixupAU8(file);
        mainNode.FixupObjID(file);
        mainNode.FixupEvent(file);

        return true;
    }

    printf("[ERRO]: (Serialize) RTPC file version not supported: %d\n", Version);

    return false;
}
