#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <functional>

namespace dedup {

// Progress callback for UI updates
using ProgressCallback = std::function<void(
    const std::string& current_file,
    size_t files_processed,
    size_t total_files_estimated
)>;

struct FileInfo {
    std::string path;
    uint64_t size_bytes;
    std::string hash;  // hex string, empty until hashed
    std::chrono::system_clock::time_point last_modified;
    
    bool operator==(const FileInfo& other) const {
        return size_bytes == other.size_bytes && hash == other.hash;
    }
};

struct DuplicateGroup {
    std::vector<FileInfo> files;
    uint64_t wasted_bytes;  // (count - 1) * size
    
    DuplicateGroup() : wasted_bytes(0) {}
};

enum class ActionType {
    ReportOnly,
    Hardlink,
    MoveToTrash,
    Delete
};

struct ScanOptions {
    std::vector<std::string> paths;
    uint64_t min_file_size = 1024;        // Ignore files < 1KB
    bool follow_symlinks = false;
    bool partial_hash_first = true;        // Hash first 4KB only
    size_t thread_count = 4;
};

} // namespace dedup
