#pragma once

#include "RtpcProp.hpp"

extern vector<string> writtenStrings;

class RtpcNode {
	void WritePadding(std::ofstream& file);

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

	bool Deserialize(std::ifstream& file);

	bool Serialize_V1(std::ofstream& file, bool writeSelf = true);
	bool Serialize_V2(std::ofstream& file, bool writeSelf = true);
	bool Serialize_V3(std::ofstream& file, bool writeSelf = true);
};
