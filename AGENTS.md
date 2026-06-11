# Agent Developer Handover & Context Guide (`AGENTS.md`)

Welcome! This document outlines the project goals, architecture, current implementation state, and immediate next steps for AI agents working on `dupfind`.

---

## 1. Project Context & Stack

`dupfind` is a high-performance duplicate file finder written in modern C++20.

- **Build Tooling:** CMake (min version 3.20, configured for C++20).
- **Core Library (`libdedup`):** Built as a static library for portability and reuse (e.g., by CLI or future GUI wrappers).
- **External Dependencies:**
  - **OpenSSL (libcrypto):** Used for fast, hardware-accelerated SHA-256 cryptographic hashing.
  - **SQLite3:** (Planned) Used to store cached file hashes to avoid re-reading huge files on subsequent scans.
- **Compiling Environment:** Tested under Linux with GCC/g++ (C++20 compliant).

---

## 2. Directory Layout & Key Files

The project has been laid down with the following structure:

- [CMakeLists.txt](file:///home/quark/Projects/dupfind/CMakeLists.txt) — Project root CMake build definition.
- [include/dedup/core/types.hpp](file:///home/quark/Projects/dupfind/include/dedup/core/types.hpp) — Data structures, option settings, and callbacks.
- [include/dedup/core/hasher.hpp](file:///home/quark/Projects/dupfind/include/dedup/core/hasher.hpp) — Declares `FileHasher` (partial/full hashing using OpenSSL).
- [include/dedup/core/detector.hpp](file:///home/quark/Projects/dupfind/include/dedup/core/detector.hpp) — Declares `DuplicateDetector` (the multi-stage scanning pipeline).
- [include/dedup/io/filesystem.hpp](file:///home/quark/Projects/dupfind/include/dedup/io/filesystem.hpp) — Declares cross-platform directory walk and file statistics wrappers.
- [include/dedup/actions/executor.hpp](file:///home/quark/Projects/dupfind/include/dedup/actions/executor.hpp) — Declares actions like hardlinking, symlinking, trashing.
- [include/dedup/actions/reporter.hpp](file:///home/quark/Projects/dupfind/include/dedup/actions/reporter.hpp) — Declares report generation interfaces.
- `src/` — Skeletons containing empty stub implementations corresponding to the headers above.
- `cli/main.cpp` — CLI wrapper entry point stub.

---

## 3. Current Project State

- **Stage:** Layout & Structure Validation.
- **Status:** **Compiles successfully.** All stub files are in place, CMake is configured, and a target binary `dedup-cli` compiles and links successfully.
- **Next Milestone:** Implement hashing, caching, and scanning core features.

---

## 4. Phase-by-Phase Implementation Instructions

### Phase 1: Core Hashing & Filesystem Scans
1. **Filesystem Wrapper:** Implement [src/io/filesystem.cpp](file:///home/quark/Projects/dupfind/src/io/filesystem.cpp) using `std::filesystem`.
   - `collectFiles` should recursively traverse directories. Handle permission errors and ignore symlinks (unless specifically enabled in options).
2. **File Hashing:** Implement [src/core/hasher.cpp](file:///home/quark/Projects/dupfind/src/core/hasher.cpp) using OpenSSL (`<openssl/evp.h>` or `<openssl/sha.h>`).
   - `computePartialHash`: read the first `partial_size_` bytes (default 4KB) of a file, hash them.
   - `computeFullHash`: read the entire file in chunks (e.g. 64KB buffers) and feed them to the hashing context to avoid loading huge files into memory.

### Phase 2: Duplicate Detection Pipeline
Implement the three-stage filtering pipeline inside [src/core/detector.cpp](file:///home/quark/Projects/dupfind/src/core/detector.cpp):
1. **Size Grouping:** Group files by size. If a size is unique, discard it.
2. **Partial Hashing:** For files sharing the same size, calculate the partial hash. Group them by partial hash. Discard unique groups.
3. **Full Hashing & Resolution:** For remaining files, calculate the full hash to confirm duplication. Put duplicates into `DuplicateGroup` structures.

### Phase 3: Hash Cache (SQLite3 integration)
Add a SQLite database to save file metadata and hashes.
- **Schema:**
  ```sql
  CREATE TABLE IF NOT EXISTS file_cache (
      filepath TEXT PRIMARY KEY,
      size INTEGER NOT NULL,
      mtime INTEGER NOT NULL,
      inode INTEGER NOT NULL,
      dev INTEGER NOT NULL,
      partial_hash TEXT,
      full_hash TEXT
  );
  ```
- **Optimization:** Query by `(inode, dev, size, mtime)` if a path isn't found to handle renamed/moved files without re-hashing.

### Phase 4: Actions & CLI Interface
1. **Actions:** Implement reporting (Text & JSON), native trashing (using `gio trash` or directory move fallbacks), and C++ filesystem hardlinking/symlinking.
2. **CLI Parser:** Implement argument parsing in `cli/main.cpp` to wire these components together, providing a clean console experience.
