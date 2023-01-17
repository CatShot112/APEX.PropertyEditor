#pragma once

#include <string>

namespace FileSystem {
    bool OpenFileDialog(std::string& currentFileName);
    bool SaveFileDialog(std::string& currentFileName);
}
