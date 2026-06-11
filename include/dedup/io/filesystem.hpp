#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace dedup {
namespace io {

// Cross-platform filesystem operation stubs
class Filesystem {
public:
    static bool exists(const std::string& path);
    static uint64_t fileSize(const std::string& path);
    static bool isRegularFile(const std::string& path);
};

} // namespace io
} // namespace dedup
