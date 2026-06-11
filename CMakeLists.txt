cmake_minimum_required(VERSION 3.20)
project(libdedup VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library
add_library(dedup STATIC
    src/core/hasher.cpp
    src/core/detector.cpp
    src/io/filesystem.cpp
    src/actions/executor.cpp
    src/actions/reporter.cpp
)

target_include_directories(dedup PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Find OpenSSL for SHA-256
find_package(OpenSSL REQUIRED)
target_link_libraries(dedup PRIVATE OpenSSL::Crypto)

# CLI executable
add_executable(dedup-cli cli/main.cpp)
target_link_libraries(dedup-cli PRIVATE dedup)

# Optional: Install targets
install(TARGETS dedup dedup-cli
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)
