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

	void ConstructStrings();
	void ConstructVec2();
	void ConstructVec3();
	void ConstructVec4();

	bool Serialize_V3_Headers(std::ofstream& file, bool writeSelf = true);
	bool Serialize_V3_Strings(std::ofstream& file);
	bool Serialize_V3_Vec2(std::ofstream& file);
	bool Serialize_V3_Vec3(std::ofstream& file);
	bool Serialize_V3_Vec4(std::ofstream& file);

};
