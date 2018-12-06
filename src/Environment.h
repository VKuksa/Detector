//
// Created by vkuksa on 06.12.2018.
//

#ifndef DETECTOR_ENVIRONMENT_H
#define DETECTOR_ENVIRONMENT_H

#include <filesystem>

class Environment {
public:
    static std::filesystem::path rootDir();

    static std::filesystem::path binaryDir();

    static std::filesystem::path resourcesDir();
};


#endif //DETECTOR_ENVIRONMENT_H
