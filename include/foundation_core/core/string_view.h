#pragma once

/**
 * @file foundation_core/core/string_view.h
 * @brief Small example module that demonstrates the FC-002 baseline contract.
 *
 * This module exists to demonstrate the shared public language defined by the
 * FC-002 task. It is intentionally simple. The type is transparent, the API
 * uses the shared status surface, and the functions illustrate the project-wide
 * include style, naming pattern, validation rules, and ownership conventions.
 */

#include <foundation_core/core/status.h>

#include <stdint.h>

/**
 * @brief Immutable non-owning string slice.
 *
 * This type does not own the referenced memory. The caller remains responsible
 * for ensuring the pointed-to character range remains valid for the duration of
 * the view's use.
 */
typedef struct fc_string_view_t {
    /**
     * @brief Pointer to the first character of the view.
     */
    const char *data;

    /**
     * @brief Length of the view in bytes, excluding any trailing null byte.
     */
    uint64_t length;
} fc_string_view_t;

/**
 * @brief Creates a string view over a null-terminated C string.
 *
 * This function does not allocate. It borrows the input string memory and
 * writes a non-owning view through @p out_view.
 *
 * @param string   Null-terminated source string.
 * @param out_view Output location that receives the resulting view.
 * @return `FC_STATUS_OK` on success or an error status on failure.
 */
fc_status_t fc_string_view_from_cstr(const char *string, fc_string_view_t *out_view);

/**
 * @brief Creates a subslice of an existing string view.
 *
 * The resulting view borrows from the same underlying storage as the input
 * view. No allocation occurs.
 *
 * @param view      Source view.
 * @param offset    Start offset of the requested slice.
 * @param length    Length of the requested slice.
 * @param out_slice Output location that receives the resulting slice.
 * @return `FC_STATUS_OK` on success or an error status on failure.
 */
fc_status_t fc_string_view_slice(fc_string_view_t view,
                                 uint64_t offset,
                                 uint64_t length,
                                 fc_string_view_t *out_slice);

/**
 * @brief Compares two string views for byte-wise equality.
 *
 * @param left  Left-hand operand.
 * @param right Right-hand operand.
 * @return `true` when both views have the same length and contents; otherwise `false`.
 */
bool fc_string_view_equal(fc_string_view_t left, fc_string_view_t right);

/**
 * @brief Returns whether a string view is empty.
 *
 * @param view View to inspect.
 * @return `true` when the view length is zero; otherwise `false`.
 */
bool fc_string_view_is_empty(fc_string_view_t view);