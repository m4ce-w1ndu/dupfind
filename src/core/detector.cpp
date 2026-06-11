#include "dedup/core/detector.hpp"

namespace dedup {

DuplicateDetector::DuplicateDetector(const ScanOptions& options) : options_(options) {}

std::vector<DuplicateGroup> DuplicateDetector::findDuplicates(ProgressCallback progress) {
    return {};
}

std::unordered_map<uint64_t, std::vector<std::string>> DuplicateDetector::groupBySize() {
    return {};
}

std::unordered_map<std::string, std::vector<std::string>> 
DuplicateDetector::groupByPartialHash(const std::unordered_map<uint64_t, std::vector<std::string>>& size_groups) {
    return {};
}

std::vector<DuplicateGroup> DuplicateDetector::resolveWithFullHash(
    const std::unordered_map<std::string, std::vector<std::string>>& partial_groups) {
    return {};
}

void DuplicateDetector::collectFiles(const std::string& path, std::vector<std::string>& out_files) {}

} // namespace dedup
