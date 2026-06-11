#pragma once

#include <algorithm>
#include <string>
#include <cstdint>

namespace dedup {

    /**
     * @brief Represents metadata and hash information for a scanned file.
     */
    struct FileInfo {
        std::string file_path;
        uint64_t file_size;
        std::string file_hash;

        /**
         * @brief Constructs a new FileInfo object.
         * @param file_path The path to the file.
         * @param file_size The size of the file in bytes.
         */
        FileInfo(std::string file_path, uint64_t file_size)
            : file_path(std::move(file_path)), file_size(file_size), file_hash("") {}
    };

}
