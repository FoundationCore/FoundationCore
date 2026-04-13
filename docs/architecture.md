# FoundationCore — Project Architecture

## Design Principles

The codebase is organized around **bounded contexts** — each top-level module owns a
cohesive set of types and operations, exposes a narrow public API through headers, and
hides its internals. Dependencies flow strictly inward: infrastructure modules never
import game logic, and game logic never reaches into network internals.

Every module follows the same physical layout:

```
include/foundation_core/<module>/   ← public API (what other modules may #include)
src/<module>/                       ← implementation (.c and .asm files)
tests/<module>/                     ← unit tests (cmocka)
```

Headers use the guard path `#pragma once` and are always included as
`#include <foundation_core/<module>/<header>.h>`.

---

## Dependency Graph

```
                  ┌──────────┐
                  │   core   │  ← shared kernel (no dependencies)
                  └────┬─────┘
           ┌───────────┼───────────┐
           ▼           ▼           ▼
      ┌────────┐  ┌────────┐  ┌────────┐
      │  nbt   │  │ crypto │  │  math  │
      └───┬────┘  └───┬────┘  └───┬────┘
          │           │           │
          ▼           ▼           │
      ┌─────────┐ ┌─────────┐     │
      │protocol │ │ network │◄────┘
      └────┬────┘ └────┬────┘
           │           │
     ┌─────┴─────┐     │
     ▼           ▼     ▼
 ┌───────┐  ┌────────────┐
 │ world │  │  registry  │
 └───┬───┘  └─────┬──────┘
     │            │
     ▼            ▼
 ┌────────┐  ┌────────┐
 │ entity │  │  chat  │
 └───┬────┘  └───┬────┘
     │           │
     ▼           ▼
 ┌─────────────────┐
 │     player      │
 └────────┬────────┘
          ▼
 ┌─────────────────┐
 │      game       │  ← orchestrator (depends on everything above)
 └────────┬────────┘
          ▼
 ┌─────────────────┐
 │  persistence    │  ← side-effect boundary (LMDB, disk I/O)
 └─────────────────┘
```

An arrow `A → B` means "A may `#include` headers from B". Cycles are forbidden.

---

## Full Tree

```
FoundationCore/
│
├── include/
│   └── foundation_core/
│       │
│       ├── core/                       ── Shared Kernel ──────────────────
│       │   ├── buffer.h                # Byte buffer: read/write cursor, auto-grow
│       │   ├── arena.h                 # Arena allocator for per-tick scratch memory
│       │   ├── hashmap.h               # Open-addressing hash map (xxhash)
│       │   ├── list.h                  # Intrusive doubly-linked list
│       │   ├── ring.h                  # Lock-free SPSC ring buffer (network ↔ game)
│       │   ├── result.h               # Error-or-value return type (fc_result_t)
│       │   ├── log.h                   # Structured logging facade (fmt backend)
│       │   ├── uuid.h                  # UUID generation and formatting (libuuid)
│       │   ├── random.h               # CSPRNG wrapper (libsodium randombytes)
│       │   ├── time.h                  # Monotonic clock, tick timing
│       │   ├── types.h                 # Shared typedefs: fc_byte_t, fc_varint_t, etc.
│       │   └── platform.h             # Endian swaps, compiler attributes, alignment
│       │
│       ├── math/                       ── Math & Geometry ────────────────
│       │   ├── vec3.h                  # vec3 operations (cglm wrappers + helpers)
│       │   ├── aabb.h                  # Axis-aligned bounding box type and tests
│       │   ├── ray.h                   # Ray type and DDA raycast interface
│       │   ├── position.h             # Block position encoding/decoding (26|12|26)
│       │   └── fixed_point.h          # Fixed-point ↔ double conversion (5 frac bits)
│       │
│       ├── nbt/                        ── NBT Serialization ──────────────
│       │   ├── nbt.h                   # Tag types, tree node, visitor pattern
│       │   ├── reader.h               # Decode NBT from a byte buffer
│       │   └── writer.h               # Encode NBT into a byte buffer
│       │
│       ├── crypto/                     ── Cryptography & Auth ────────────
│       │   ├── rsa.h                   # RSA-1024 keypair, DER export, PKCS#1 decrypt
│       │   ├── aes_cfb8.h             # AES-128/CFB8 continuous stream cipher context
│       │   ├── sha1.h                  # Minecraft-flavored SHA1 hexdigest (signed)
│       │   └── session.h              # Mojang session server client (hasJoined GET)
│       │
│       ├── protocol/                   ── Protocol Codec ─────────────────
│       │   ├── varint.h               # VarInt / VarLong encode and decode
│       │   ├── codec.h                # Data type codecs: String, Position, Angle, UUID, Slot
│       │   ├── compression.h          # zlib packet compression / decompression
│       │   ├── packet.h               # Base packet type, direction enum, state enum
│       │   │
│       │   ├── handshake/
│       │   │   └── packets.h          # 0x00 Handshake
│       │   │
│       │   ├── status/
│       │   │   └── packets.h          # 0x00 Request/Response, 0x01 Ping/Pong
│       │   │
│       │   ├── login/
│       │   │   └── packets.h          # Login Start, Encryption Req/Resp,
│       │   │                          # Set Compression, Login Success, Disconnect
│       │   │
│       │   └── play/
│       │       ├── clientbound.h      # All S→C play packets (Join Game, Chunk Data, etc.)
│       │       └── serverbound.h      # All C→S play packets (Player Position, Chat, etc.)
│       │
│       ├── network/                    ── Network & I/O ──────────────────
│       │   ├── server.h               # TCP listener, accept loop (libuv)
│       │   ├── connection.h           # Per-client connection: state machine, buffers,
│       │   │                          # encryption ctx, compression ctx
│       │   ├── pipeline.h             # Read/write pipeline: frame → decrypt → decompress → dispatch
│       │   ├── uring_sender.h         # io_uring batched send for chunk floods
│       │   └── handler.h             # Packet handler dispatch table (state × packet_id → fn)
│       │
│       ├── registry/                   ── Static Data Registries ─────────
│       │   ├── block.h                # Block ID → properties (hardness, light, solid, etc.)
│       │   ├── item.h                 # Item ID → properties (stack size, durability, etc.)
│       │   ├── entity_type.h          # Entity type ID → metadata schema, dimensions
│       │   ├── biome.h                # Biome ID → color, temperature, rainfall
│       │   ├── enchantment.h          # Enchantment ID → max level, applicable items
│       │   └── sound.h               # Sound event → name string
│       │
│       ├── chat/                       ── Chat & Text Components ─────────
│       │   ├── text.h                 # JSON text component builder (cjson)
│       │   ├── color.h               # Chat color codes and formatting
│       │   └── command.h             # Command parsing and dispatch
│       │
│       ├── world/                      ── World Domain ───────────────────
│       │   ├── chunk.h                # Chunk column: sections, bitmask, block access
│       │   ├── section.h             # 16³ section: block array, light nibble arrays
│       │   ├── block_state.h         # Block state (type << 4 | meta), encode/decode
│       │   ├── light.h               # Light propagation engine (block + sky light)
│       │   ├── generation.h          # Chunk generator interface + flat world impl
│       │   ├── region.h              # Loaded region management, chunk lifecycle
│       │   └── world.h               # World aggregate: dimension, time, weather, spawn
│       │
│       ├── entity/                     ── Entity Domain ──────────────────
│       │   ├── entity.h               # Base entity: EID, UUID, position, velocity, type
│       │   ├── metadata.h            # Entity metadata codec (type-index-value)
│       │   ├── mob.h                  # Mob entity: AI state, pathfinding hooks
│       │   ├── object.h              # Object entity: minecart, arrow, item, etc.
│       │   ├── tracker.h             # Entity tracker: who can see whom, delta updates
│       │   └── physics.h             # Movement validation, AABB sweep, gravity
│       │
│       ├── player/                     ── Player Domain ──────────────────
│       │   ├── player.h               # Player aggregate: entity + connection + inventory
│       │   │                          # + game mode + abilities + tab list entry
│       │   ├── inventory.h           # Inventory windows: player, chest, crafting, etc.
│       │   ├── slot.h                 # Slot data: item ID, count, damage, NBT
│       │   ├── abilities.h           # Player abilities: fly, creative, speed
│       │   └── skin.h                # Skin/cape texture properties (from Mojang response)
│       │
│       ├── game/                       ── Game Orchestration ─────────────
│       │   ├── server.h               # Game server: tick loop, player join/leave,
│       │   │                          # world management, global state
│       │   ├── tick.h                 # Tick scheduler: 20 TPS timing, task queue
│       │   ├── event.h               # Event bus: block break, entity damage, chat, etc.
│       │   ├── scoreboard.h          # Scoreboard objectives, teams, display slots
│       │   ├── boss_bar.h            # Boss bar state and packets
│       │   └── config.h              # Server properties (port, motd, max players, etc.)
│       │
│       └── persistence/                ── Storage Layer ──────────────────
│           ├── database.h             # LMDB environment setup, transaction helpers
│           ├── chunk_store.h         # Chunk serialize/deserialize to LMDB (zstd compressed)
│           ├── player_store.h        # Player data save/load (position, inventory, etc.)
│           └── region_io.h           # Deferred fsync via io_uring after LMDB commits
│
├── src/
│   ├── main.c                          # Entry point: parse config, init subsystems, run
│   │
│   ├── core/
│   │   ├── buffer.c
│   │   ├── arena.c
│   │   ├── hashmap.c
│   │   ├── list.c
│   │   ├── ring.c
│   │   ├── log.c
│   │   ├── uuid.c
│   │   ├── random.c
│   │   └── time.c
│   │
│   ├── math/
│   │   ├── aabb.c
│   │   ├── ray.c
│   │   ├── position.c
│   │   ├── fixed_point.c
│   │   ├── aabb_sweep.asm             # SIMD AABB sweep test (hot path)
│   │   └── ray_dda.asm               # DDA raycast inner loop
│   │
│   ├── nbt/
│   │   ├── nbt.c                      # Tag constructors, tree manipulation, free
│   │   ├── reader.c
│   │   └── writer.c
│   │
│   ├── crypto/
│   │   ├── rsa.c
│   │   ├── aes_cfb8.c
│   │   ├── sha1.c
│   │   └── session.c
│   │
│   ├── protocol/
│   │   ├── varint.c
│   │   ├── codec.c
│   │   ├── compression.c
│   │   ├── handshake/
│   │   │   └── packets.c
│   │   ├── status/
│   │   │   └── packets.c
│   │   ├── login/
│   │   │   └── packets.c
│   │   └── play/
│   │       ├── clientbound.c          # Will likely split as it grows:
│   │       │                          #   spawn.c, chunk.c, entity.c, window.c, etc.
│   │       └── serverbound.c
│   │
│   ├── network/
│   │   ├── server.c
│   │   ├── connection.c
│   │   ├── pipeline.c
│   │   ├── uring_sender.c
│   │   └── handler.c
│   │
│   ├── registry/
│   │   ├── block.c
│   │   ├── item.c
│   │   ├── entity_type.c
│   │   ├── biome.c
│   │   ├── enchantment.c
│   │   └── sound.c
│   │
│   ├── chat/
│   │   ├── text.c
│   │   ├── color.c
│   │   └── command.c
│   │
│   ├── world/
│   │   ├── chunk.c
│   │   ├── section.c
│   │   ├── block_state.c
│   │   ├── light.c
│   │   ├── generation.c
│   │   ├── region.c
│   │   ├── world.c
│   │   └── light_scan.asm             # Nibble array scan (SIMD hot path)
│   │
│   ├── entity/
│   │   ├── entity.c
│   │   ├── metadata.c
│   │   ├── mob.c
│   │   ├── object.c
│   │   ├── tracker.c
│   │   └── physics.c
│   │
│   ├── player/
│   │   ├── player.c
│   │   ├── inventory.c
│   │   ├── slot.c
│   │   ├── abilities.c
│   │   └── skin.c
│   │
│   ├── game/
│   │   ├── server.c
│   │   ├── tick.c
│   │   ├── event.c
│   │   ├── scoreboard.c
│   │   ├── boss_bar.c
│   │   └── config.c
│   │
│   └── persistence/
│       ├── database.c
│       ├── chunk_store.c
│       ├── player_store.c
│       └── region_io.c
│
├── tests/
│   ├── core/
│   │   ├── test_buffer.c
│   │   ├── test_hashmap.c
│   │   └── test_ring.c
│   │
│   ├── math/
│   │   ├── test_aabb.c
│   │   ├── test_position.c
│   │   └── test_fixed_point.c
│   │
│   ├── nbt/
│   │   ├── test_reader.c
│   │   ├── test_writer.c
│   │   └── fixtures/                  # Binary NBT blobs for round-trip tests
│   │       ├── hello_world.nbt
│   │       └── bigtest.nbt
│   │
│   ├── protocol/
│   │   ├── test_varint.c
│   │   ├── test_codec.c
│   │   ├── test_compression.c
│   │   └── fixtures/                  # Raw captured packet bytes
│   │       ├── handshake.bin
│   │       └── login_start.bin
│   │
│   ├── crypto/
│   │   ├── test_sha1.c               # Notch/jeb_/simon digest vectors
│   │   └── test_aes_cfb8.c
│   │
│   ├── world/
│   │   ├── test_chunk.c
│   │   ├── test_section.c
│   │   └── test_block_state.c
│   │
│   ├── entity/
│   │   ├── test_metadata.c
│   │   └── test_physics.c
│   │
│   └── player/
│       ├── test_inventory.c
│       └── test_slot.c
│
├── data/                              ── Static Data Files ───────────────
│   ├── blocks.json                    # Block registry (extracted from Notchian JAR)
│   ├── items.json                     # Item registry
│   ├── entities.json                  # Entity type table
│   ├── biomes.json                    # Biome properties
│   └── recipes.json                   # Crafting recipes
│
├── cmake/
│   └── toolchain-clang.cmake
│
├── .conan2/
│   └── profiles/
│       ├── clang
│       └── clang-ci
│
├── .github/
│   └── workflows/
│       ├── ci.yaml
│       └── release.yaml
│
├── CMakeLists.txt
├── CMakePresets.json
├── conanfile.txt
├── Justfile
├── cliff.toml
├── .clang-format
├── .clang-tidy
├── .gitignore
├── LICENSE.md
└── README.md
```

---

## Module Responsibilities

### `core/` — Shared Kernel

The foundation layer. **Zero game knowledge.** Provides data structures, memory
primitives, logging, and platform abstractions that every other module depends on.

Key design decisions:

- `buffer.h` is the workhorse of the entire codebase. Every packet read/write goes
  through it. It tracks a read cursor and a write cursor over a contiguous byte array,
  with auto-grow on write. Think of it as a `ByteBuf` from Netty but in C.
- `arena.h` gives you per-tick scratch memory. At the start of each game tick, reset
  the arena. Packet parsing, temporary entity lists, pathfinding scratch — all allocated
  from the arena, all freed in one shot. This eliminates thousands of individual
  malloc/free pairs per tick.
- `hashmap.h` uses xxhash internally and open addressing. The Minecraft server needs
  fast lookups by chunk coordinate, entity ID, and player UUID — this single type
  covers all three.
- `ring.h` is for the network→game thread boundary. Network callbacks (libuv) push
  parsed packets into the ring; the game tick drains it.

### `math/` — Math & Geometry

Pure functions over coordinate types. **No allocations, no side effects.** This is
where cglm types live and where the NASM hot-path kernels go.

- `position.h` handles the 26|12|26 bit-packed block position encoding that the
  protocol uses everywhere.
- `aabb.h` and `physics.h` (in entity/) together own the collision story. The AABB
  type lives in math/; the sweep-test logic that uses it against the block grid lives
  in entity/physics.
- `aabb_sweep.asm` and `ray_dda.asm` are NASM kernels that operate directly on cglm's
  aligned vec3/AABB types.

### `nbt/` — NBT Serialization

A self-contained NBT encoder/decoder. Used by protocol/ (for chunk and entity packets),
world/ (for block entity data), and player/ (for inventory item tags).

The tree is a tagged union (`nbt_tag_t`) with a visitor interface for traversal.
The reader consumes from a `fc_buffer_t`; the writer produces into one.

### `crypto/` — Cryptography & Authentication

Owns the entire encryption handshake:

- `rsa.h` — generate the 1024-bit keypair at startup, export DER, decrypt the
  client's shared secret and verify token.
- `aes_cfb8.h` — wrap an OpenSSL EVP context for AES-128/CFB8. One context per
  direction per connection. The cipher is continuous (never reset between packets).
- `sha1.h` — the Minecraft-specific signed hexdigest.
- `session.h` — the libcurl GET to `sessionserver.mojang.com/session/minecraft/hasJoined`.
  This runs on libuv's thread pool (via `uv_queue_work`) to avoid blocking the
  event loop.

### `protocol/` — Protocol Codec

Pure serialization. **Knows how to read and write packets, but not what to do with
them.** This is the boundary between bytes-on-the-wire and typed structs.

- `varint.h` — encode/decode VarInt and VarLong.
- `codec.h` — higher-level type codecs: read/write String, Position, Angle, UUID,
  Slot, and Metadata from/into a `fc_buffer_t`.
- `compression.h` — zlib deflate/inflate for packet-level compression.
- `packet.h` — the base packet type and enums for direction (clientbound/serverbound)
  and state (handshaking/status/login/play).
- Subdirectories per state hold the packet struct definitions and their
  encode/decode functions. Each packet is a plain C struct.

The play/ subdirectory will grow the fastest. When `clientbound.c` gets unwieldy,
split by concern: `spawn_packets.c`, `chunk_packets.c`, `entity_packets.c`,
`window_packets.c`, etc. The header can stay unified or split — your call.

### `network/` — Network & I/O

TCP connection lifecycle and the read/write pipeline. Owns libuv and io_uring.

- `server.h` — binds the TCP socket, runs the libuv event loop, accepts connections.
- `connection.h` — the per-client connection object. Contains the state machine
  (current protocol state), read buffer (accumulating partial packets), write queue,
  encryption contexts (from crypto/), compression threshold, and a backpointer to
  the player (once in Play state).
- `pipeline.h` — the packet processing pipeline:
    - **Read path:** raw bytes → decrypt (if enabled) → deframe (length-prefix) →
      decompress (if enabled) → decode packet ID → dispatch to handler.
    - **Write path:** encode packet struct → compress (if above threshold) →
      encrypt → enqueue for send.
- `uring_sender.h` — batched io_uring send for the chunk-data flood on login.
  When a player joins, 25+ chunk packets need to go out fast. Instead of one
  `uv_write` per packet, they're submitted as a single `io_uring_submit` batch.
- `handler.h` — a dispatch table indexed by `(state, packet_id)` that maps to
  handler functions. Each handler receives the decoded packet struct and the
  connection, then calls into the appropriate domain.

### `registry/` — Static Data Registries

Lookup tables for block, item, entity, biome, and sound properties. Loaded once
at startup from the JSON files in `data/`. These are **read-only** after init.

These decouple the game logic from magic numbers. Instead of `if (block_id == 4)`,
you query `block_registry_get(id)->hardness`.

### `chat/` — Chat & Text Components

The 1.8.9 protocol encodes chat messages, disconnect reasons, tab headers, and
the server list MOTD as JSON text components. This module builds and serializes
those components using cJSON.

`command.h` parses chat messages that start with `/` and dispatches to registered
command handlers.

### `world/` — World Domain

The largest domain. Owns the in-memory representation of the Minecraft world.

- `chunk.h` — a chunk column (16 sections stacked vertically). The primary bitmask
  tracks which sections are present. Provides `chunk_get_block(chunk, x, y, z)` and
  `chunk_set_block(...)`.
- `section.h` — a 16³ section. Contains the 8192-byte block array (type << 4 | meta,
  two bytes per block), the 2048-byte block light nibble array, and the 2048-byte
  sky light nibble array.
- `block_state.h` — encodes/decodes the (type, metadata) pair into the two-byte
  section format and into the VarInt global palette ID used in packets.
- `light.h` — light propagation (BFS flood fill). Block light sources (torches,
  lava, glowstone) and sky light from the top down. This is one of the more
  CPU-intensive systems.
- `generation.h` — chunk generator interface. The first implementation is a flat
  world (layers of stone + dirt + grass). Later: noise-based overworld, nether,
  end.
- `region.h` — manages the set of currently loaded chunks, handles load/unload
  based on player view distance.
- `world.h` — the world aggregate: ties together the chunk grid, entity list,
  time of day, weather state, and spawn position.

### `entity/` — Entity Domain

Everything that moves.

- `entity.h` — base entity: entity ID, UUID, type, position, velocity, on_ground,
  bounding box. All mobs, objects, and players extend this conceptually (via
  composition, not inheritance — this is C).
- `metadata.h` — the type-index-value codec for entity metadata. Encodes the
  `(type << 5 | index)` byte, reads/writes values, terminates with 0x7F.
- `tracker.h` — the entity tracker decides which entities are visible to which
  players (based on distance), and computes delta updates (relative move, look,
  teleport) to send each tick.
- `physics.h` — movement validation. When the client sends a position update,
  the server checks it against the block grid using AABB sweep tests. This is
  where the NASM aabb_sweep kernel is called.

### `player/` — Player Domain

The player is the **aggregate root** that ties together a network connection, an
entity in the world, an inventory, abilities, and a tab list entry.

- `player.h` — the main player struct. Holds references to (not copies of) the
  connection (network/), the entity (entity/), the inventory, and game mode.
  Provides high-level operations: `player_send_chunk`, `player_teleport`,
  `player_set_game_mode`.
- `inventory.h` — window management. The player's inventory is window 0.
  Opening a chest creates a new window. Each window has slots, and slot clicks
  go through the inventory logic.
- `slot.h` — a single inventory slot: item ID (short), count (byte), damage
  (short), and optional NBT tag compound.

### `game/` — Game Orchestration

The top-level coordinator. This is where the tick loop lives.

- `server.h` (game server, not TCP server) — the game-level server object.
  Manages the world list, online player list, handles player join/leave,
  broadcasts chat, and runs the tick loop.
- `tick.h` — the 20-TPS tick scheduler. Each tick: drain the packet ring buffer,
  process player inputs, run entity physics, update world state, flush outbound
  packets.
- `event.h` — an event bus for decoupling. When a block is broken, an event fires.
  Listeners (drop items, update light, notify nearby players) respond without the
  block-breaking code knowing about them.
- `config.h` — server.properties equivalent. Port, MOTD, max players, view
  distance, online mode, difficulty, etc.

### `persistence/` — Storage Layer

The side-effect boundary. Everything that touches disk goes through here.

- `database.h` — LMDB environment init, transaction helpers, error mapping.
- `chunk_store.h` — serializes chunks to bytes (using nbt/ for block entities
  and zstd for compression), writes to LMDB. On read, decompresses and
  deserializes back into the world/ chunk types.
- `player_store.h` — saves/loads player data: position, inventory, game mode,
  health, food level.
- `region_io.h` — the deferred fsync strategy. After each LMDB commit
  (MDB_NOSYNC), queues an `IORING_FSYNC_DATASYNC` via io_uring to recover
  durability without blocking the game tick.

---

## How a Packet Flows Through the System

To make the boundaries concrete, here's the life of a Player Position packet:

```
1. [network/server.c]       libuv on_read callback fires, raw bytes arrive
2. [network/pipeline.c]     decrypt (AES/CFB8) → deframe → decompress (zlib)
3. [protocol/varint.c]      decode packet ID → 0x04 (Player Position)
4. [protocol/play/sb.c]     decode fields into player_position_packet_t struct
5. [network/handler.c]      dispatch table calls handle_player_position(conn, pkt)
6. [player/player.c]        high-level handler: validate, update entity position
7. [entity/physics.c]       AABB sweep against block grid, check for illegal movement
8. [entity/tracker.c]       compute delta, queue Entity Relative Move to nearby players
9. [protocol/play/cb.c]     encode Entity Relative Move into bytes
10.[network/pipeline.c]     compress → encrypt → enqueue → libuv uv_write
```

Each layer only calls into the layer below it or into its own internals.
The handler in step 5 is the seam where network meets game logic.

---

## CMake Integration

The source tree maps directly to CMake targets or to a single target with
organized source groups. For a single-binary project like this, one executable
target with all sources is simplest:

```cmake
file(GLOB_RECURSE C_SOURCES CONFIGURE_DEPENDS "src/*.c")
file(GLOB_RECURSE ASM_SOURCES CONFIGURE_DEPENDS "src/*.asm")

add_executable(${PROJECT_NAME} ${C_SOURCES} ${ASM_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

If you later want per-module static libraries (for faster incremental builds or
to enforce dependency rules at link time), each `src/<module>/` becomes its own
`add_library(fc_<module> STATIC ...)` with explicit `target_link_libraries`
declaring the allowed dependencies.

---

## Naming Conventions

| Entity           | Convention         | Example                       |
|------------------|--------------------|-------------------------------|
| Files            | `snake_case`       | `aes_cfb8.c`, `block_state.h` |
| Functions        | `module_verb_noun` | `buffer_write_varint()`       |
| Types (structs)  | `snake_case_t`     | `fc_connection_t`, `chunk_t`  |
| Type (enums)     | `UPPER_CASE`       | `PACKET_STATE_PLAY`           |
| Macros           | `FC_UPPER_CASE`    | `FC_MAX_PLAYERS`              |
| Global variables | `g_` prefix        | `g_block_registry`            |

All public symbols are prefixed with `fc_` or the module name to avoid collisions.