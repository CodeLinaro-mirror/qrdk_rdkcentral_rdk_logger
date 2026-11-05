#ifndef RDK_DYNAMIC_LOGGER_PARSER_H
#define RDK_DYNAMIC_LOGGER_PARSER_H

#include "rdk_dynamic_logger.h"

#include <stddef.h>
#include <string.h>

static int rdk_dyn_log_parse_request(
        const unsigned char *buf,
        size_t length,
        const char *program,
        char *component,
        size_t component_capacity,
        unsigned char *log_level)
{
    const size_t app_capacity = RDK_DYNAMIC_LOG_APP_NAME_MAX + 1;
    const size_t component_field_capacity = RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX + 1;
    const unsigned char *app_name;
    const unsigned char *component_name;
    const unsigned char *app_end;
    const unsigned char *component_end;
    size_t program_length;
    size_t app_length;
    size_t component_length;

    if (buf == NULL || program == NULL || component == NULL || log_level == NULL ||
            component_capacity == 0 || length != sizeof(rdk_dynamic_log_message_t))
        return 0;
    if (memcmp(buf, RDK_DYNAMIC_LOG_SIGNATURE, RDK_DYNAMIC_LOG_SIGNATURE_LEN) != 0)
        return 0;

    program_length = strlen(program);
    app_name = buf + offsetof(rdk_dynamic_log_message_t, app_name);
    app_end = memchr(app_name, '\0', app_capacity);
    if (app_end == NULL)
        return 0;
    app_length = (size_t)(app_end - app_name);
    if (app_length == 0 || app_length != program_length ||
            memcmp(app_name, program, app_length) != 0)
        return 0;

    component_name = buf + offsetof(rdk_dynamic_log_message_t, component_name);
    component_end = memchr(component_name, '\0', component_field_capacity);
    if (component_end == NULL)
        return 0;
    component_length = (size_t)(component_end - component_name);
    if (component_length > RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX ||
            component_length >= component_capacity)
        return 0;

    memcpy(component, component_name, component_length + 1);
    *log_level = buf[offsetof(rdk_dynamic_log_message_t, log_level)];
    return 1;
}

#endif
