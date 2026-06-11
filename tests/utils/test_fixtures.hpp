#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace dupfind { namespace test {

/**
 * @class TestFixtures
 * @brief Singleton class to manage, create, and cleanup file-based test fixtures.
 *
 * This class handles setting up a temporary test directory and populating it with
 * various file types (empty, small, large, duplicate, binary) to facilitate testing
 * the deduplication engine.
 */
class TestFixtures {
public:
    /**
     * @brief Gets the singleton instance of the TestFixtures class.
     * @return Reference to the singleton instance.
     */
    static TestFixtures& get_instance();

    /**
     * @brief Initializes the test fixtures.
     * 
     * Creates the temporary directory and generates all test files (empty, small,
     * large, duplicate, and binary files) if they have not been initialized yet.
     */
    void initialize();

    /**
     * @brief Retrieves the filesystem path for a given fixture file name.
     * @param name The unique identifier/name of the fixture.
     * @return The absolute path to the fixture file.
     */
    std::filesystem::path get_path(const std::string& name) const;

    /**
     * @brief Retrieves the precalculated/expected SHA-256 hash of a fixture file.
     * @param name The unique identifier/name of the fixture.
     * @return The hexadecimal string representation of the expected SHA-256 hash.
     */
    std::string get_expected_hash(const std::string& name) const;

    /**
     * @brief Gets the file size of a specified fixture.
     * @param name The unique identifier/name of the fixture.
     * @return The size of the file in bytes.
     */
    uint64_t get_file_size(const std::string& name) const;

    /**
     * @brief Checks if two fixture files are duplicate copies of each other.
     * @param name1 The unique identifier/name of the first fixture.
     * @param name2 The unique identifier/name of the second fixture.
     * @return True if the fixtures are duplicates, false otherwise.
     */
    bool are_duplicates(const std::string& name1, const std::string& name2) const;

    /**
     * @brief Retrieves all fixture names belonging to a specific category.
     * @param category The category name (e.g., "empty", "small", "duplicate").
     * @return A vector of fixture names.
     */
    std::vector<std::string> get_files_in_category(const std::string& category) const;

    /**
     * @brief Cleans up all generated test fixtures and removes the temporary directory.
     */
    void cleanup();

private:
    TestFixtures() = default;
    ~TestFixtures() = default;

    TestFixtures(const TestFixtures&) = delete;
    TestFixtures& operator=(const TestFixtures&) = delete;

    /**
     * @brief Helper method to set up directories and trigger generation of all fixtures.
     * @param path The base directory path where fixtures will be created.
     */
    void create_fixtures(const std::filesystem::path& path);

    /**
     * @brief Helper to create an empty test file (0 bytes).
     * @param path Destination file path.
     */
    void create_empty_file(const std::filesystem::path& path);

    /**
     * @brief Helper to create a small text file.
     * @param path Destination file path.
     */
    void create_small_file(const std::filesystem::path& path);

    /**
     * @brief Helper to create a large test file.
     * @param path Destination file path.
     */
    void create_large_file(const std::filesystem::path& path);

    /**
     * @brief Helper to create duplicate copies of existing files.
     * @param path Destination directory path for duplicates.
     */
    void create_duplicate_files(const std::filesystem::path& path);

    /**
     * @brief Helper to create a binary (non-text) file.
     * @param path Destination file path.
     */
    void create_binary_file(const std::filesystem::path& path);

    /// The base directory path for temporary test fixtures.
    std::filesystem::path root_dir_;
    /// Flag indicating whether the fixtures have been initialized.
    bool initialized_ = false;

    /**
     * @struct FixtureInfo
     * @brief Internal metadata structure for tracking each test fixture.
     */
    struct FixtureInfo {
        std::filesystem::path path;    ///< File system path of the fixture.
        std::string expected_hash;     ///< Precalculated SHA-256 hash.
        uint64_t size;                 ///< Size of the file in bytes.
        std::string category;          ///< Categorized type of file.
    };

    /// Registry map linking unique fixture names to their FixtureInfo.
    std::unordered_map<std::string, FixtureInfo> fixtures_;

    /**
     * @brief Inline helper to get a fixture file's path.
     * @param name The unique name of the fixture.
     * @return Path to the fixture file.
     */
    inline std::filesystem::path fixture_path(const std::string& name)
    {
        return TestFixtures::get_instance().get_path(name);
    }

    /**
     * @brief Inline helper to get a fixture file's expected SHA-256 hash.
     * @param name The unique name of the fixture.
     * @return Precalculated hexadecimal hash string.
     */
    inline std::string fixture_hash(const std::string& name)
    {
        return TestFixtures::get_instance().get_expected_hash(name);
    }
};

}}
