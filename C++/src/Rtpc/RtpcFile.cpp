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

	if (Version == 1)
		return mainNode.Serialize_V1(file);
	else if (Version == 2)
		return mainNode.Serialize_V2(file);
	//else if (Version == 3)
	//	return mainNode.Serialize_V3(file);

	printf("[ERRO]: (Serialize) RTPC file version not supported: %d\n", Version);

	return false;
}
