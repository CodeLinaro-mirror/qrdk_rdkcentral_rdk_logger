#ifndef RDK_DYNAMIC_LOGGER_PARSER_H
#define RDK_DYNAMIC_LOGGER_PARSER_H

#include <stddef.h>
#include <string.h>

static int rdk_dyn_log_parse_request(const unsigned char *buf, size_t length, const char *program, char *component, size_t component_capacity, unsigned char *log_level)
{
    const size_t signature_length = 4;
    const size_t app_offset = 7;
    size_t program_length;
    size_t app_length;
    size_t component_length_offset;
    size_t component_length;
    size_t component_offset;

    if (buf == NULL || program == NULL || component == NULL || log_level == NULL || component_capacity == 0 || length < app_offset)
        return 0;
    if ((size_t)buf[4] + signature_length + 1 != length)
        return 0;
    if (memcmp(buf, "COMC", signature_length) != 0)
        return 0;

    program_length = strlen(program);
    app_length = buf[6];
    if (app_length != program_length || app_length > length - app_offset)
        return 0;
    if (memcmp(buf + app_offset, program, app_length) != 0)
        return 0;

    component_length_offset = app_offset + app_length;
    if (component_length_offset >= length)
        return 0;
    component_length = buf[component_length_offset];
    component_offset = component_length_offset + 1;
    if (component_length >= component_capacity || component_length > length - component_offset || component_offset + component_length != length)
        return 0;

    memcpy(component, buf + component_offset, component_length);
    component[component_length] = '\0';
    *log_level = buf[5];
    return 1;
}

#endif
