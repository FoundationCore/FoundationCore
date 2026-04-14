#pragma once

/**
 * @file foundation_core/core/platform.h
 * @brief Narrow platform and low-level helper surface for FoundationCore.
 *
 * This header centralizes a small number of low-level helpers that would otherwise be repeated ad
 * hoc throughout the codebase. It is intentionally narrow. It should not become a dumping ground
 * for unrelated utilities.
 */

#include <foundation_core/core/status.h>

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Compiler branch prediction hint for likely conditions.
 *
 * This macro is a narrow wrapper around compiler support where available.
 * Callers should use it sparingly and only for conditions that are clearly
 * justified by profiling or stable control-flow expectations.
 */
#if defined(__GNUC__) || defined(__clang__)
#define FC_LIKELY(expression) __builtin_expect(!!(expression), 1)
#else
#define FC_LIKELY(expression) (expression)
#endif

/**
 * @brief Compiler branch prediction hint for unlikely conditions.
 *
 * This macro is a narrow wrapper around compiler support where available.
 * Callers should use it sparingly and only for conditions that are clearly
 * justified by profiling or stable control-flow expectations.
 */
#if defined(__GNUC__) || defined(__clang__)
#define FC_UNLIKELY(expression) __builtin_expect(!!(expression), 0)
#else
#define FC_UNLIKELY(expression) (expression)
#endif

/**
 * @brief Returns the number of elements in a fixed-size array.
 *
 * This macro must only be used with actual arrays, not pointers.
 */
#define FC_ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

/**
 * @brief Returns whether a pointer value is aligned to the provided alignment.
 *
 * @param pointer   Pointer value to inspect.
 * @param alignment Expected alignment in bytes.
 * @return `true` when the pointer is aligned to @p alignment; otherwise `false`.
 */
bool fc_platform_is_aligned(const void *pointer, size_t alignment);

/**
 * @brief Aligns an unsigned 64-bit value upward to the next aligned boundary.
 *
 * The result is written through @p out_aligned_value on success.
 *
 * @param value             Input value to align.
 * @param alignment         Alignment in bytes. Must be non-zero.
 * @param out_aligned_value Output location that receives the aligned result.
 * @return `FC_STATUS_OK` on success or an error status on failure.
 */
fc_status_t
fc_platform_align_up_u64(uint64_t value, uint64_t alignment, uint64_t *out_aligned_value);

/**
 * @brief Byte-swaps a 16-bit unsigned integer.
 *
 * @param value Input value.
 * @return Byte-swapped result.
 */
uint16_t fc_platform_bswap_u16(uint16_t value);

/**
 * @brief Byte-swaps a 32-bit unsigned integer.
 *
 * @param value Input value.
 * @return Byte-swapped result.
 */
uint32_t fc_platform_bswap_u32(uint32_t value);

/**
 * @brief Byte-swaps a 64-bit unsigned integer.
 *
 * @param value Input value.
 * @return Byte-swapped result.
 */
uint64_t fc_platform_bswap_u64(uint64_t value);