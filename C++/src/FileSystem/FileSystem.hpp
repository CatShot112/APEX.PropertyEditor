#pragma once

#include <string>

namespace FileSystem {
	bool OpenFileDialog(std::wstring& currentFileName);
	bool SaveFileDialog(std::wstring& currentFileName);
}
