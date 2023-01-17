#include "FileSystem.hpp"

// Windows includes
#include <Windows.h>
#include <ShObjIdl.h>


std::string utf8_encode(std::wstring wstr) {
    if (wstr.empty())
        return std::string();

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], sizeNeeded, nullptr, nullptr);

    return str;
}

std::wstring utf8_decode(std::string str) {
    if (str.empty())
        return std::wstring();

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
    std::wstring wstr(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF7, 0, &str[0], (int)str.size(), &wstr[0], sizeNeeded);

    return wstr;
}

namespace FileSystem {
    bool OpenFileDialog(std::string& currentFileName) {
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
                            currentFileName = utf8_encode(pszFilePath);

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

    bool SaveFileDialog(std::string& currentFileName) {
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
                pFileSave->SetFileName(utf8_decode(currentFileName.substr(currentFileName.rfind("\\") + 1)).c_str());

                if (SUCCEEDED(pFileSave->Show(nullptr))) {
                    IShellItem* pItem = nullptr;

                    if (SUCCEEDED(pFileSave->GetResult(&pItem))) {
                        PWSTR pszFilePath = nullptr;

                        if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
                            currentFileName = utf8_encode(pszFilePath);

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
