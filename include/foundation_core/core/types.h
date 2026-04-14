#pragma once

/**
 * @file foundation_core/core/types.h
 * @brief Shared semantic types used across FoundationCore.
 *
 * This header defines a small set of project-level types that carry semantic meaning across module
 * boundaries. It intentionally does not wrap every C primitive type.
 *
 * The project standard remains the direct use of fixed-width integer types from the standard
 * library unless a dedicated semantic alias is useful to an API contract cleaner.
 */

#include <stdint.h>

/**
 * @brief Canonical byte type used for raw buffers and wire data.
 */
typedef uint8_t fc_byte_t;

/**
 * @brief Signed 32-bit VarInt logical type used by the protocol layer.
 *
 * This type represents the decoded semantic value, not its encoded byte shape.
 */
typedef int32_t fc_varint_t;

/**
 * @brief Signed 64-bit VarLong logical type used by the protocol layer.
 *
 * This type represents the decoded semantic value, not its encoded byte shape.
 */
typedef int64_t fc_varlong_t;

/**
 * @brief Millisecond counter used for time-related APIs.
 */
typedef uint64_t fc_millis_t;

/**
 * @brief Tick counter used for game-loop or scheduler-related APIs.
 */
typedef uint64_t fc_tick_t;

/**
 * @brief Immutable byte view over caller-owned memory.
 *
 * This is a transparent value type. It does not own the referenced memory.
 *
 * Callers are responsible for ensuring the pointed-to data remains valid for the duration of the
 * view's use.
 */
typedef struct fc_bytes_view_t {
    /**
     * @brief Pointer to the first byte of the viewed range.
     */
    const fc_byte_t *data;

    /**
     * @brief Length of the viewed range in bytes.
     */
    uint64_t length;
} fc_bytes_view_t;