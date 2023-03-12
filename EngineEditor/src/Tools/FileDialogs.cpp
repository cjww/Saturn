#include "FileDialogs.h"
#include "Application.h"

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif // !_WIN32


bool sa::FileDialogs::OpenFile(const char* filter, std::filesystem::path& filePath, const std::filesystem::path& initialPath) {

#ifdef _WIN32
    CHAR szfile[260] = { 0 };

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = (HWND)Application::get()->getWindow()->getWin32WindowHandle();
    ofn.lpstrFile = szfile;
    ofn.nMaxFile = sizeof(szfile);
    
    ofn.lpstrInitialDir = initialPath.generic_string().c_str();

    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn)) {
        filePath = ofn.lpstrFile;
        return true;
    }
    
#endif
    return false;
}

bool sa::FileDialogs::SaveFile(const char* filter, std::filesystem::path& filePath, const std::filesystem::path& initialPath) {

#ifdef _WIN32
    CHAR szfile[260] = { 0 };

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = (HWND)Application::get()->getWindow()->getWin32WindowHandle();
    ofn.lpstrFile = szfile;
    ofn.nMaxFile = sizeof(szfile);

    ofn.lpstrInitialDir = initialPath.generic_string().c_str();

    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetSaveFileNameA(&ofn)) {
        filePath = ofn.lpstrFile;
        return true;
    }
#endif
    return false;
}
