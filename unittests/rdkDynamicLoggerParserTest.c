#include "rdk_dynamic_logger_parser.h"

#include <string.h>

static size_t make_request(unsigned char *buffer, size_t capacity, size_t component_length)
{
    const char program[] = "app";
    size_t length = 7 + sizeof(program) - 1 + 1 + component_length;
    if (length > capacity)
        return 0;
    memset(buffer, 0, capacity);
    memcpy(buffer, "COMC", 4);
    buffer[4] = (unsigned char)(length - 5);
    buffer[5] = 4;
    buffer[6] = sizeof(program) - 1;
    memcpy(buffer + 7, program, sizeof(program) - 1);
    buffer[10] = (unsigned char)component_length;
    memset(buffer + 11, 'x', component_length);
    return length;
}

int main(void)
{
    unsigned char request[128];
    unsigned char level = 0;
    char component[64];
    size_t length = make_request(request, sizeof(request), 63);

    if (!rdk_dyn_log_parse_request(request, length, "app", component, sizeof(component), &level))
        return 1;
    if (strlen(component) != 63 || level != 4)
        return 1;
    if (rdk_dyn_log_parse_request(request, length - 1, "app", component, sizeof(component), &level))
        return 1;

    component[0] = 'q';
    request[4]--;
    if (rdk_dyn_log_parse_request(request, length, "app", component, sizeof(component), &level) || component[0] != 'q')
        return 1;
    request[4]++;
    if (rdk_dyn_log_parse_request(request, length + 1, "app", component, sizeof(component), &level))
        return 1;
    if (rdk_dyn_log_parse_request(request, length, "app", component, 1, &level))
        return 1;
    if (rdk_dyn_log_parse_request(NULL, length, "app", component, sizeof(component), &level))
        return 1;

    length = make_request(request, sizeof(request), 64);
    if (rdk_dyn_log_parse_request(request, length, "app", component, sizeof(component), &level))
        return 1;
    if (rdk_dyn_log_parse_request(request, length, "other", component, sizeof(component), &level))
        return 1;

    request[6] = 127;
    if (rdk_dyn_log_parse_request(request, length, "app", component, sizeof(component), &level))
        return 1;
    return 0;
}
