#include <filesystem>
#include <gtest/gtest.h>
#include "dedup/core/types.hpp"

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
