#pragma once

#include "RtpcNode.hpp"

class RtpcFile {
public:
	// Header data
	u32 Magic;
	u32 Version;

	RtpcNode mainNode;

	RtpcFile();

	void Clear();

	bool Deserialize(std::ifstream& file);

	bool Serialize(std::ofstream& file);
};
