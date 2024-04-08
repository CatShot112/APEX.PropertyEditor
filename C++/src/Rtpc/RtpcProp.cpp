#include "RtpcProp.hpp"

#include <string>

RtpcProp::RtpcProp() {
    HashedName = 0;
    DataRaw = 0;
    Type = 0;

    Offset = 0;
}

bool RtpcProp::Deserialize(std::ifstream& file, bool handleShared) {
    // Read header
    file.read((char*)&HashedName, sizeof(u32));
    file.read((char*)&DataRaw, sizeof(u32));
    file.read((char*)&Type, sizeof(u8));

    // Save current read position
    std::streamoff oPos = file.tellg();

    // Seek to data if required
    if (Type != 0 && Type != 1 && Type != 2)
        file.seekg(DataRaw);

    switch (Type) {
    case 0: // None
    case 1: // U32
    case 2: // F32
        break;
    case 3: // Str
        std::getline(file, DataStr, '\0');
        break;
    case 4: // Vec2
        DataFinal.Resize(sizeof(float) * 2);
        file.read((char*)&DataFinal[0], sizeof(float) * 2);
        break;
    case 5: // Vec3
        DataFinal.Resize(sizeof(float) * 3);
        file.read((char*)&DataFinal[0], sizeof(float) * 3);
        break;
    case 6: // Vec4
        DataFinal.Resize(sizeof(float) * 4);
        file.read((char*)&DataFinal[0], sizeof(float) * 4);
        break;
    case 7: // Mat3x3
        DataFinal.Resize(sizeof(float) * 3 * 3);
        file.read((char*)&DataFinal[0], sizeof(float) * 3 * 3);
        break;
    case 8: // Mat4x4
        DataFinal.Resize(sizeof(float) * 4 * 4);
        file.read((char*)&DataFinal[0], sizeof(float) * 4 * 4);
        break;
    case 13: // ObjID
        DataFinal.Resize(sizeof(u64));
        file.read((char*)&DataFinal[0], sizeof(u64));
        break;
    case 9: // A[U32]
    {
        u32 arraySize = 0;
        file.read((char*)&arraySize, sizeof(u32));

        if (arraySize) {
            DataFinal.Resize(sizeof(u32) * arraySize);
            file.read((char*)&DataFinal[0], sizeof(u32) * arraySize);
        }

        break;
    }
    case 10: // A[F32]
    {
        u32 arraySize = 0;
        file.read((char*)&arraySize, sizeof(u32));

        if (arraySize) {
            DataFinal.Resize(sizeof(float) * arraySize);
            file.read((char*)&DataFinal[0], sizeof(float) * arraySize);
        }

        break;
    }
    case 11: // A[U8]
    {
        u32 arraySize = 0;
        file.read((char*)&arraySize, sizeof(u32));

        if (arraySize) {
            DataFinal.Resize(sizeof(u8) * arraySize);
            file.read((char*)&DataFinal[0], sizeof(u8) * arraySize);
        }

        break;
    }
    case 14: // Event (A[U64])
    {
        u32 arraySize = 0;
        file.read((char*)&arraySize, sizeof(u32));

        if (arraySize) {
            DataFinal.Resize(sizeof(u64) * arraySize);
            file.read((char*)&DataFinal[0], sizeof(u64) * arraySize);
        }

        break;
    }
    case 15:
    case 16:
    default:
        printf("[WARN]: Property type not handled: %d\n", Type);
        break;;
    }

    // Go to saved read position
    file.seekg(oPos);

    return true;
}
