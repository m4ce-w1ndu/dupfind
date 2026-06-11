#pragma once

#include "types.hpp"
#include <string>
#include <array>
#include <unordered_map>
#include <vector>

namespace dedup {

/**
 * @class FileHasher
 * @brief Computes cryptographic hashes (SHA-256) of files, with support for partial hashing and in-memory caching.
 *
 * This class provides methods to calculate full file hashes as well as fast partial hashes (the first block of a file)
 * to optimize duplicate detection. It optionally caches full hashes in memory to prevent redundant file reads.
 */
class FileHasher
{
public:
    /**
     * @brief Constructs a new FileHasher object.
     * @param partial_size_bytes The size of the first block to read for partial hashing. Defaults to 4096 bytes (4 KB).
     */
    explicit FileHasher(size_t partial_size_bytes = 4096);

    /**
     * @brief Computes the SHA-256 hash of the first partial block of the file.
     * @param path The path to the file.
     * @return The lowercase hexadecimal representation of the partial SHA-256 hash.
     */
    std::string compute_partial_hash(const std::string& path);

    /**
     * @brief Computes the SHA-256 hash of the entire file.
     * @param path The path to the file.
     * @return The lowercase hexadecimal representation of the full SHA-256 hash.
     */
    std::string compute_full_hash(const std::string& path);

    /**
     * @brief Checks if two files are identical in content.
     * @param path_a The path to the first file.
     * @param path_b The path to the second file.
     * @return True if the contents are identical, false otherwise.
     */
    bool are_identical(const std::string& path_a, const std::string& path_b);

    /**
     * @brief Clears the in-memory cache of calculated full hashes.
     */
    void clear_cache();

    /**
     * @brief Enables or disables in-memory hash caching for subsequent full hash computations.
     * @param enabled True to enable caching, false to disable.
     */
    void set_caching_enabled(bool enabled) { caching_enabled_ = enabled; }

private:
    /// Size of the block used for partial hashing (in bytes).
    size_t partial_size_;

    /// Controls whether calculated full hashes are cached in the cache_ map.
    bool caching_enabled_ = true;

    /// In-memory cache mapping file paths to their raw 32-byte SHA-256 hashes.
    std::unordered_map<std::string, std::array<uint8_t, 32>> cache_;

    /// In-memory cache mapping file paths to their raw 32-byte partial SHA-256 hashes.
    std::unordered_map<std::string, std::array<uint8_t, 32>> partial_cache_;

    /**
     * @brief Internal helper to calculate the raw SHA-256 hash of a file.
     * @param filepath The path to the file.
     * @param full True to hash the entire file, false to hash only the first partial_size_ bytes.
     * @return A 32-byte array containing the raw binary SHA-256 hash.
     */
    std::array<uint8_t, 32> compute_raw_hash(const std::string& filepath, bool full);

    /**
     * @brief Converts a raw 32-byte binary hash into a lowercase hexadecimal string.
     * @param hash The raw 32-byte hash array.
     * @return A 64-character lowercase hexadecimal string.
     */
    static std::string to_hex_string(const std::array<uint8_t, 32>& hash);

    /**
     * @brief Reads a specific chunk of bytes from a file.
     * @param path The path to the file.
     * @param offset The starting byte offset.
     * @param size The number of bytes to read.
     * @return A vector containing the read bytes.
     */
    std::vector<uint8_t> read_chunk(const std::string& path, size_t offset, size_t size);
};

}
