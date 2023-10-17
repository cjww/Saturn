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

bool sa::FileDialogs::OpenFileInTextEditor(const std::filesystem::path& applicationPath, const std::filesystem::path& file) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);

    std::wstring commandLine = L"\"" + applicationPath.wstring() + L"\" " + file.generic_wstring();

	bool success = CreateProcess(NULL,   // the path
        commandLine.data(),           // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return success;
}
