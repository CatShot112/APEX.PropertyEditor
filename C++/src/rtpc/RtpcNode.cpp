#include "RtpcNode.hpp"

vector<string> writtenStrings;

RtpcNode::RtpcNode() {
	HashedName = 0;
	DataOffset = 0;
	PropsCount = 0;
	ChildCount = 0;
}

void RtpcNode::Clear() {
	HashedName = 0;
	DataOffset = 0;
	PropsCount = 0;
	ChildCount = 0;

	props.clear();
	childs.clear();
}

void RtpcNode::WritePadding(std::ofstream& file) {
	std::streamoff pos = file.tellp();
	int add = (4 - (pos % 4)) % 4;

	for (int p = 0; p < add; p++)
		file.write("\x50", 1);
}

bool RtpcNode::Deserialize(std::ifstream& file) {
	// Read header
	file.read((char*)&HashedName, sizeof(u32));
	file.read((char*)&DataOffset, sizeof(u32));
	file.read((char*)&PropsCount, sizeof(u16));
	file.read((char*)&ChildCount, sizeof(u16));

	// Save current read position and go to node data
	std::streamoff oPos = file.tellg();
	file.seekg(DataOffset);

	// Read props headers
	for (u16 i = 0; i < PropsCount; i++) {
		RtpcProp prop{};

		prop.Deserialize(file);
		props.emplace_back(prop);
	}

	// 4-byte align after props headers
	std::streamoff pos = file.tellg();
	file.seekg(pos + (4 - (pos % 4)) % 4);

	// Read child nodes headers
	for (u16 i = 0; i < ChildCount; i++) {
		RtpcNode child;

		child.Deserialize(file);
		childs.emplace_back(child);
	}

	// Go to saved read position
	file.seekg(oPos);

	return true;
}

bool RtpcNode::Serialize_V1(std::ofstream& file, bool writeSelf) {
	
	return false;
}

bool RtpcNode::Serialize_V2(std::ofstream& file, bool writeSelf) {
	// Write header
	if (writeSelf) {
		file.write((char*)&HashedName, sizeof(u32));
		file.write((char*)&DataOffset, sizeof(u32));
		file.write((char*)&PropsCount, sizeof(u16));
		file.write((char*)&ChildCount, sizeof(u16));

#ifdef _DEBUG
		file.flush(); // DEBUG
#endif
	}

	// Write props headers and count valid props
	u32 cnt = 0;

	for (u16 i = 0; i < PropsCount; i++) {
		file.write((char*)&props[i].HashedName, sizeof(u32));
		file.write((char*)&props[i].DataRaw, sizeof(u32));
		file.write((char*)&props[i].Type, sizeof(u8));

		if (props[i].Type != 0)
			cnt++;

#ifdef _DEBUG
		file.flush(); // DEBUG
#endif
	}

	// 4-byte align after props headers
	if (PropsCount)
		WritePadding(file);

	// Write childs headers
	for (u16 i = 0; i < ChildCount; i++) {
		file.write((char*)&childs[i].HashedName, sizeof(u32));
		file.write((char*)&childs[i].DataOffset, sizeof(u32));
		file.write((char*)&childs[i].PropsCount, sizeof(u16));
		file.write((char*)&childs[i].ChildCount, sizeof(u16));

#ifdef _DEBUG
		file.flush(); // DEBUG
#endif
	}

	// Write number of valid props
	file.write((char*)&cnt, sizeof(u32));

	// Write props data
	for (u16 i = 0; i < PropsCount; i++) {
		switch (props[i].Type) {
		case 4: // Vec2
			file.write((char*)&props[i].DataFinal[0], sizeof(float) * 2);
			break;
		case 5: // Vec3
			file.write((char*)&props[i].DataFinal[0], sizeof(float) * 3);
			break;
		case 6: // Vec4
			file.write((char*)&props[i].DataFinal[0], sizeof(float) * 4);
			break;
		case 7: // Mat3x3
			file.write((char*)&props[i].DataFinal[0], sizeof(float) * 3 * 3);
			break;
		case 8: // Mat4x4
			file.write((char*)&props[i].DataFinal[0], sizeof(float) * 4 * 4);
			break;
		case 13:
			file.write((char*)&props[i].DataFinal[0], sizeof(u64));
			break;
		case 9: // A[U32]
		{
			u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(u32);
			file.write((char*)&arraySize, sizeof(u32));

			if (arraySize)
				file.write((char*)&props[i].DataFinal[0], sizeof(u32) * arraySize);

			break;
		}
		case 10: // A[F32]
		{
			u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(float);
			file.write((char*)&arraySize, sizeof(u32));

			if (arraySize)
				file.write((char*)&props[i].DataFinal[0], sizeof(float) * arraySize);

			break;
		}
		case 11: // A[U8]
		{
			u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(u8);
			file.write((char*)&arraySize, sizeof(u32));

			if (arraySize)
				file.write((char*)&props[i].DataFinal[0], sizeof(u8) * arraySize);

			break;
		}
		case 14: // Event
		{
			u32 arraySize = (u32)props[i].DataFinal.GetSize() / sizeof(u64);
			file.write((char*)&arraySize, sizeof(u32));

			if (arraySize)
				file.write((char*)&props[i].DataFinal[0], sizeof(u64) * arraySize);

			break;
		}
		case 3: // Str
		{
			// Make sure we only write one instance of a string
			if (!std::count(writtenStrings.begin(), writtenStrings.end(), props[i].DataStr)) {
				file.write(props[i].DataStr.c_str(), strlen(props[i].DataStr.c_str()) + 1);
				writtenStrings.emplace_back(props[i].DataStr);

#ifdef _DEBUG
				file.flush(); // DEBUG
#endif
			}

			// We need to align strings to 4 bytes before other data type (except none), and if string is last property
			// TODO: Comment this properly :)
			if (i == PropsCount - 1)
				WritePadding(file);

			if (i + 1 <= PropsCount - 1) {
				bool pad = true;

				if (props[i + 1].Type == 0) {
					for (u16 j = i + 1; j < PropsCount - 1; j++) {
						if (props[j + 1].Type == 3)
							pad = false;
					}
				}

				if (pad && props[i + 1].Type != 3)
					WritePadding(file);
			}

			break;
		}
		default:
			break;
		}

#ifdef _DEBUG
		file.flush(); // DEBUG
#endif
	}

	// Write child nodes
	for (u16 i = 0; i < ChildCount; i++)
		childs[i].Serialize_V2(file, false);

	return true;
}

bool RtpcNode::Serialize_V3(std::ofstream& file, bool writeSelf) {

	return false;
}
