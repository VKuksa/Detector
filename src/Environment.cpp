#include "Environment.h"

#include <windows.h>

namespace {
    std::filesystem::path getExecPath() {
#ifdef _WIN32
        std::vector<TCHAR> buffer;
        DWORD copied;
        do {
            buffer.resize(buffer.size() + MAX_PATH);
            copied = GetModuleFileName(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        } while (copied >= buffer.size());

        LPCTSTR path = buffer.data();

        return std::filesystem::path(path);
#else
        char result[PATH_MAX] = {0};
        readlink("/proc/self/exe", result, PATH_MAX);
        return std::filesystem::path(result);
#endif
    }
}

std::filesystem::path Environment::rootDir() {
    return binaryDir().parent_path();
}

std::filesystem::path Environment::resourcesDir() {
    return rootDir() / std::string("resources");
}

std::filesystem::path Environment::binaryDir() {
    return getExecPath().parent_path();
}
