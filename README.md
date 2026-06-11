# dupfind

`dupfind` is a high-performance duplicate file finder written in modern C++20. It is designed to quickly scan massive directories, identify duplicate files regardless of their filenames, and safely reclaim storage using various deduplication actions.

To ensure peak performance on large files, `dupfind` uses a multi-stage filtering process combined with a persistent database cache.

---

## Key Features

*   **Multi-Stage Filtering Pipeline:** Only hashes what is absolutely necessary to identify duplicates, saving valuable disk I/O.
*   **Fast Hashing:** Powered by OpenSSL's highly optimized, hardware-accelerated SHA-256 implementation.
*   **SQLite3 Hash Cache:** Caches calculated hashes. Rescans of unchanged directories are near-instant.
*   **Smart Rename Tracking:** Detects renamed or moved files using device and inode lookups, reusing the cached hash without re-reading the file.
*   **Advanced Deduplication Actions:**
    *   **Interactive Mode:** Step-by-step resolution prompts in the console.
    *   **Hardlinking:** Replaces duplicate files on the same drive with hard links to save space.
    *   **Symlinking:** Replaces duplicate files with relative symbolic links.
    *   **System Trash:** Moves duplicate files safely to the desktop trash folder using native desktop utilities (`gio trash`).
    *   **Detailed Reporting:** Generates comprehensive JSON or formatted text reports.

---

## Hashing Strategy

To optimize speed and minimize disk reads, `dupfind` implements a three-stage filtering pipeline:

1.  **Size Filtering:** The directory is traversed, and files are grouped by size. Files with unique sizes are discarded immediately because they cannot be duplicates.
2.  **Partial Hashing:** For files sharing the same size, the application reads and hashes only the first 4 KB of content. Files with unique partial hashes are discarded.
3.  **Full Hashing:** For files sharing both the same size and partial hash, the entire file content is read and hashed to confirm duplication.

---

## Directory Structure

The project separates core business logic from front-end interfaces:

*   **`libdedup`:** A clean static library implementing all core filesystem scanning, hashing, caching, and deduplication logic.
*   **`cli/`:** A thin command-line interface wrapper that handles command parsing, status display, and interactive prompts.
*   **`include/dedup/`:** Public API headers for core types, hasher, detector, filesystem utils, and actions.

---

## Requirements

*   CMake 3.20 or newer
*   A C++20-compliant compiler (such as GCC 13+ or Clang 15+)
*   OpenSSL development libraries (libcrypto)
*   SQLite3 development libraries

---

## Building the Project

Configure and build the project using CMake:

```bash
# Configure the build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Compile the library and CLI
cmake --build build
```

This generates:
*   `build/libdedup.a`: The static deduplication engine library.
*   `build/dupfind-cli`: The command-line executable.

---

## Command Line Usage

Scan directories and output duplicates:

```bash
# Scan a folder and run in interactive mode (default)
./build/dupfind-cli /path/to/downloads /path/to/backup

# Scan and output a JSON report
./build/dupfind-cli --action report --output report.json /path/to/downloads

# Automate hardlinking for duplicate files found
./build/dupfind-cli --action hardlink /path/to/downloads
```

### Available CLI Options:
*   `-h, --help`: Show the help message.
*   `-m, --min-size <size>`: Minimum file size to process (e.g., `10k`, `1M`, `500M`). Defaults to `1` (ignores empty 0-byte files).
*   `-a, --action <report|trash|hardlink|symlink|interactive>`: Deduplication action. Defaults to `interactive`.
*   `-c, --cache-path <path>`: Custom path to the SQLite cache database. Defaults to `~/.cache/dupfind/cache.db`.
*   `--no-cache`: Disables hash caching entirely.
*   `-v, --verbose`: Enable verbose logging output.

---

## License

This project is licensed under the terms of the GNU General Public License, Version 2 (GPL-2.0). See the [LICENSE](LICENSE) file for the full text.
