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

- Minecraft 1.8.9 protocol compliance (protocol version 47)
- Native C implementation — no JVM, no GC pauses
- NASM assembly for performance-critical hot paths (AABB sweeps, noise evaluation, light nibble scans)
- SIMD-accelerated math via cglm (SSE2/AVX) for entity physics and raycasting
- io_uring batched I/O for chunk data floods and deferred persistence syncs
- Memory-mapped world storage via LMDB — zero-copy reads on cache-hot chunks
- Drop-in high-performance allocator (mimalloc) tuned for server allocation patterns
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

## Dependencies

Dependencies are managed via Conan 2 and declared in `conanfile.txt`. Each is chosen for a specific, non-overlapping
role — no two libraries own the same concern.

### Cryptography

**openssl** — owns all protocol crypto. The 1.8.9 encryption handshake requires a 1024-bit RSA keypair (public key
sent to the client in DER format), an AES/CFB8 stream cipher using the negotiated shared secret as both key and IV
(running continuously across packet boundaries, never reset), and a non-standard SHA1 hexdigest for authentication with
Mojang's session server. libsodium's API deliberately excludes these older primitives, making OpenSSL the only viable
option.

**libsodium** — used exclusively for `randombytes_buf` to generate the verify token sent in the Encryption Request
packet. Kept intentionally separate from OpenSSL: sodium is used only where its API is unambiguous and its guarantees
are strongest (random generation). OpenSSL owns all the actual protocol cryptography.

### Networking & I/O

**libuv** — event loop layer built on Linux epoll. Owns TCP connection lifecycle: `uv_tcp_t` handles per-client sockets,
`uv_loop_t` dispatches readability events, and `uv_queue_work` provides the thread pool used to push blocking
operations — LMDB reads on cache miss, chunk generation — off the network thread, preventing event loop stalls.

**liburing** — io_uring interface for batched kernel I/O submission. Used alongside libuv for the one path where
individual syscall overhead matters: the chunk data flood sent to a client on login (25+ chunk packets in rapid
succession). These are submitted as a single batch of `IORING_OP_SEND` operations, reducing syscall count to one
`io_uring_submit` call. Also used for deferred `fsync` after LMDB commits (see Persistence).

**libcurl** — HTTP/HTTPS client for the Mojang session server. Online-mode login requires a `GET` to
`sessionserver.mojang.com/session/minecraft/hasJoined` to verify the client's join hash. This is one request per
login, not a hot path, and curl's easy interface handles the TLS and redirect complexity cleanly.

### Compression

**zlib** — required by the protocol. After the server sends a `Set Compression` packet, all subsequent packets above
the configured threshold are zlib-deflated and the framing format changes to include an uncompressed-length field.
This is a protocol requirement, not an internal design choice.

**zstd** — internal compression only, not protocol-facing. Used for chunk storage on disk and any in-memory caching
layer. At level 1-3 it significantly outperforms zlib in both speed and ratio. Supports dictionary training on
representative chunk data to improve compression of small NBT payloads, which are otherwise too short to compress
efficiently without a dictionary.

### Persistence

**lmdb** — memory-mapped B-tree database for world and chunk persistence. `mdb_get` returns a direct pointer into the
mmap — on a page-cache hit this is a pointer dereference with no syscall and no copy. The MVCC design allows any
number of concurrent read transactions with no locking; writes serialize to one transaction at a time through a lock
file. Used with `MDB_NOSYNC` to skip the per-commit `fsync`, with liburing issuing a deferred `IORING_FSYNC_DATASYNC`
asynchronously after each commit to recover durability without blocking the game tick.

### Serialization & Text

**cjson** — JSON for protocol text components. The 1.8.9 protocol encodes chat messages, disconnect reasons, and the
server list MOTD as JSON strings. cjson is a single `.c`/`.h` pair with no transitive dependencies, making it the
minimal correct choice for this role.

**fmt** — structured logging and string formatting. Used for all human-readable output: startup banners, per-connection
logs, packet traces in debug builds, and error messages. Its compile-time format checking catches format/argument
mismatches that `printf` silently ignores.

### Math & Geometry

**cglm** — SIMD-accelerated linear algebra for C. Provides `vec3`, `vec4`, `mat4`, and AABB types with SSE2/AVX/NEON
paths. Used in three places: AABB sweep tests in entity physics validation (checking the client's claimed movement
against the block neighbourhood), DDA raycasting for mob line-of-sight and combat reach validation, and bounding-box
overlap tests during entity tick. The SIMD alignment guarantees let hand-written NASM kernels operate directly on
cglm's types on the inner loops.

### Performance & Allocation

**mimalloc** — drop-in replacement for glibc malloc. Its segment-based free-list sharding gives better cache locality
for the small, frequent allocations produced by packet parsing, per-tick entity state updates, and NBT node trees.
Benchmarks show consistent performance advantages over jemalloc and tcmalloc on server-like workloads with mixed
allocation sizes and thread patterns. Integration requires no code changes — link-time replacement or `LD_PRELOAD`
both work as a starting point.

**xxhash** — non-cryptographic hash function for all runtime hash tables. `XXH3_64bits` is the fastest
general-purpose hash for both short keys (chunk coordinates packed into a `uint64_t`, player UUIDs) and bulk data.
Used as the hash function for open-addressing tables mapping chunk positions to loaded chunk state and player UUIDs to
entity records.

**libuuid** — RFC 4122 UUID generation. Used where the protocol requires a standards-compliant UUID format rather than
a raw random buffer — entity UUIDs sent in `Spawn Player` and `Spawn Object` packets must be valid RFC 4122 type 4
UUIDs. Random-byte generation for verify tokens uses libsodium instead.

### Testing

**cmocka** — unit testing framework. Provides per-test setup/teardown, mock expectations (`will_return`,
`expect_function_call`), and structured failure output compatible with CTest. The packet codec, VarInt encoder/decoder,
NBT parser, and AABB collision resolver are all tested by injecting raw byte buffers directly into pure-function
interfaces — no live socket or running server required. Running the test suite under AddressSanitizer and
UndefinedBehaviorSanitizer (`-fsanitize=address,undefined`) catches memory and arithmetic errors in the codec layer
before they become exploitable.

---

## Protocol Reference

FoundationCore targets the **1.8.9 protocol (version 47)**. The following resources were used as reference:

- [wiki.vg/Protocol (1.8)](https://wiki.vg/index.php?title=Protocol&oldid=7368) — packet structure and field definitions
- [wiki.vg/Protocol_History](https://wiki.vg/Protocol_History) — changelog across versions

---

## License

This project is public domain under the [Unlicense](./LICENSE.md). Do whatever you want with it.