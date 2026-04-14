/**
 * @file status.c
 * @brief Implementation of the shared project-wide status surface.
 */

#include <foundation_core/core/status.h>

bool fc_status_is_ok(const fc_status_t status)
{
    return status == FC_STATUS_OK;
}

const char *fc_status_to_cstr(const fc_status_t status)
{
    switch (status) {
        case FC_STATUS_OK:
            return "ok";

        case FC_STATUS_INVALID_ARGUMENT:
            return "invalid argument";

        case FC_STATUS_OUT_OF_MEMORY:
            return "out of memory";

        case FC_STATUS_OVERFLOW:
            return "overflow";

        case FC_STATUS_UNDERFLOW:
            return "underflow";

        case FC_STATUS_IO_ERROR:
            return "i/o error";

        case FC_STATUS_FORMAT_ERROR:
            return "format error";

        case FC_STATUS_PROTOCOL_ERROR:
            return "protocol error";

        case FC_STATUS_NOT_FOUND:
            return "not found";

        case FC_STATUS_ALREADY_EXISTS:
            return "already exists";

        case FC_STATUS_UNSUPPORTED:
            return "unsupported";

        case FC_STATUS_INTERNAL_ERROR:
            return "internal error";

        default:
            return "unknown status";
    }
}
