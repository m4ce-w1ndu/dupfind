#include <filesystem>
#include <gtest/gtest.h>
#include "dedup/core/types.hpp"

static dedup::FileInfo make_file(const std::string& path, uint64_t size, const std::string& hash = "",
                   std::filesystem::file_time_type last_modified = std::filesystem::file_time_type::min()) {
    dedup::FileInfo file(path, size, last_modified);
    file.hash = hash;
    return file;
}

// ============================================================================
// FileInfo tests
// ============================================================================

TEST(FileInfoTest, FullConstructorInitializesCorrectly)
{
    auto now = std::filesystem::file_time_type::clock::now();
    dedup::FileInfo file("/path/to/file.txt", 1024, now);

    EXPECT_EQ(file.path, "/path/to/file.txt");
    EXPECT_EQ(file.size_bytes, 1024);
    EXPECT_EQ(file.hash, "");
    EXPECT_EQ(file.last_modified, now);
}

TEST(FileInfoTest, TwoParamsConstructoWorks)
{
    dedup::FileInfo file("/test.txt", 100);

    EXPECT_EQ(file.path, "/test.txt");
    EXPECT_EQ(file.size_bytes, 100);
    EXPECT_EQ(file.hash, "");
    EXPECT_EQ(file.last_modified, std::filesystem::file_time_type::min());
}

TEST(FileInfoTest, HashCanBeSetAfterConstruction)
{
    dedup::FileInfo file("/test.txt", 100);
    file.hash = "abc123def456";

    EXPECT_EQ(file.hash, "abc123def456");
}

// ============================================================================
// DuplicateGroup construction tests
// ============================================================================

TEST(DuplicateGroupTest, CannotCreateEmptyGroup) {
    std::vector<dedup::FileInfo> empty;

    // In debug mode, assert will trigger
    // In release, behavior is undefined but we test the assertion exists
    #ifndef NDEBUG
    EXPECT_DEATH({ dedup::DuplicateGroup group(empty); }, ".*");
    #endif
}

TEST(DuplicateGroupTest, SingleFileGroupWorks) {
    std::vector<dedup::FileInfo> files;
    files.push_back(make_file("/file.txt", 100, "hash123"));

    dedup::DuplicateGroup group(std::move(files));

    EXPECT_TRUE(group.is_valid());
    EXPECT_EQ(group.count(), 1);
    EXPECT_EQ(group.wasted_bytes(), 0);
    EXPECT_EQ(group.keep().path, "/file.txt");
}

TEST(DuplicateGroupTest, TwoIdenticalFilesAreValid) {
    std::vector<dedup::FileInfo> files;
    files.push_back(make_file("/file1.txt", 100, "samehash"));
    files.push_back(make_file("/file2.txt", 100, "samehash"));

    dedup::DuplicateGroup group(std::move(files));

    EXPECT_TRUE(group.is_valid());
    EXPECT_EQ(group.count(), 2);
    EXPECT_EQ(group.wasted_bytes(), 100);
}

TEST(DuplicateGroupTest, ThreeIdenticalFilesWasteCorrectAmount) {
    std::vector<dedup::FileInfo> files;
    for (int i = 0; i < 3; i++) {
        files.push_back(make_file("/file" + std::to_string(i) + ".txt", 500, "samehash"));
    }

    dedup::DuplicateGroup group(std::move(files));

    EXPECT_EQ(group.wasted_bytes(), 1000);  // (3-1) * 500
    EXPECT_EQ(group.count(), 3);
}

TEST(DuplicateGroupTest, DifferentSizesTriggerAssert) {
    std::vector<dedup::FileInfo> files;
    files.push_back(make_file("/file1.txt", 100, "hash1"));
    files.push_back(make_file("/file2.txt", 200, "hash1"));  // Different size

    #ifndef NDEBUG
    EXPECT_DEATH({ dedup::DuplicateGroup group(std::move(files)); }, ".*");
    #endif
}

TEST(DuplicateGroupTest, DifferentHashesTriggerAssert) {
    std::vector<dedup::FileInfo> files;
    files.push_back(make_file("/file1.txt", 100, "hash1"));
    files.push_back(make_file("/file2.txt", 100, "hash2"));  // Different hash

    #ifndef NDEBUG
    EXPECT_DEATH({ dedup::DuplicateGroup group(std::move(files)); }, ".*");
    #endif
}

// ============================================================================
// DuplicateGroup accessor tests
// ============================================================================

TEST(DuplicateGroupTest, KeepReturnsFirstFile) {
    std::vector<dedup::FileInfo> files;
    files.push_back(make_file("/first.txt", 100, "samehash"));
    files.push_back(make_file("/second.txt", 100, "samehash"));

    dedup::DuplicateGroup group(std::move(files));

    EXPECT_EQ(group.keep().path, "/first.txt");
}

TEST(DuplicateGroupTest, DuplicatesReturnsAllButFirst) {
    std::vector<dedup::FileInfo> files;
    files.push_back(make_file("/keep.txt", 100, "samehash"));
    files.push_back(make_file("/dup1.txt", 100, "samehash"));
    files.push_back(make_file("/dup2.txt", 100, "samehash"));

    dedup::DuplicateGroup group(std::move(files));

    auto duplicates = group.duplicates();
    EXPECT_EQ(duplicates.size(), 2);
    EXPECT_EQ(duplicates[0].path, "/dup1.txt");
    EXPECT_EQ(duplicates[1].path, "/dup2.txt");
}

// ============================================================================
// Canonical file selection tests
// ============================================================================

TEST(DuplicateGroupTest, CanonicalFilePrefersShortestPath) {
    std::vector<dedup::FileInfo> files;
    files.push_back(make_file("/very/long/path/to/file.txt", 100, "hash"));
    files.push_back(make_file("/short.txt", 100, "hash"));

    dedup::DuplicateGroup group(std::move(files));

    const auto& canonical = group.get_canonical_file();
    EXPECT_EQ(canonical.path, "/short.txt");
}

TEST(DuplicateGroupTest, CanonicalFilePrefersOldestWhenPathsEqualLength) {
    auto now = std::filesystem::file_time_type::clock::now();
    auto older = now - std::chrono::hours(24);
    auto newer = now;

    std::vector<dedup::FileInfo> files;
    files.push_back(make_file("/path/file.txt", 100, "hash", newer));
    files.push_back(make_file("/path/file.txt", 100, "hash", older));

    dedup::DuplicateGroup group(std::move(files));

    const auto& canonical = group.get_canonical_file();
    EXPECT_EQ(canonical.last_modified, older);
}

TEST(DuplicateGroupTest, CanonicalFilePrefersLexicographicallySmallestWhenTied) {
    std::vector<dedup::FileInfo> files;
    files.push_back(make_file("/path/b.txt", 100, "hash"));
    files.push_back(make_file("/path/a.txt", 100, "hash"));

    dedup::DuplicateGroup group(std::move(files));

    const auto& canonical = group.get_canonical_file();
    EXPECT_EQ(canonical.path, "/path/a.txt");
}
