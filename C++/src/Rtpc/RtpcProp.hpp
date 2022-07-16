#pragma once

#include "../Typedefs.hpp"
#include "../DataBuf/DataBuf.hpp"

#include <fstream>

extern vector<u32> readOffsets;

class RtpcProp {
public:
	// Header data
	u32 HashedName;
	u32 DataRaw;
	u8 Type;

	// Custom data
	string DehashedName;
	string DataStr;
	DataBuf DataFinal;

	bool IsShared;

	RtpcProp();

	bool Deserialize(std::ifstream& file, bool handleShared = false);
};
