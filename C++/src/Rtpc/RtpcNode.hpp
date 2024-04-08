#pragma once

#include "RtpcProp.hpp"

extern vector<string> writtenStrings;

class RtpcNode {
    void WritePadding(std::ofstream& file, int alignTo);

public:
    // Header data
    u32 HashedName = 0;
    u32 DataOffset = 0;
    u16 PropsCount = 0;
    u16 ChildCount = 0;

    // Node data
    vector<RtpcProp> props;
    vector<RtpcNode> childs;

    // Custom data
    string DehashedName;

    RtpcNode();

    void Clear();
    void ClearWrite();

    bool Deserialize(std::ifstream& file, bool handleShared = false);

    bool Serialize_V1(std::ofstream& file, bool writeSelf = true);
    bool Serialize_V2(std::ofstream& file, bool writeSelf = true);

    // TODO: Check if order is right.
    // TODO: A lot of repeated code. SIMPLIFY IT!!!

    void ConstructStrings();
    void ConstructVec2();
    void ConstructVec3();
    void ConstructVec4();
    void ConstructMat3x3();
    void ConstructMat4x4();
    void ConstructAU32();
    void ConstructAF32();
    void ConstructAU8();
    void ConstructObjID();
    void ConstructEvent();

    bool Serialize_V3_Headers(std::ofstream& file, bool writeSelf = true);

    void FixupStrings(std::ofstream& file);
    void FixupVec2(std::ofstream& file);
    void FixupVec3(std::ofstream& file);
    void FixupVec4(std::ofstream& file);
    void FixupMat3x3(std::ofstream& file);
    void FixupMat4x4(std::ofstream& file);
    void FixupAU32(std::ofstream& file);
    void FixupAF32(std::ofstream& file);
    void FixupAU8(std::ofstream& file);
    void FixupObjID(std::ofstream& file);
    void FixupEvent(std::ofstream& file);
};
