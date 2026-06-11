#pragma once

#include <algorithm>
#include <string>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <cassert>

namespace dedup
{

    /**
     * @brief Represents metadata and hash information for a scanned file.
     */
    struct FileInfo {
        std::string path;
        uint64_t size_bytes;
        std::string hash;
        std::filesystem::file_time_type last_modified;

        /**
         * @brief Constructs a new FileInfo object.
         * @param path The path to the file.
         * @param size_bytes The size of the file in bytes.
         * @param last_modified The last modification time of the file.
         */
        FileInfo(std::string path, uint64_t size_bytes, std::filesystem::file_time_type last_modified)
            : path(std::move(path)), size_bytes(size_bytes), hash(""), last_modified(last_modified) {}
    };

    /**
     * @brief Groups duplicate files and provides analysis and deduplication methods.
     */
    struct DuplicateGroup {
        /**
         * @brief The list of files in the duplicate group.
         */
        std::vector<FileInfo> files;

        /**
         * @brief Constructs a new DuplicateGroup object.
         * @param f A collection of files suspected or confirmed to be identical.
         */
        DuplicateGroup(std::vector<FileInfo> f) : files(std::move(f)) {
            assert(is_valid() && "DuplicateGroup created with mismatched files");
        }

        /**
         * @brief Validates the duplicate group by checking if all files have the same size and hash.
         * @return True if the group is valid and all files match, false otherwise.
         */
        bool is_valid() const
        {
            if (files.empty()) return false;
            const FileInfo& first = files[0];
            return std::all_of(files.begin(), files.end(),
                [&](const FileInfo& f) {
                    return f.size_bytes == first.size_bytes &&
                           f.hash == first.hash;
                });
        }

        /**
         * @brief Calculates the total number of bytes wasted by redundant copies.
         * @return The wasted bytes (size * (count - 1)).
         */
        uint64_t wasted_bytes() const
        {
            if (files.size() < 2) return 0;
            return (files.size() - 1) * files[0].size_bytes;
        }

        /**
         * @brief Returns the total number of files in the group.
         * @return The size of the file collection.
         */
        size_t count() const { return files.size(); }

        /**
         * @brief Returns the first file in the group as the primary copy.
         * @return A reference to the first FileInfo object.
         */
        const FileInfo& keep() const { return files[0]; }

        /**
         * @brief Returns all files in the group except the first one.
         * @return A vector of redundant FileInfo objects.
         */
        std::vector<FileInfo> duplicates() const
        {
            if (files.size() < 2) return {};
            return std::vector<FileInfo>(files.begin() + 1, files.end());
        }

        /**
         * @brief Determines the canonical (original) file to keep using an advanced heuristic.
         *
         * The heuristic selects:
         * 1. The file with the shortest path length (likely closer to the root or with a cleaner name).
         * 2. The oldest modified file (in case of path length ties).
         * 3. The lexicographically smallest path (to break any remaining ties).
         *
         * @return A reference to the canonical FileInfo object.
         * @throws std::runtime_error If the duplicate group is empty.
         */
        const FileInfo& get_canonical_file() const
        {
            if (files.empty())
                throw std::runtime_error("Cannot get canonical file from an empty DuplicateGroup");

            const FileInfo* canonical = &files[0];

            for (size_t i = 1; i < files.size(); ++i) {
                const auto& f = files[i];
                if (f.path.length() < canonical->path.length()) {
                    canonical = &f;
                }
                else if (f.path.length() == canonical->path.length()) {
                    if (f.last_modified < canonical->last_modified) {
                        canonical = &f;
                    }
                    else if (f.last_modified == canonical->last_modified) {
                        if (f.path < canonical->path) {
                            canonical = &f;
                        }
                    }
                }
            }

            return *canonical;
        }

        /**
         * @brief Returns all redundant copies of the file, excluding the canonical copy.
         * @return A vector of redundant FileInfo objects.
         */
        std::vector<FileInfo> get_redundant_files() const
        {
            if (files.size() < 2) return {};
            const FileInfo& canonical = get_canonical_file();

            std::vector<FileInfo> redundant;
            redundant.reserve(files.size() - 1);

            for (const auto& f : files) {
                if (f.path != canonical.path) redundant.push_back(f);
            }

            return redundant;
        }

        /**
         * @brief Checks if the group contains only a single file (no duplicates).
         * @return True if there is exactly one file, false otherwise.
         */
        bool is_single() const { return files.size() == 1; }

        /**
         * @brief Checks if the group contains any duplicate files.
         * @return True if there are two or more files, false otherwise.
         */
        bool has_duplicates() const { return files.size() > 1; }

        /**
         * @brief Returns a short human-readable summary of the duplicate group.
         * @return A string summary.
         */
        std::string summary() const
        {
            return std::to_string(files.size()) + " files, " +
                   std::to_string(wasted_bytes()) + " bytes wasted";
        }
    };
}
