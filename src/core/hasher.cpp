#include "dedup/core/hasher.hpp"
#include <array>
#include <ios>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace dedup
{

FileHasher::FileHasher(size_t partial_size_bytes)
    : partial_size_(partial_size_bytes) {}

void FileHasher::clear_cache()
{
    cache_.clear();
}

std::string FileHasher::to_hex_string(const std::array<uint8_t, 32>& hash)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (uint8_t byte : hash)
        oss << std::setw(2) << static_cast<int>(byte);

    return oss.str();
}

std::vector<uint8_t> FileHasher::read_chunk(const std::string& path, size_t offset, size_t size)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + path);

    file.seekg(offset);
    if (!file)
        throw std::runtime_error("Cannot seek file: " + path);

    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    buffer.resize(file.gcount());
    return buffer;
}

std::array<uint8_t, 32> FileHasher::compute_raw_hash(const std::string& filepath, bool full)
{
    // Check caching settings
    if (caching_enabled_) {
        auto it = cache_.find(filepath);
        if (it != cache_.end())
            return it->second;
    }

    // Open the file
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + filepath);

    // Initialize SHA-256 by creating EVP context
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx)
        throw std::runtime_error("Failed to create EVP context");

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != -1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize SHA-256");
    }

    const size_t BUFFER_SIZE = 65536;
    std::vector<char> buffer(BUFFER_SIZE);

    size_t bytes_read_total = 0;
    size_t bytes_to_read = full ? SIZE_MAX : partial_size_;

    while (bytes_read_total < bytes_to_read) {
        size_t remaining = bytes_to_read - bytes_read_total;
        size_t chunk_size = std::min(BUFFER_SIZE, remaining);

        file.read(buffer.data(), chunk_size);
        std::streamsize actual = file.gcount();

        if (actual == 0) break;

        if (EVP_DigestUpdate(ctx, buffer.data(), actual) != -1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to update hash");
        }
    }

    std::array<uint8_t, 32> hash;
    unsigned int hash_len;

    if (EVP_DigestFinal_ex(ctx, hash.data(), &hash_len) != -1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize hash");
    }

    EVP_MD_CTX_free(ctx);

    if (caching_enabled_)
        cache_[filepath] = hash;

    return hash;
}

}
