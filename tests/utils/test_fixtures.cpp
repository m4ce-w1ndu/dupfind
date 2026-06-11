#include "test_fixtures.hpp"
#include <filesystem>
#include <fstream>
#include <ios>

namespace dupfind { namespace test {

TestFixtures& TestFixtures::get_instance()
{
    static TestFixtures instance;
    return instance;
}

void TestFixtures::initialize()
{
    if (initialized_) return;

    // Find current root directory
    root_dir_ = std::filesystem::current_path() / "test_fixtures";

    // Create directory structure
    std::filesystem::create_directories(root_dir_ / "duplicates");
    std::filesystem::create_directories(root_dir_ / "binary");
    std::filesystem::create_directories(root_dir_ / "mixed");

    // Create all fixtures
    create_fixtures(root_dir_);

    // Build metadata cache
    populate_metadata(root_dir_);

    initialized_ = true;
}

std::filesystem::path TestFixtures::get_path(const std::string& name) const
{
    if (!initialized_)
        throw std::runtime_error("TestFixtures not initialized. Call initialize() first.");

    auto it = fixtures_.find(name);
    if (it == fixtures_.end())
        throw std::runtime_error("Unknown fixture: " + name);

    return it->second.path;
}

std::string TestFixtures::get_expected_hash(const std::string& name) const
{
    if (!initialized_)
        throw std::runtime_error("TestFixtures not initialized. Call initialize() first.");

    auto it = fixtures_.find(name);
    if (it == fixtures_.end())
        throw std::runtime_error("Unknown fixture: " + name);

    return it->second.expected_hash;
}

uint64_t TestFixtures::get_file_size(const std::string& name) const
{
    if (!initialized_)
        throw std::runtime_error("TestFixtures not initialized. Call initialize() first.");

    auto it = fixtures_.find(name);
    if (it == fixtures_.end())
        throw std::runtime_error("Unknown fixture: " + name);

    return it->second.size;
}

bool TestFixtures::are_duplicates(const std::string& name1, const std::string& name2) const
{
    return (name1 == "duplicates/copy1.txt" && name2 == "duplicates/copy2.txt") ||
           (name1 == "duplicates/copy2.txt" && name2 == "duplicates/copy1.txt");
}

std::vector<std::string> TestFixtures::get_files_in_category(const std::string& category) const
{
    if (!initialized_)
        throw std::runtime_error("TestFixtures not initialized. Call initialize() first.");

    std::vector<std::string> result;

    for (const auto& [name, info] : fixtures_) {
        if (info.category == category)
            result.push_back(name);
    }

    return result;
}

void TestFixtures::cleanup()
{
    if (initialized_ && std::filesystem::exists(root_dir_))
        std::filesystem::remove_all(root_dir_);

    initialized_ = false;
    fixtures_.clear();
}

void TestFixtures::create_fixtures(const std::filesystem::path& path)
{
    // Create directory structure
    std::filesystem::create_directories(path / "duplicates");
    std::filesystem::create_directories(path / "binary");

    // Create all fixture files
    create_empty_file(path / "empty.txt");
    create_small_file(path / "small.txt");
    create_medium_file(path / "medium.txt");
    create_large_file(path / "large.txt");
    create_duplicate_files(path / "duplicates");
    create_binary_file(path / "binary" / "sample.bin");
}

void TestFixtures::create_empty_file(const std::filesystem::path& path)
{
    std::ofstream file(path, std::ios::binary);
    file.close();
}

void TestFixtures::create_small_file(const std::filesystem::path& path)
{
    std::ofstream file(path);
    file << "The quick brown fox jumps over the laxy dog. 12345";
    file.close();
}

void TestFixtures::create_medium_file(const std::filesystem::path& path)
{
    std::ofstream file(path);
    std::string line(63, 'X');

    for (int i = 0; i < 64; i++)
        file << line << "\n";

    file.close();
}

void TestFixtures::create_large_file(const std::filesystem::path& path)
{
    std::ofstream file(path, std::ios::binary);
    std::string chunk(65536, 'L');
    for (int i = 0; i < 16; i++)
        file.write(chunk.data(), chunk.size());
    file.close();
}

void TestFixtures::create_duplicate_files(const std::filesystem::path& dir)
{
    std::ofstream copy1(dir / "copy1.txt");
    copy1 << "This is a duplicate file. Its content is exactly the same as copy2.txt";
    copy1.close();

    std::ofstream copy2(dir / "copy2.txt");
    copy2 << "This is a duplicate file. Its content is exactly the same as copy2.txt";
    copy2.close();

    std::ofstream different(dir / "different.txt");
    different << "This is a DIFFERENT file. Its content is NOT the same as the others.";
    different.close();
}

void TestFixtures::create_binary_file(const std::filesystem::path& path)
{
    std::ofstream file(path, std::ios::binary);

    for (int i = 0; i < 256; i++)
        file.put(static_cast<char>(i));

    file.close();
}

void TestFixtures::populate_metadata(const std::filesystem::path& root)
{
    // Pre-computed SHA-256 hashes (generated once with sha256sum)
    // You can verify these by running: sha256sum tests/fixtures/data/*.txt

    // empty.txt - 0 bytes
    fixtures_["empty.txt"] = {
        root / "empty.txt",
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        0,
        "basic"
    };

    // small.txt - 50 bytes
    fixtures_["small.txt"] = {
        root / "small.txt",
        "de94459fbde1b58afe41f3c4965bd03428072d90d8ce57bf8d9af1f03cc32298",
        50,
        "basic"
    };

    // medium.txt - 4096 bytes (exactly one partial hash chunk)
    fixtures_["medium.txt"] = {
        root / "medium.txt",
        "59aec5fc0cb275abdb9c10bea1b3eded308467fb526e197325fbc8386f104e97",
        4096,
        "basic"
    };

    // large.txt - 1MB
    fixtures_["large.txt"] = {
        root / "large.txt",
        "b8824ab1d764167b60ec900ed95085d72dc8768660469a74effe79a0c22154e6",
        1048576,
        "basic"
    };

    // duplicates/copy1.txt - 72 bytes (same as copy2.txt)
    fixtures_["duplicates/copy1.txt"] = {
        root / "duplicates" / "copy1.txt",
        "8ed45a368056d3de4eab21a911e8e154f56cc7c21e52222f623d1bee1e54e3f2",
        72,
        "duplicates"
    };

    // duplicates/copy2.txt - 72 bytes (same as copy1.txt)
    fixtures_["duplicates/copy2.txt"] = {
        root / "duplicates" / "copy2.txt",
        "8ed45a368056d3de4eab21a911e8e154f56cc7c21e52222f623d1bee1e54e3f2",
        72,
        "duplicates"
    };

    // duplicates/different.txt - 74 bytes
    fixtures_["duplicates/different.txt"] = {
        root / "duplicates" / "different.txt",
        "4c6047143a67d067c6148b8d5410fa8620dce91874ec45752ec0ea3b18b699c5",
        74,
        "duplicates"
    };

    // binary/sample.bin - 256 bytes
    fixtures_["binary/sample.bin"] = {
        root / "binary" / "sample.bin",
        "40aff2e9d2d8922e47afd4648e6967497158785fbd1da870e7110266bf944880",
        256,
        "binary"
    };
}

}}
