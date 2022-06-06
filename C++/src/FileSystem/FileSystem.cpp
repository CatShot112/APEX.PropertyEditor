#include "FileSystem.hpp"

// Windows includes
#include <Windows.h>
#include <ShObjIdl.h>

namespace FileSystem {
	bool OpenFileDialog(std::wstring& currentFileName) {
        bool result = false;

        if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
            IFileOpenDialog* pFileOpen = nullptr;

            if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen)))) {
                COMDLG_FILTERSPEC rgSpec[] = {
                    { L"All Files", L"*.*"},
                    { L"Binary Files", L"*.bin"}
                };

                pFileOpen->SetFileTypes(2, rgSpec);

                if (SUCCEEDED(pFileOpen->Show(nullptr))) {
                    IShellItem* pItem = nullptr;

                    if (SUCCEEDED(pFileOpen->GetResult(&pItem))) {
                        PWSTR pszFilePath = nullptr;

                        if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
                            currentFileName = pszFilePath;

                            result = true;

                            CoTaskMemFree(pszFilePath);
                        }

                        pItem->Release();
                    }
                }

                pFileOpen->Release();
            }

            CoUninitialize();
        }

        return result;
	}

    bool SaveFileDialog(std::wstring& currentFileName) {
        bool result = false;

        if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
            IFileSaveDialog* pFileSave = nullptr;

            if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave)))) {
                COMDLG_FILTERSPEC rgSpec[] = {
                    { L"All Files", L"*.*"},
                    { L"Binary Files", L"*.bin"}
                };

                pFileSave->SetFileTypes(2, rgSpec);
                pFileSave->SetDefaultExtension(L"bin");
                pFileSave->SetFileName(currentFileName.substr(currentFileName.rfind(L"\\") + 1).c_str());

                if (SUCCEEDED(pFileSave->Show(nullptr))) {
                    IShellItem* pItem = nullptr;

                    if (SUCCEEDED(pFileSave->GetResult(&pItem))) {
                        PWSTR pszFilePath = nullptr;

                        if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
                            currentFileName = pszFilePath;

                            result = true;

                            CoTaskMemFree(pszFilePath);
                        }

                        pItem->Release();
                    }
                }

                pFileSave->Release();

            }

            CoUninitialize();
        }

        return result;
	}
}
