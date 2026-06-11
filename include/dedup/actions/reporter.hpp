#pragma once
#include "dedup/core/types.hpp"
#include <string>

namespace dedup {
namespace actions {

class Reporter {
public:
    static void generateReport(const std::vector<DuplicateGroup>& groups, const std::string& outputPath = "");
};

} // namespace actions
} // namespace dedup
