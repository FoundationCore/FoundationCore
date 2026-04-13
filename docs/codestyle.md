# FoundationCore Code Style Guide

This document defines the official code style for FoundationCore. It is intentionally strict. The purpose is not
aesthetic uniformity for its own sake, but operational clarity: code must be easy to read under pressure, easy to audit,
easy to debug, and difficult to misuse by accident.

The guide applies to the whole project, including C23 source code, public headers, internal headers, tests, and NASM
assembly for x86_64 System V. It is written to fit the project’s current architectural shape, where code is organized by
bounded context under `include/foundation_core/<module>/`, `src/<module>/`, and `tests/<module>/`, and where public APIs
are intentionally narrow and internals are hidden whenever possible.

This is a prose specification, not a checklist. The goal is to explain the reasoning behind the rules so the project
remains coherent even when the formatter or linter cannot fully enforce a detail.

## 1. The style philosophy

FoundationCore prefers correctness, safety, and maintenance clarity over cleverness. Performance matters, but
performance must remain inspectable. A fast system that is difficult to reason about is not considered well-written. In
practice this means code should make ownership, invariants, assumptions, and failure paths obvious to the next person
reading it.

The project also favors explicitness over convenience. A few extra lines are acceptable when they make behavior clearer.
Hidden control flow, ambiguous ownership, surprising macros, or code that compresses too much logic into one expression
are all discouraged because they raise the cognitive cost of maintenance.

This guide is also shaped by two facts. First, the codebase is low-level and mixes C with NASM. Second, the architecture
is modular and layered: `core` acts as a shared kernel, domain modules own their own concepts, and infrastructure
modules must not leak their assumptions into unrelated layers. A code style that ignores those boundaries would make the
architecture brittle.

When two rules seem to conflict, the project resolves them in this order: correctness first, then safety, then
architectural integrity, then debuggability, then performance clarity, then stylistic consistency.

## 2. Language baseline and portability

The project uses C23. Even so, code should remain as close to portable ISO C as reasonably possible. Compiler-specific
extensions are not part of the default style. If a non-ISO feature is introduced, the burden of proof is on the author
to show that it is justified, documented, and isolated behind a narrow interface.

In other words, the style does not treat “the compiler accepts it” as a sufficient reason. Extensions may occasionally
be useful, but FoundationCore should not drift into a dialect that silently narrows portability or complicates tooling.

Assembly uses NASM, targets x86_64 only, and assumes the System V ABI. Inline assembly is forbidden. Assembly exists to
implement carefully chosen hot paths or operations that are materially clearer or faster in handwritten machine-level
form. It is not a stylistic flourish.

## 3. Formatting as the mechanical baseline

The current formatter defines the mechanical shape of the code: 4-space indentation, a 100-column limit, Linux-style
braces, no tabs, and right-aligned pointer declarators. That baseline is not optional. Developers should not fight the
formatter.

At the same time, the formatter is not the whole style. A file can be perfectly formatted and still be poor
FoundationCore code. The formatter cannot decide when a function is too nested, whether an ownership boundary is
documented clearly enough, or whether a macro should have been a real function.

In practice, code should look like this:

```c
fc_status_t
fc_buffer_write(fc_buffer_t *buffer, const uint8_t *data, uint64_t size)
{
    if (buffer == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    if (data == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    if (size == 0U) {
        return FC_STATUS_OK;
    }

    return fc_buffer_write_impl(buffer, data, size);
}
```

The layout is plain on purpose. Short functions are not collapsed into one line. Conditions always use braces. Arguments
are named descriptively. Validation is immediate. The function body reads top-down without surprises.

## 4. File structure and module boundaries

FoundationCore follows the architecture document closely. Public headers live under `include/foundation_core/<module>/`.
Implementations live under `src/<module>/`. Tests live under `tests/<module>/`. That physical structure is not just an
organizational preference. It encodes ownership.

A module owns its concepts, its public API, and its invariants. Other modules may include its public headers, but they
should not reach around the API boundary into its internals. Files should therefore be named after stable concepts, not
temporary implementation accidents. A module should read like a small, coherent subsystem.

Because of that, header content should remain disciplined. A header is where a module explains what it offers. A source
file is where the module explains how it works. Crossing those concerns carelessly makes the whole tree harder to reason
about.

The architecture also establishes the include form used by the project:

```c
#include <foundation_core/core/result.h>
#include <foundation_core/world/chunk.h>
```

This angle-bracket form is the intended project-wide style because it reinforces that these are project-level public
headers, not local ad hoc includes.

## 5. Header style and include discipline

Headers use `#pragma once`. This is the project convention and should be applied uniformly.

Public headers should expose only what is needed for consumers of the module. Internal helper declarations,
implementation-only dependencies, or layout details that are not part of the API should remain in source files or
internal headers. FoundationCore is not a general-purpose library, so a header does not need to pretend to be more
standalone than the architecture requires, but it should still avoid unnecessary dependency drag.

A `.c` file should include its matching public header first when such a header exists. This catches missing dependencies
early and helps keep headers honest.

A normal source file therefore begins like this:

```c
#include <foundation_core/core/buffer.h>

#include <foundation_core/core/result.h>
#include <foundation_core/core/types.h>

#include <stdint.h>
#include <string.h>

#include <sys/mman.h>
```

The groups should be visually separated and conceptually ordered: matching header first, then other project headers,
then standard C headers, then platform or system headers.

Unused includes are forbidden. Forward declarations are preferred when they reduce coupling without obscuring meaning.
If a pointer to an opaque type is sufficient, do not include the entire defining header just to satisfy habit.

## 6. Naming: the authoritative project model

The project naming model is lowercase snake case for ordinary identifiers, with explicit prefixes and suffixes where
they carry semantic meaning. The guiding idea is that names should reveal both the namespace they belong to and the kind
of symbol they represent.

Functions, public and private, begin with `fc_`. Public APIs and internal helpers should look like they belong to the
same codebase.

```c
fc_status_t fc_buffer_reserve(fc_buffer_t *buffer, uint64_t capacity);
static fc_status_t fc_buffer_grow(fc_buffer_t *buffer, uint64_t required_capacity);
```

Typedef names use `fc_*_t`. This aligns with the current tool preference for lowercase typedefs ending in `_t`, while
still preserving the project namespace.

```c
typedef struct fc_buffer fc_buffer_t;
typedef enum fc_status fc_status_t;
```

Struct and enum tags use lowercase snake case with the `fc_` prefix.

```c
struct fc_buffer
{
    uint8_t *m_data;
    uint64_t m_length;
    uint64_t m_capacity;
};

enum fc_status
{
    FC_STATUS_OK = 0,
    FC_STATUS_INVALID_ARGUMENT,
    FC_STATUS_OUT_OF_MEMORY,
    FC_STATUS_IO_ERROR
};
```

Enum constants, macro names, and compile-time symbolic constants use uppercase snake case. In practice they should
usually be namespaced with `FC_`.

```c
#define FC_ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
static const uint64_t FC_RING_DEFAULT_CAPACITY = 1024U;
```

Global variables use the `g_` prefix.

```c
fc_logger_t *g_logger = NULL;
```

Fields that belong to internal or opaque implementation-defined structures use the `m_` prefix. This is how
FoundationCore signals “this field is part of the module’s managed internal state, not open struct data meant for casual
external manipulation.”

```c
struct fc_connection
{
    int32_t m_socket_fd;
    fc_buffer_t m_read_buffer;
    fc_buffer_t m_write_buffer;
    fc_protocol_state_t m_state;
};
```

Open value types may omit `m_` when direct field access is a deliberate part of the API. For example, a small
mathematical value object may reasonably expose plain fields if that makes the API more ergonomic and the type has no
hidden invariants.

```c
typedef struct fc_vec3_t
{
    float x;
    float y;
    float z;
} fc_vec3_t;
```

The rule is not “every field must use `m_`.” The rule is “internal state uses `m_`; externally legible value data may
remain plain when that is part of the design.”

## 7. Opaque types and public API shape

FoundationCore should prefer opaque types for stateful or invariant-heavy objects. An opaque type is declared in the
public header without exposing its internal layout, and fully defined only in the source file.

This is the preferred pattern for connections, allocators, registries, parsers, world/server objects, cryptographic
contexts, and other module-owned state.

```c
/* connection.h */
typedef struct fc_connection fc_connection_t;

fc_status_t fc_connection_create(fc_connection_t **out_connection);
void fc_connection_destroy(fc_connection_t *connection);
```

```c
/* connection.c */
struct fc_connection
{
    int32_t m_socket_fd;
    fc_buffer_t m_read_buffer;
    fc_buffer_t m_write_queue;
    fc_protocol_state_t m_state;
};
```

This keeps the API stable, reduces accidental misuse, and stops other modules from taking dependencies on layout details
they do not own.

Public APIs should also follow a stable parameter order. The context or receiver comes first. Immutable input parameters
come next. Mutable outputs come last.

```c
fc_status_t
fc_world_get_block(const fc_world_t *world, fc_block_position_t position, fc_block_state_t *out_block);
```

This order matters because it makes signatures more predictable. Over time that reduces mental friction across the whole
project.

## 8. Function design and control flow

FoundationCore allows early returns and encourages them when they simplify validation or make a fast-fail path obvious.
The project does not force a single exit point ideology onto every function. Instead, it distinguishes between simple
validation failures and resource-owning teardown paths.

Validation should happen at the top. Resource cleanup should happen through a `goto cleanup` block when multiple
resources or state transitions must be unwound safely.

A small validation-heavy function should therefore read like this:

```c
fc_status_t
fc_string_view_from_cstr(const char *string, fc_string_view_t *out_view)
{
    if (string == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    if (out_view == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    out_view->data = string;
    out_view->length = strlen(string);

    return FC_STATUS_OK;
}
```

A resource-owning function should instead centralize teardown:

```c
fc_status_t
fc_config_load(const char *path, fc_config_t **out_config)
{
    fc_status_t status = FC_STATUS_OK;
    FILE *file = NULL;
    fc_config_t *config = NULL;

    if (path == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    if (out_config == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    *out_config = NULL;

    status = fc_config_create(&config);
    if (status != FC_STATUS_OK) {
        goto cleanup;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        status = FC_STATUS_IO_ERROR;
        goto cleanup;
    }

    status = fc_config_read_from_file(config, file);
    if (status != FC_STATUS_OK) {
        goto cleanup;
    }

    *out_config = config;
    config = NULL;

cleanup:
    if (file != NULL) {
        fclose(file);
    }

    if (config != NULL) {
        fc_config_destroy(config);
    }

    return status;
}
```

This is the preferred style because it makes teardown auditable. Deeply nested cleanup ladders are harder to inspect and
easier to break.

Functions should remain focused. Eight parameters is the practical upper bound. If a signature pushes beyond that, the
design should be questioned. Either the function is doing too much, or some state should be grouped into a dedicated
context or configuration type.

Nesting should not exceed three levels in normal code. When a function becomes more nested than that, it is a signal
that the logic should be inverted, extracted, or decomposed. FoundationCore encourages `continue` when it flattens loop
logic.

```c
for (uint64_t i = 0; i < player_count; ++i) {
    fc_player_t *player = players[i];

    if (player == NULL) {
        continue;
    }

    if (!fc_player_is_connected(player)) {
        continue;
    }

    status = fc_player_flush(player);
    if (status != FC_STATUS_OK) {
        return status;
    }
}
```

The flow is easier to scan than a nested chain of `if` statements.

## 9. Error handling: the project-wide default

The project settled on a simple, explicit error model: strongly typed status enums plus out-parameters. That is the
default across the codebase.

In other words, FoundationCore does not use raw integer error codes as its stylistic baseline, and it does not force
generic “result unions” onto every API. Most operations should return `fc_status_t` or another well-namespaced module
status type and write successful results through output parameters.

```c
typedef enum fc_status
{
    FC_STATUS_OK = 0,
    FC_STATUS_INVALID_ARGUMENT,
    FC_STATUS_OUT_OF_MEMORY,
    FC_STATUS_IO_ERROR,
    FC_STATUS_PROTOCOL_ERROR,
    FC_STATUS_UNDERFLOW
} fc_status_t;

fc_status_t
fc_buffer_read_u32(fc_buffer_t *buffer, uint32_t *out_value);
```

The caller can read the contract at a glance: the function may fail, and the decoded value is returned through
`out_value` only on success.

On failure, out-pointers must be left in a safe state. Pointer outputs must be set to `NULL`. Scalar outputs should be
set to zero where doing so is meaningful and not misleading. The goal is that a caller never receives partially
initialized garbage because an operation failed halfway through.

```c
fc_status_t
fc_packet_decode_login_start(fc_buffer_t *buffer, fc_login_start_packet_t **out_packet)
{
    fc_status_t status = FC_STATUS_OK;
    fc_login_start_packet_t *packet = NULL;

    if (buffer == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    if (out_packet == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    *out_packet = NULL;

    status = fc_login_start_packet_create(&packet);
    if (status != FC_STATUS_OK) {
        return status;
    }

    status = fc_codec_read_string(buffer, &packet->m_username);
    if (status != FC_STATUS_OK) {
        fc_login_start_packet_destroy(packet);
        return status;
    }

    *out_packet = packet;
    return FC_STATUS_OK;
}
```

The architecture currently includes a `result.h` concept in `core`. That is acceptable, but the style guide defines a
narrower rule: the generic result abstraction should not replace the project’s ordinary public API language unless a
particular internal boundary benefits materially from it. The default remains `fc_status_t` plus out-parameters.

## 10. Ownership, memory, and lifetime

Ownership must always be explicit. A function that allocates memory, borrows memory, stores memory, or transfers memory
must state that fact clearly in its name and in its documentation.

FoundationCore uses naming to signal ownership semantics. `create` and `destroy` are for heap-owned objects. `init` and
`deinit` are for caller-owned storage. `clone` duplicates ownership. `borrow` may be used when an interface
intentionally returns or stores a non-owning reference.

```c
fc_status_t fc_buffer_init(fc_buffer_t *buffer, uint64_t initial_capacity);
void fc_buffer_deinit(fc_buffer_t *buffer);

fc_status_t fc_buffer_create(fc_buffer_t **out_buffer, uint64_t initial_capacity);
void fc_buffer_destroy(fc_buffer_t *buffer);

fc_status_t fc_string_clone(fc_string_t **out_string, const fc_string_t *source);
```

A caller reading only the name should already have a strong guess about who owns the returned object and who must
release it.

Every public function that receives pointers must validate them at runtime. Every boundary where a pointer changes hands
must be checked. The caller is still responsible for passing semantically valid data, but the callee is responsible for
defending its own invariants.

Documentation should explicitly answer four questions whenever ownership is not trivial: who allocates, who frees,
whether the callee stores the pointer, and how long the pointed-to data must remain valid.

A good public API comment looks like this:

```c
/**
 * Decodes a login start packet from the provided byte buffer.
 *
 * This function allocates a new packet object on success and transfers ownership
 * of that object to the caller through @p out_packet. The caller becomes
 * responsible for releasing it with fc_login_start_packet_destroy().
 *
 * The function validates both input pointers at runtime. On failure,
 * @p out_packet is set to NULL.
 *
 * @param buffer     Input byte buffer positioned at the start of the packet body.
 * @param out_packet Output location that receives the allocated packet on success.
 * @return           FC_STATUS_OK on success or an error status on failure.
 */
fc_status_t
fc_packet_decode_login_start(fc_buffer_t *buffer, fc_login_start_packet_t **out_packet);
```

## 11. Assertions and runtime validation

One of the main refinements from the design discussion was separating assertions from runtime validation.

FoundationCore uses assertions aggressively for invariants, impossible states, internal contracts, and diagnostics. At
the same time, FoundationCore does not rely on assertions alone for public correctness. External inputs, public API
parameters, file contents, network data, and ABI boundaries must be validated at runtime even in release builds.

That means the correct pattern is often both an assertion and a runtime check.

```c
fc_status_t
fc_buffer_read_u32(fc_buffer_t *buffer, uint32_t *out_value)
{
    assert(buffer != NULL);
    assert(out_value != NULL);

    if (buffer == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    if (out_value == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    if (buffer->m_length - buffer->m_read_cursor < sizeof(uint32_t)) {
        return FC_STATUS_UNDERFLOW;
    }

    memcpy(out_value, buffer->m_data + buffer->m_read_cursor, sizeof(uint32_t));
    buffer->m_read_cursor += sizeof(uint32_t);

    return FC_STATUS_OK;
}
```

The assertion makes programmer mistakes obvious during debugging. The runtime check preserves release safety and correct
error reporting.

If the project later introduces custom assertion wrappers such as `FC_ASSERT`, they should preserve this same
philosophy: assertions supplement runtime validation; they do not replace it.

## 12. Comments and documentation style

FoundationCore prefers heavily documented code, but not noisy code. Comments exist to explain intent, invariants,
assumptions, ownership, and non-obvious design choices. They should not narrate every trivial assignment.

Doxygen-style documentation is the standard for public APIs. Every public type, public enum, public macro, and public
function should have a proper documentation block. Complex internal functions should also be documented when their
correctness depends on subtle invariants, teardown paths, memory semantics, protocol assumptions, or performance
constraints.

The most important distinction is that comments should explain both what the code does and why the code is written that
way.

```c
/**
 * Grows the buffer capacity to satisfy a pending write.
 *
 * The function rounds the requested capacity upward instead of growing to the
 * exact required byte count. This reduces allocator churn during bursty packet
 * traffic and preserves amortized linear append behavior.
 *
 * The buffer contents remain valid on success. On failure, the original buffer
 * is left unchanged.
 */
static fc_status_t
fc_buffer_grow(fc_buffer_t *buffer, uint64_t required_capacity)
{
    /* implementation */
}
```

Performance-sensitive code deserves especially careful comments, because the reader must know which parts are
fundamental constraints and which parts are deliberate optimizations.

## 13. Expressions, declarations, and local clarity

FoundationCore uses one declaration per line. Variables are declared close to first meaningful use. `const` should be
used aggressively where it sharpens the contract and protects against accidental mutation.

The project standardizes on fixed-width integer types such as `uint32_t`, `int64_t`, and `uint8_t`. Bare `int`,
`unsigned`, `long`, and similar types should be avoided unless an external API requires those exact types.

Assignments inside conditions are forbidden. The comma operator is effectively forbidden outside narrow unavoidable
cases such as a `for` header. The ternary operator is forbidden. These rules exist because FoundationCore values
readability over compression.

The preferred style is therefore explicit, even when it costs an extra line or two.

```c
fc_status_t status = FC_STATUS_OK;
uint32_t value = 0U;

status = fc_buffer_read_u32(buffer, &value);
if (status != FC_STATUS_OK) {
    return status;
}

if (value == 0U) {
    return FC_STATUS_PROTOCOL_ERROR;
}
```

Not this:

```c
if ((status = fc_buffer_read_u32(buffer, &value)) != FC_STATUS_OK) {
    return status;
}
```

And not this:

```c
return value != 0U ? FC_STATUS_OK : FC_STATUS_PROTOCOL_ERROR;
```

The project intentionally chooses the more verbose spelling because the failure path and the decision point remain
easier to scan.

## 14. Macros and inline utilities

FoundationCore avoids macros unless they are materially necessary. A `static inline` function is preferred over a
function-like macro whenever the language can express the behavior directly.

When macros are necessary, they must be safe, obviously macro-shaped, and free of hidden control flow. Macro arguments
must be parenthesized. Statement-like multiline macros must use the classic `do { ... } while (0)` shape.

A safe macro therefore looks like this:

```c
#define FC_MIN(a, b) (((a) < (b)) ? (a) : (b))
```

But even here the project should ask whether a typed `static inline` utility would be better.

```c
static inline uint64_t
fc_min_u64(uint64_t left, uint64_t right)
{
    if (left < right) {
        return left;
    }

    return right;
}
```

The second version is usually preferred because it is type-safe, debuggable, and easier to document.

## 15. Concrete C examples: the house style in practice

A public opaque API should look like this:

```c
/* include/foundation_core/core/ring.h */
#pragma once

#include <stdint.h>

#include <foundation_core/core/status.h>

typedef struct fc_ring fc_ring_t;

/**
 * Allocates and initializes a single-producer single-consumer ring buffer.
 *
 * Ownership of the created object is transferred to the caller through
 * @p out_ring. The caller must release it with fc_ring_destroy().
 *
 * @param out_ring  Output location that receives the allocated ring.
 * @param capacity  Requested element capacity. Must be greater than zero.
 * @return          FC_STATUS_OK on success or an error code on failure.
 */
fc_status_t
fc_ring_create(fc_ring_t **out_ring, uint64_t capacity);

void
fc_ring_destroy(fc_ring_t *ring);

fc_status_t
fc_ring_push(fc_ring_t *ring, void *element);

fc_status_t
fc_ring_pop(fc_ring_t *ring, void **out_element);
```

The implementation should hide its state cleanly:

```c
/* src/core/ring.c */
#include <foundation_core/core/ring.h>

#include <assert.h>
#include <stdlib.h>

struct fc_ring
{
    void **m_elements;
    uint64_t m_capacity;
    uint64_t m_read_index;
    uint64_t m_write_index;
    uint64_t m_count;
};

fc_status_t
fc_ring_create(fc_ring_t **out_ring, uint64_t capacity)
{
    fc_ring_t *ring = NULL;

    assert(out_ring != NULL);

    if (out_ring == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    *out_ring = NULL;

    if (capacity == 0U) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    ring = calloc(1U, sizeof(*ring));
    if (ring == NULL) {
        return FC_STATUS_OUT_OF_MEMORY;
    }

    ring->m_elements = calloc(capacity, sizeof(*ring->m_elements));
    if (ring->m_elements == NULL) {
        free(ring);
        return FC_STATUS_OUT_OF_MEMORY;
    }

    ring->m_capacity = capacity;
    *out_ring = ring;

    return FC_STATUS_OK;
}
```

This is the target feel of the codebase: explicit, quiet, and unsurprising.

## 16. NASM style and assembly discipline

Assembly in FoundationCore is part of the same codebase culture. It should look like FoundationCore code translated into
NASM, not like a separate subculture with a different sense of discipline.

All assembly uses lowercase mnemonics, lowercase registers, lowercase directives, lowercase labels, and `0x` hexadecimal
notation. Exported function names must match the C naming convention and begin with `fc_`. Internal labels should be
descriptive and local.

Every assembly routine requires a documentation block explaining what it does, why it exists in assembly instead of C,
which ABI it follows, which arguments it expects, what it returns, which registers it clobbers, what stack alignment it
assumes, and which ISA or alignment assumptions must hold.

A typical FoundationCore assembly function therefore looks like this:

```nasm
; WHAT:
;   Copies exactly 64 bytes from source to destination using aligned SIMD loads.
;
; WHY:
;   This routine exists because profiling showed the scalar C implementation on the
;   chunk-light fast path was measurably slower than a fixed-size SIMD variant.
;
; ABI:
;   x86_64 System V
;
; INPUTS:
;   rdi = destination pointer
;   rsi = source pointer
;
; OUTPUTS:
;   none
;
; CLOBBERS:
;   xmm0, xmm1, xmm2, xmm3
;
; ASSUMPTIONS:
;   - rdi is 16-byte aligned
;   - rsi is 16-byte aligned
;   - the CPU supports the required SIMD instructions
;
section .text
global fc_copy_64_aligned

fc_copy_64_aligned:
    push rbp
    mov rbp, rsp

    movdqa xmm0, [rsi]
    movdqa xmm1, [rsi + 16]
    movdqa xmm2, [rsi + 32]
    movdqa xmm3, [rsi + 48]

    movdqa [rdi], xmm0
    movdqa [rdi + 16], xmm1
    movdqa [rdi + 32], xmm2
    movdqa [rdi + 48], xmm3

    mov rsp, rbp
    pop rbp
    ret
```

The default FoundationCore rule is to use a standard prologue and epilogue. This favors readability, debugger
friendliness, and consistent review. The only exception is a documented hot-path leaf routine where omitting the frame
materially matters and the author explains why the omission is justified.

For example, a frame-less leaf is acceptable only when the reason is made explicit:

```nasm
; WHAT:
;   Returns whether a 16-byte aligned region is entirely zero.
;
; WHY:
;   Leaf hot-path helper used in chunk diff scanning. The frame is omitted to reduce
;   instruction count on a heavily repeated micro-path.
;
; ABI:
;   x86_64 System V
;
; INPUTS:
;   rdi = pointer to 16-byte aligned memory
;
; OUTPUTS:
;   eax = 1 if all zero, 0 otherwise
;
; CLOBBERS:
;   xmm0
;
; ASSUMPTIONS:
;   - rdi is 16-byte aligned
;   - frame omission is safe because the function does not call out
;
section .text
global fc_block_is_zero_16

fc_block_is_zero_16:
    pxor xmm0, xmm0
    pcmpeqb xmm0, [rdi]
    pmovmskb eax, xmm0
    cmp eax, 0xffff
    sete al
    movzx eax, al
    ret
```

Even in optimized cases, the routine still documents itself like the rest of the project.

## 17. C and NASM interoperability

Every assembly function callable from C must have a matching declaration in a header. Names must match exactly. Argument
order must follow the C declaration. Stack alignment must be preserved at call boundaries. Ownership rules still apply
when pointers cross the language boundary.

A C declaration and its assembly implementation should form a readable pair.

```c
/* include/foundation_core/math/aabb.h */
#pragma once

#include <foundation_core/core/status.h>

fc_status_t
fc_aabb_sweep_aligned(const fc_aabb_t *moving,
                      const fc_aabb_t *stationary,
                      fc_sweep_result_t *out_result);
```

```nasm
section .text
global fc_aabb_sweep_aligned

fc_aabb_sweep_aligned:
    push rbp
    mov rbp, rsp
    ; implementation
    mov rsp, rbp
    pop rbp
    ret
```

If an assembly routine depends on struct layout, field offsets, alignment, endianness, aliasing assumptions, or a
specific SIMD level, those dependencies must be documented close to the routine and, where practical, shared through
centralized definitions rather than duplicated magic offsets.

## 18. Architecture-sensitive and unsafe code

FoundationCore forbids inline assembly. Unaligned memory access is forbidden unless it is explicitly justified and
documented as safe. Endianness-sensitive code must state which byte order it expects and where conversion occurs. SIMD
code must state which ISA level it requires, which alignment rules it depends on, and whether a scalar fallback exists.

This applies to both C and assembly. If a piece of code assumes little-endian layout or a 16-byte aligned load, that
assumption must not remain implicit.

A good example comment for endian-sensitive code is simple and direct:

```c
/*
 * The protocol field is encoded as little-endian on the wire. Decoding happens
 * at the boundary, so all internal storage past this point is host-endian.
 */
```

A good example comment for a SIMD path is equally explicit:

```c
/*
 * This path assumes 16-byte alignment and SSE2 availability. Callers must route
 * unaligned input through the scalar fallback.
 */
```

## 19. Testing style

Tests are first-class code and should follow the same naming, formatting, and clarity standards as production files. A
test file should read like executable documentation for a module contract.

Tests should therefore use descriptive names, explicit setup, and straightforward assertions.

```c
static void
fc_buffer_read_u32_returns_underflow_when_not_enough_bytes(void **state)
{
    fc_buffer_t buffer;
    uint32_t value = 0U;
    fc_status_t status = FC_STATUS_OK;

    (void) state;

    fc_buffer_init(&buffer, 0U);
    status = fc_buffer_read_u32(&buffer, &value);

    assert_int_equal(status, FC_STATUS_UNDERFLOW);
    assert_int_equal(value, 0U);

    fc_buffer_deinit(&buffer);
}
```

The project should prefer tests that reveal the contract rather than tests that chase overly dense abstraction in the
test harness.

## 20. Tooling authority and current mismatches

The formatter and linter are important, but this document is the semantic authority. Where tooling and specification
disagree, the disagreement should be resolved deliberately rather than ignored.

At the time this guide was written, three important mismatches exist.

First, the architecture specifies project includes in the form `<foundation_core/...>`, but the current formatter
include categorization recognizes project headers using the quoted form `"foundation_core/..."`. This means the style
guide and formatter do not yet describe the same include taxonomy. The correct resolution is to keep the architecture’s
angle-bracket include style and adjust the formatter include regex accordingly.

Second, the current linter naming rules strongly support lowercase snake case and `_t` typedef suffixes, but they do not
fully express the desired project prefixes such as `fc_` for functions and tags or `m_` for internal fields. Those
details still require manual review unless the linter configuration is expanded.

Third, the current linter configuration for struct and enum naming can conflict with the preferred C spelling
`struct fc_buffer` / `fc_buffer_t` and `enum fc_status` / `fc_status_t`. The specification intentionally keeps the
familiar two-name C model because it is clearer for a systems codebase, but the naming checks should be updated so they
validate that model instead of pushing the code toward a contradictory style.

The practical rule is therefore simple: developers should follow this guide, run the formatter and linter, and treat any
mismatch between tools and the specification as a signal to improve the tools rather than silently weakening the style.

## 21. Final examples of good FoundationCore style

A small public value type that is intentionally transparent:

```c
#pragma once

#include <stdint.h>

typedef struct fc_block_position_t
{
    int32_t x;
    int32_t y;
    int32_t z;
} fc_block_position_t;

fc_status_t
fc_block_position_encode(fc_block_position_t position, uint64_t *out_encoded);
```

A stateful opaque type with module-managed invariants:

```c
#pragma once

#include <stdint.h>

#include <foundation_core/core/status.h>

typedef struct fc_world fc_world_t;

fc_status_t
fc_world_create(fc_world_t **out_world);

void
fc_world_destroy(fc_world_t *world);

fc_status_t
fc_world_set_block(fc_world_t *world,
                   fc_block_position_t position,
                   fc_block_state_t block,
                   fc_block_state_t *out_previous_block);
```

A loop written in the preferred explicit style:

```c
for (uint64_t i = 0U; i < packet_count; ++i) {
    fc_packet_t *packet = packets[i];

    if (packet == NULL) {
        continue;
    }

    status = fc_packet_encode(packet, buffer);
    if (status != FC_STATUS_OK) {
        return status;
    }
}
```

A switch with an explicit default and no ambiguous fallthrough:

```c
switch (connection->m_state) {
    case FC_PROTOCOL_STATE_HANDSHAKE:
        return fc_handle_handshake(connection, packet);

    case FC_PROTOCOL_STATE_STATUS:
        return fc_handle_status(connection, packet);

    case FC_PROTOCOL_STATE_LOGIN:
        return fc_handle_login(connection, packet);

    case FC_PROTOCOL_STATE_PLAY:
        return fc_handle_play(connection, packet);

    default:
        return FC_STATUS_PROTOCOL_ERROR;
}
```

A documentation block that explains both what and why:

```c
/**
 * Flushes buffered outbound packets for a player connection.
 *
 * The flush is performed in packet order because later protocol messages may
 * depend on earlier state transitions already having been observed by the client.
 * Reordering for throughput would make the networking layer faster but would also
 * make state bugs significantly harder to reason about.
 */
fc_status_t
fc_player_flush(fc_player_t *player);
```

## 22. Closing rule

The simplest way to decide whether code fits FoundationCore is to ask four questions.

Does the code make ownership obvious?

Does the code make failure explicit?

Does the code respect the architectural boundary of the module it lives in?

Would the next engineer understand why it is written this way without reverse-engineering hidden assumptions?

If the answer to any of those questions is no, the code is not finished, even if it compiles and even if the formatter
is happy.