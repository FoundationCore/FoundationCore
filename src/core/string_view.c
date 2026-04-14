/**
 * @file string_view.c
 * @brief Implementation of the FC-002 example module.
 */
#include <foundation_core/core/status.h>
#include <foundation_core/core/string_view.h>

#include <assert.h>
#include <string.h>

fc_status_t fc_string_view_from_cstr(const char *string, fc_string_view_t *out_view)
{
    assert(string != NULL);
    assert(out_view != NULL);

    if (string == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    if (out_view == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    out_view->data   = string;
    out_view->length = strlen(string);

    return FC_STATUS_OK;
}

fc_status_t fc_string_view_slice(const fc_string_view_t view,
                                 const uint64_t offset,
                                 const uint64_t length,
                                 fc_string_view_t *out_slice)
{
    assert(out_slice != NULL);

    if (out_slice == NULL) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    out_slice->data   = NULL;
    out_slice->length = 0U;

    if (offset > view.length) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    if (length > (view.length - offset)) {
        return FC_STATUS_INVALID_ARGUMENT;
    }

    out_slice->data   = view.data + offset;
    out_slice->length = length;

    return FC_STATUS_OK;
}

bool fc_string_view_equal(const fc_string_view_t left, const fc_string_view_t right)
{
    if (left.length != right.length) {
        return false;
    }

    if (left.length == 0U) {
        return true;
    }

    return (bool) (memcmp(left.data, right.data, left.length) == 0);
}

bool fc_string_view_is_empty(const fc_string_view_t view)
{
    return (bool) (view.length == 0U);
}