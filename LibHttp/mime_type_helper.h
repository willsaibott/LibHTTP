#pragma once

#include <boost/utility/string_view.hpp>
#include <unordered_map>

namespace helper {

    // Return a reasonable mime type based on the extension of a file.
    inline boost::string_view
    mime_type(std::string_view extension) {
        static const std::unordered_map<std::string_view, std::string_view>
            mime_types {
                { ".htm",   "text/html" },
                { ".html",  "text/html" },
                { ".php",   "text/html" },
                { ".css",   "text/css" },
                { ".txt",   "text/plain" },
                { ".js",    "application/javascript" },
                { ".json",  "application/json" },
                { ".xml",   "application/xml" },
                { ".swf",   "application/x-shockwave-flash" },
                { ".flv",   "video/x-flv" },
                { ".png",   "image/png" },
                { ".jpe",   "image/jpeg" },
                { ".jpeg",  "image/jpeg" },
                { ".jpg",   "image/jpeg" },
                { ".gif",   "image/gif" },
                { ".bmp",   "image/bmp" },
                { ".ico",   "image/vnd.microsoft.icon" },
                { ".tiff",  "image/tiff" },
                { ".tif",   "image/tiff" },
                { ".svg",   "image/svg+xml" },
                { ".svgz",  "image/svg+xml" },
            };
        std::string_view response { "application/text"  };
        auto it = mime_types.find(extension);
        if (it != mime_types.end()) {
            response = it->second;
        }
        return { response.data(), response.size() };
    }

}
