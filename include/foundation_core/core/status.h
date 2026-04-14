#pragma once

/**
 * @file foundation_core/core/status.h
 * @brief Project-wide status and error surface for FoundationCore.
 *
 * FoundationCore uses typed status codes plus out-parameters as the default public API error model.
 * Successful return values are written through output parameters. The returned status communicates
 * whether the operation succeeded and, if it did not, why it failed.
 */

#include <stdbool.h>

/**
 * @brief Project-wide operation status.
 *
 * The enumerators in this type are intentionally generic so they can be used by
 * any module without leaking subsystem-specific implementation details into the
 * shared public contract surface.
 */
typedef enum fc_status {
    /**
     * @brief Operation completed successfully.
     */
    FC_STATUS_OK = 0,

    /**
     * @brief A required argument was invalid, missing, or inconsistent.
     */
    FC_STATUS_INVALID_ARGUMENT,

    /**
     * @brief Memory allocation failed.
     */
    FC_STATUS_OUT_OF_MEMORY,

    /**
     * @brief An arithmetic or capacity overflow occurred.
     */
    FC_STATUS_OVERFLOW,

    /**
     * @brief The operation required more input or stored data than was available.
     */
    FC_STATUS_UNDERFLOW,

    /**
     * @brief An input or output operation failed.
     */
    FC_STATUS_IO_ERROR,

    /**
     * @brief Input data had an invalid or unsupported format.
     */
    FC_STATUS_FORMAT_ERROR,

    /**
     * @brief Protocol data violated an expected rule or invariant.
     */
    FC_STATUS_PROTOCOL_ERROR,

    /**
     * @brief The requested object or entry was not found.
     */
    FC_STATUS_NOT_FOUND,

    /**
     * @brief The requested object already exists.
     */
    FC_STATUS_ALREADY_EXISTS,

    /**
     * @brief The requested operation is not supported.
     */
    FC_STATUS_UNSUPPORTED,

    /**
     * @brief An internal invariant or unexpected condition failed.
     */
    FC_STATUS_INTERNAL_ERROR
} fc_status_t;

/**
 * @brief Returns whether a status value represents success.
 *
 * @param status Status value to inspect.
 * @return `true` when @p status is `FC_STATUS_OK`; otherwise `false`.
 */
bool fc_status_is_ok(fc_status_t status);

/**
 * @brief Returns a stable human-readable string for a status value.
 *
 * The returned string is statically allocated and must not be freed or
 * modified by the caller.
 *
 * @param status Status value to format.
 * @return Constant string describing the status.
 */
const char *fc_status_to_cstr(fc_status_t status);