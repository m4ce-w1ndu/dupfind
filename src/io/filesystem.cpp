#include "dedup/io/filesystem.hpp"

namespace dedup {
namespace io {

bool Filesystem::exists(const std::string& path) {
    return false;
}

uint64_t Filesystem::fileSize(const std::string& path) {
    return 0;
}

bool Filesystem::isRegularFile(const std::string& path) {
    return false;
}

} // namespace io
} // namespace dedup
