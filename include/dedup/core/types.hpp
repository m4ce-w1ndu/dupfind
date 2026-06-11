#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace dedup {

    /**
     * @brief Represents metadata and hash information for a scanned file.
     */
    struct FileInfo {
        std::string path;
        uint64_t size_bytes;
        std::string hash;
        uint64_t last_modified;

        /**
         * @brief Constructs a new FileInfo object.
         * @param path The path to the file.
         * @param size_bytes The size of the file in bytes.
         */
        FileInfo(std::string path, uint64_t size_bytes)
            : path(std::move(path)), size_bytes(size_bytes), hash(""), last_modified(0) {}
    };

}
