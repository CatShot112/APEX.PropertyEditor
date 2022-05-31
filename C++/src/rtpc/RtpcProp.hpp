#pragma once

#include "../Typedefs.hpp"
#include "../DataBuf/DataBuf.hpp"

#include <fstream>

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

	RtpcProp();

	bool Deserialize(std::ifstream& file);
};
