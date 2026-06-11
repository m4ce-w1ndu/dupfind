cmake_minimum_required(VERSION 3.20)

include(FetchContent)

# Fetch GoogleTest v1.15.2
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(dupfind_tests
    test_placeholder.cpp
)

target_link_libraries(dupfind_tests PRIVATE
    gtest_main
    dedup
)

# GoogleTest includes GTest::gtest and GTest::gtest_main, and CMake also exposes GTest::gtest / GTest::gtest_main when FindGTest is used.
# Since we use FetchContent, targets 'gtest' and 'gtest_main' are defined. We can also alias or link to them.
# Linking directly to gtest_main handles the main() entry point.

# Discover tests automatically
include(GoogleTest)
gtest_discover_tests(dupfind_tests)
