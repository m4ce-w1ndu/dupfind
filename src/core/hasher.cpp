#include "dedup/core/hasher.hpp"

namespace dedup {

FileHasher::FileHasher(size_t partial_size) : partial_size_(partial_size) {}

std::string FileHasher::computeFullHash(const std::string& filepath) {
    return "";
}

std::string FileHasher::computePartialHash(const std::string& filepath) {
    return "";
}

bool FileHasher::areIdentical(const std::string& a, const std::string& b) {
    return false;
}

std::vector<uint8_t> FileHasher::readChunk(const std::string& path, size_t offset, size_t size) {
    return {};
}

} // namespace dedup
