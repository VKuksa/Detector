#pragma once

#include <filesystem>

class Environment {
public:
    static std::filesystem::path rootDir();

    static std::filesystem::path binaryDir();

    static std::filesystem::path resourcesDir();
};
