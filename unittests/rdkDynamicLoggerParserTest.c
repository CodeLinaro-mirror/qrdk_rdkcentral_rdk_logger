#include "rdk_dynamic_logger_parser.h"

#include <stdio.h>
#include <string.h>

static void make_request(rdk_dynamic_log_message_t *message, const char *app_name, size_t component_length)
{
    memset(message, 0, sizeof(*message));
    memcpy(message->signature, RDK_DYNAMIC_LOG_SIGNATURE, RDK_DYNAMIC_LOG_SIGNATURE_LEN);
    message->log_level = 4;
    snprintf(message->app_name, sizeof(message->app_name), "%s", app_name);
    memset(message->component_name, 'x', component_length);
    message->component_name[component_length] = '\0';
}

int main(void)
{
    rdk_dynamic_log_message_t request;
    unsigned char level = 0;
    char component[RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX + 1];
    size_t length = sizeof(request);

    make_request(&request, "app", RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX);
    if (!rdk_dyn_log_parse_request((const unsigned char *)&request, length, "app", component, sizeof(component), &level))
        return 1;
    if (strlen(component) != RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX || level != 4)
        return 1;

    component[0] = 'q';
    if (rdk_dyn_log_parse_request((const unsigned char *)&request, length - 1, "app", component, sizeof(component), &level) || component[0] != 'q')
        return 1;
    if (rdk_dyn_log_parse_request((const unsigned char *)&request, length + 1, "app", component, sizeof(component), &level))
        return 1;
    if (rdk_dyn_log_parse_request((const unsigned char *)&request, length, "other", component, sizeof(component), &level))
        return 1;

    request.app_name[0] = '\0';
    if (rdk_dyn_log_parse_request((const unsigned char *)&request, length, "", component, sizeof(component), &level))
        return 1;
    make_request(&request, "app", RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX);
    memset(request.app_name, 'a', sizeof(request.app_name));
    if (rdk_dyn_log_parse_request((const unsigned char *)&request, length, "app", component, sizeof(component), &level))
        return 1;

    make_request(&request, "app", RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX);
    if (rdk_dyn_log_parse_request((const unsigned char *)&request, length, "app", component, sizeof(component) - 1, &level))
        return 1;

    request.component_name[RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX] = 'x';
    if (rdk_dyn_log_parse_request((const unsigned char *)&request, length, "app", component, sizeof(component), &level))
        return 1;

    make_request(&request, "app", 1);
    request.signature[0] = 'X';
    if (rdk_dyn_log_parse_request((const unsigned char *)&request, length, "app", component, sizeof(component), &level))
        return 1;
    if (rdk_dyn_log_parse_request(NULL, length, "app", component, sizeof(component), &level))
        return 1;

    return 0;
}
