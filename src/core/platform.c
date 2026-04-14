/**
 * @file platform.c
 * @brief Implementation of narrow low-level platform helpers.
 */

#include <foundation_core/core/platform.h>

#include <assert.h>
#include <stdint.h>

bool fc_platform_is_aligned(const void *pointer, const size_t alignment)
{
    uintptr_t address = 0U;

    if (alignment == 0U) {
        return false;
    }

    address = (uintptr_t) pointer;
    return (address % alignment) == 0U;
}

fc_status_t fc_platform_align_up_u64(const uint64_t value,
                                     const uint64_t alignment,
                                     uint64_t *out_aligned_value)
{
    uint64_t remainder = 0U;
    uint64_t delta     = 0U;

    assert(out_aligned_value != NULL);

    if (out_aligned_value == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    *out_aligned_value = 0U;

    if (alignment == 0U) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    remainder = value % alignment;
    if (remainder == 0U) {
        *out_aligned_value = value;
        return FC_STATUS_OK;
    }

    delta = alignment - remainder;
    if (value > UINT64_MAX - delta) {
        return FC_STATUS_OVERFLOW;
    }

    *out_aligned_value = value + delta;
    return FC_STATUS_OK;
}

uint16_t fc_platform_bswap_u16(const uint16_t value)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap16(value);
#else
    return (uint16_t) (((value & 0x00ffU) << 8U) | ((value & 0xff00U) >> 8U));
#endif
}

uint32_t fc_platform_bswap_u32(const uint32_t value)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(value);
#else
    return ((value & 0x000000ffU) << 24U) | ((value & 0x0000ff00U) << 8U) |
           ((value & 0x00ff0000U) >> 8U) | ((value & 0xff000000U) >> 24U);
#endif
}

uint64_t fc_platform_bswap_u64(const uint64_t value)
{
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap64(value);
#else
    return ((value & 0x00000000000000ffULL) << 56U) | ((value & 0x000000000000ff00ULL) << 40U) |
           ((value & 0x0000000000ff0000ULL) << 24U) | ((value & 0x00000000ff000000ULL) << 8U) |
           ((value & 0x000000ff00000000ULL) >> 8U) | ((value & 0x0000ff0000000000ULL) >> 24U) |
           ((value & 0x00ff000000000000ULL) >> 40U) | ((value & 0xff00000000000000ULL) >> 56U);
#endif
}