#pragma once

#include <string>

namespace FileSystem {
	extern bool OpenFileDialog(std::wstring& currentFileName);
	extern bool SaveFileDialog(std::wstring& currentFileName);
}
