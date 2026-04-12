# FoundationCore

A Minecraft 1.8.9 server implementation written in C — built for fun, research, and performance. FoundationCore aims to
be fully compliant with the original Minecraft protocol while exploring how far a native, low-level implementation can
push server performance compared to the JVM-based reference.

> **Status:** Work in progress. Not production-ready.

---

## Motivation

The official Minecraft server is a Java application with inherent overhead from the JVM, garbage collection pauses, and
decades of legacy code. FoundationCore asks: what does a compliant 1.8.9 server look like if you start from scratch in
C, with full control over memory, I/O, and concurrency?

This is not a reimplementation of Notchian server logic for competitive use — it's a systems programming exercise and
protocol study.

---

## Features

- Minecraft 1.8.9 protocol compliance
- Native C implementation — no JVM, no GC pauses
- NASM assembly for performance-critical hot paths
- Built with Clang/LLVM for modern optimizations
- Dependency management via Conan 2

---

## Requirements

The following must be installed on your system. On Arch Linux, `just install-deps` handles everything:

| Tool           | Purpose                      |
|----------------|------------------------------|
| `clang`        | C compiler (LLVM)            |
| `lld`          | LLVM linker                  |
| `lldb`         | LLVM debugger                |
| `libc++`       | LLVM C standard library      |
| `nasm`         | Assembler for `.asm` sources |
| `cmake` ≥ 3.25 | Build system                 |
| `ninja`        | Build backend                |
| `conan` ≥ 2.0  | C/C++ package manager        |

---

## Building

### Arch Linux (recommended)

```bash
# One-time: install all system dependencies
just install-deps

# Build (debug is the default)
just

# Or explicitly
just debug
just release
```

### Other Linux distributions

Install the dependencies listed above through your package manager, then:

```bash
# Debug
conan install . --profile ./conan/profiles/clang --build missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug

# Release
conan install . --profile ./conan/profiles/clang --build missing -s build_type=Release
cmake --preset conan-release
cmake --build --preset conan-release
```

Binaries are output to `build/debug/` and `build/release/` respectively.

---

## Project Structure

```
FoundationCore/
├── conan/
│   └── profiles/
│       └── clang           # Conan build profile (version-controlled)
├── cmake/
│   └── toolchain-clang.cmake
├── include/
│   └── foundation_core/   # Public headers
├── src/                   # C and ASM sources
├── CMakeLists.txt
├── CMakePresets.json
├── conanfile.txt
└── Justfile
```

---

## Protocol Reference

FoundationCore targets the **1.8.9 protocol (version 47)**. The following resources were used as reference:

- [wiki.vg/Protocol (1.8)](https://wiki.vg/index.php?title=Protocol&oldid=7368) — packet structure and field definitions
- [wiki.vg/Protocol_History](https://wiki.vg/Protocol_History) — changelog across versions

---

## License

This project is public domain under the [Unlicense](./LICENSE.md). Do whatever you want with it.