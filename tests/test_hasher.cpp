#include <gtest/gtest.h>
#include "dedup/core/hasher.hpp"
#include "utils/test_fixtures.hpp"
#include <chrono>

class HasherTest : public ::testing::Test {
protected:
    void SetUp() override {
        hasher_ = std::make_unique<dedup::FileHasher>();
        fixtures_ = &dupfind::test::TestFixtures::get_instance();
    }

    std::unique_ptr<dedup::FileHasher> hasher_;
    dupfind::test::TestFixtures* fixtures_;
};

// ============================================================================
// Basic hashing tests
// ============================================================================

TEST_F(HasherTest, EmptyFileHasCorrectHash) {
    auto path = fixtures_->get_path("empty.txt");
    std::string hash = hasher_->compute_full_hash(path.string());

    EXPECT_EQ(hash.length(), 64);  // SHA-256 hex is 64 chars
    // Known SHA-256 of empty file
    EXPECT_EQ(hash, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST_F(HasherTest, SmallFileHashIsConsistent) {
    auto path = fixtures_->get_path("small.txt");

    std::string hash1 = hasher_->compute_full_hash(path.string());
    std::string hash2 = hasher_->compute_full_hash(path.string());

    EXPECT_EQ(hash1, hash2);
}

TEST_F(HasherTest, DifferentFilesHaveDifferentHashes) {
    auto path1 = fixtures_->get_path("small.txt");
    auto path2 = fixtures_->get_path("medium.txt");

    std::string hash1 = hasher_->compute_full_hash(path1.string());
    std::string hash2 = hasher_->compute_full_hash(path2.string());

    EXPECT_NE(hash1, hash2);
}

// ============================================================================
// Partial hash tests
// ============================================================================

TEST_F(HasherTest, PartialHashEqualsFullHashForSmallFile) {
    auto path = fixtures_->get_path("small.txt");
    dedup::FileHasher small_hasher(4096);  // Partial size larger than file (50 bytes)

    std::string partial = small_hasher.compute_partial_hash(path.string());
    std::string full = small_hasher.compute_full_hash(path.string());

    // For files smaller than partial_size, they should be identical
    EXPECT_EQ(partial, full);
}

TEST_F(HasherTest, PartialHashDiffersFromFullHashForLargeFile) {
    auto path = fixtures_->get_path("large.txt");
    dedup::FileHasher small_hasher(100);  // Only first 100 bytes

    std::string partial = small_hasher.compute_partial_hash(path.string());
    std::string full = small_hasher.compute_full_hash(path.string());

    EXPECT_NE(partial, full);
}

TEST_F(HasherTest, PartialHashOnlyHashesFirstNBytes) {
    auto path = fixtures_->get_path("large.txt");
    dedup::FileHasher hasher_100(100);
    dedup::FileHasher hasher_200(200);

    std::string hash_100 = hasher_100.compute_partial_hash(path.string());
    std::string hash_200 = hasher_200.compute_partial_hash(path.string());

    EXPECT_NE(hash_100, hash_200);
}

// ============================================================================
// Duplicate detection tests
// ============================================================================

TEST_F(HasherTest, DuplicateFilesHaveSameHash) {
    auto path1 = fixtures_->get_path("duplicates/copy1.txt");
    auto path2 = fixtures_->get_path("duplicates/copy2.txt");

    std::string hash1 = hasher_->compute_full_hash(path1.string());
    std::string hash2 = hasher_->compute_full_hash(path2.string());

    EXPECT_EQ(hash1, hash2);
}

TEST_F(HasherTest, DifferentFilesInDuplicateFolderHaveDifferentHashes) {
    auto path1 = fixtures_->get_path("duplicates/copy1.txt");
    auto path2 = fixtures_->get_path("duplicates/different.txt");

    std::string hash1 = hasher_->compute_full_hash(path1.string());
    std::string hash2 = hasher_->compute_full_hash(path2.string());

    EXPECT_NE(hash1, hash2);
}

// ============================================================================
// are_identical tests
// ============================================================================

TEST_F(HasherTest, AreIdenticalReturnsTrueForSameFile) {
    auto path = fixtures_->get_path("small.txt");
    EXPECT_TRUE(hasher_->are_identical(path.string(), path.string()));
}

TEST_F(HasherTest, AreIdenticalReturnsTrueForDuplicateFiles) {
    auto path1 = fixtures_->get_path("duplicates/copy1.txt");
    auto path2 = fixtures_->get_path("duplicates/copy2.txt");

    EXPECT_TRUE(hasher_->are_identical(path1.string(), path2.string()));
}

TEST_F(HasherTest, AreIdenticalReturnsFalseForDifferentFiles) {
    auto path1 = fixtures_->get_path("small.txt");
    auto path2 = fixtures_->get_path("medium.txt");

    EXPECT_FALSE(hasher_->are_identical(path1.string(), path2.string()));
}

TEST_F(HasherTest, AreIdenticalReturnsFalseForNonexistentFile) {
    auto path = fixtures_->get_path("small.txt");
    EXPECT_FALSE(hasher_->are_identical(path.string(), "/nonexistent/file.txt"));
}

// ============================================================================
// Caching tests
// ============================================================================

TEST_F(HasherTest, CachingPreventsRehashing) {
    auto path = fixtures_->get_path("large.txt");

    // First call should compute hash
    std::string first = hasher_->compute_full_hash(path.string());

    // Second call should use cache (same result)
    std::string second = hasher_->compute_full_hash(path.string());

    EXPECT_EQ(first, second);
}

TEST_F(HasherTest, ClearCacheWorks) {
    auto path = fixtures_->get_path("small.txt");

    std::string before_clear = hasher_->compute_full_hash(path.string());
    hasher_->clear_cache();
    std::string after_clear = hasher_->compute_full_hash(path.string());

    EXPECT_EQ(before_clear, after_clear);
}

TEST_F(HasherTest, DisablingCacheWorks) {
    auto path = fixtures_->get_path("small.txt");

    hasher_->set_caching_enabled(false);
    std::string hash1 = hasher_->compute_full_hash(path.string());
    std::string hash2 = hasher_->compute_full_hash(path.string());

    EXPECT_EQ(hash1, hash2);  // Still correct, just not cached
}

// ============================================================================
// Error handling tests
// ============================================================================

TEST_F(HasherTest, ComputeHashThrowsForNonexistentFile) {
    EXPECT_THROW(
        hasher_->compute_full_hash("/definitely/does/not/exist.txt"),
        std::runtime_error
    );
}

TEST_F(HasherTest, ComputePartialHashThrowsForNonexistentFile) {
    EXPECT_THROW(
        hasher_->compute_partial_hash("/definitely/does/not/exist.txt"),
        std::runtime_error
    );
}

// ============================================================================
// Binary file tests
// ============================================================================

TEST_F(HasherTest, BinaryFileHashIsConsistent) {
    auto path = fixtures_->get_path("binary/sample.bin");

    std::string hash1 = hasher_->compute_full_hash(path.string());
    std::string hash2 = hasher_->compute_full_hash(path.string());

    EXPECT_EQ(hash1, hash2);
    EXPECT_EQ(hash1.length(), 64);
}

// ============================================================================
// Medium file exact boundary test
// ============================================================================

TEST_F(HasherTest, MediumFileExactBoundary) {
    auto path = fixtures_->get_path("medium.txt");
    dedup::FileHasher exact_hasher(4096);  // File is exactly 4096 bytes

    std::string partial = exact_hasher.compute_partial_hash(path.string());
    std::string full = exact_hasher.compute_full_hash(path.string());

    // When file size equals partial_size, partial and full should match
    EXPECT_EQ(partial, full);
}

// ============================================================================
// Performance test (disabled by default)
// ============================================================================

TEST_F(HasherTest, DISABLED_LargeFileHashIsFast) {
    auto path = fixtures_->get_path("large.txt");

    auto start = std::chrono::steady_clock::now();
    hasher_->compute_full_hash(path.string());
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // Hashing 1MB should take < 100ms on modern hardware
    EXPECT_LT(duration.count(), 100);
}
