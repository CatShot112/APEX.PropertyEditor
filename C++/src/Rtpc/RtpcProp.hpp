#pragma once

#include "../Typedefs.hpp"
#include "../DataBuf/DataBuf.hpp"

#include <fstream>

extern vector<u32> readOffsets;

enum PTYPE {
	PTYPE_NONE,
	PTYPE_U32,
	PTYPE_F32,
	PTYPE_STR,
	PTYPE_VEC2,
	PTYPE_VEC3,
	PTYPE_VEC4,
	PTYPE_MAT3X3,
	PTYPE_MAT4X4,
	PTYPE_AU32,
	PTYPE_AF32,
	PTYPE_AU8,
	PTYPE_DEPRECATED_01,
	PTYPE_OBJID,
	PTYPE_EVENT
};

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
