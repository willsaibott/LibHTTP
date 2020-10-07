#pragma once


#include <string>

namespace helper {

    inline std::string
    file_extension(const std::string& path) {
        const auto pos = path.rfind(".");
        std::string extension;
        if (pos != std::string::npos) {
          extension = path.substr(pos);
        }
        return extension;
    }
}