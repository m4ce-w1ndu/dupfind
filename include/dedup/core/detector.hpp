#pragma once
#include "types.hpp"
#include "hasher.hpp"
#include <vector>
#include <unordered_map>
#include <memory>

namespace dedup {

class DuplicateDetector {
public:
    DuplicateDetector(const ScanOptions& options);
    
    // Main entry point
    std::vector<DuplicateGroup> findDuplicates(ProgressCallback progress = nullptr);
    
    // Get scan statistics
    uint64_t totalBytesScanned() const { return total_bytes_; }
    size_t totalFilesScanned() const { return total_files_; }
    
private:
    ScanOptions options_;
    FileHasher hasher_;
    uint64_t total_bytes_ = 0;
    size_t total_files_ = 0;
    
    // Step 1: Group by size (fastest filter)
    std::unordered_map<uint64_t, std::vector<std::string>> groupBySize();
    
    // Step 2: Group by partial hash
    std::unordered_map<std::string, std::vector<std::string>> 
        groupByPartialHash(const std::unordered_map<uint64_t, std::vector<std::string>>& size_groups);
    
    // Step 3: Full hash for remaining conflicts
    std::vector<DuplicateGroup> resolveWithFullHash(
        const std::unordered_map<std::string, std::vector<std::string>>& partial_groups);
    
    void collectFiles(const std::string& path, std::vector<std::string>& out_files);
};

} // namespace dedup
