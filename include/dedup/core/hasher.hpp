#pragma once
#include "types.hpp"
#include <string>
#include <vector>

namespace dedup {

class FileHasher {
public:
    explicit FileHasher(size_t partial_size = 4096);
    
    // Returns hex string of SHA-256
    std::string computeFullHash(const std::string& filepath);
    
    // For fast pre-filtering (first N bytes)
    std::string computePartialHash(const std::string& filepath);
    
    // Check if two files are identical without full hash
    bool areIdentical(const std::string& a, const std::string& b);
    
private:
    size_t partial_size_;
    
    // Internal: read file chunk and return bytes
    std::vector<uint8_t> readChunk(const std::string& path, size_t offset, size_t size);
};

} // namespace dedup
