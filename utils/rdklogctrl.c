#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "rdk_dynamic_logger.h"

#define COMP_SIGNATURE "LOG.RDK."
#define COMP_SIGNATURE_LEN 8

static void usage(const char* app_name)
{
    printf("usage: %s <pid> <module_name> <loglevel> \n",app_name);
    printf("Parameters:\n");
    printf("pid         -> Process ID of the application to update\n");
    printf("module_name -> Module name.\n");
    printf("               For RDK component, the 'Module name' is expected to start with 'LOG.RDK.' string\n");
    printf("               For CPC component like 'Receiver', the module name can be either 'LOG.RDK.' or 'XREConnection', 'RmfMediaPlayer', etc.\n");
    printf("loglevel    -> Log Level of the Component to be modified\n");
    printf("               Possible values - FATAL, ERROR, WARN, NOTICE, INFO, DEBUG, TRACE, NONE\n");
}

static int process_name_for_pid(pid_t pid, char *process_name, size_t process_name_size)
{
    char proc_path[64];
    char command_line[PATH_MAX + 1];
    char *argument_end;
    char *base_name;
    FILE *proc_file;
    size_t bytes_read;
    size_t name_length;

    if (snprintf(proc_path, sizeof(proc_path), "/proc/%ld/cmdline", (long)pid) >= (int)sizeof(proc_path))
        return -1;

    proc_file = fopen(proc_path, "rb");
    if (proc_file == NULL)
        return -1;

    bytes_read = fread(command_line, 1, sizeof(command_line) - 1, proc_file);
    fclose(proc_file);
    argument_end = memchr(command_line, '\0', bytes_read);
    if (argument_end == NULL || argument_end == command_line)
        return -1;

    *argument_end = '\0';
    base_name = strrchr(command_line, '/');
    base_name = base_name == NULL ? command_line : base_name + 1;
    name_length = strlen(base_name);
    if (name_length == 0 || name_length >= process_name_size ||
            name_length > RDK_DYNAMIC_LOG_APP_NAME_MAX)
        return -1;

    memcpy(process_name, base_name, name_length + 1);
    return 0;
}

static int8_t validate_loglevel(const char* level)
{
    char *loglevel = (char *)level;

    if (!level)
        return -1;

    if(0 == strncmp(loglevel, "FATAL", 5))
        return RDK_LOG_FATAL;
    else if(0 == strncmp(loglevel, "ERROR", 5))
        return RDK_LOG_ERROR;
    else if(0 == strncmp(loglevel, "WARN", 4))
        return RDK_LOG_WARN;
    else if(0 == strncmp(loglevel, "NOTICE", 6))
        return RDK_LOG_NOTICE;
    else if(0 == strncmp(loglevel, "INFO", 4))
        return RDK_LOG_INFO;
    else if(0 == strncmp(loglevel, "DEBUG", 5))
        return RDK_LOG_DEBUG;
    else if(0 == strncmp(loglevel, "TRACE", 6))
        return RDK_LOG_TRACE;
    else if(0 == strncmp(loglevel, "NONE", 4))
        return RDK_LOG_NONE;
    else
        return -1;
}

int main(int argc, char *argv[])
{
    struct sockaddr_un destAddr;
    int sockfd, numbytes;
    long parsed_pid;
    char *pid_end;
    const char *pid_character;
    pid_t target_pid;
    char process_name[RDK_DYNAMIC_LOG_APP_NAME_MAX + 1];
    size_t app_len, comp_len;
    int8_t level = -1;
    rdk_dynamic_log_message_t message = {0};

    if (argc != 4) {
        usage(argv[0]);
        return -1;
    }

    for (pid_character = argv[1]; *pid_character != '\0'; ++pid_character) {
        if (*pid_character < '0' || *pid_character > '9') {
            fprintf(stderr, "Invalid process ID\n");
            return -1;
        }
    }

    errno = 0;
    parsed_pid = strtol(argv[1], &pid_end, 10);
    if (errno != 0 || pid_end == argv[1] || *pid_end != '\0' ||
            parsed_pid <= 0 || parsed_pid > INT_MAX) {
        fprintf(stderr, "Invalid process ID\n");
        return -1;
    }

    target_pid = (pid_t)parsed_pid;

    comp_len = strlen(argv[2]);
    if (comp_len == 0 || comp_len > RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX) {
        fprintf(stderr, "App or module name exceeds packet field size\n");
        return -1;
    }

    if( (0 != strncmp(argv[2], COMP_SIGNATURE, COMP_SIGNATURE_LEN)))
    {
        printf("Invalid module name\n");
        usage(argv[0]);
        return -1;
    }

    level = validate_loglevel(argv[3]);
    if(-1 == level) {
        printf("Invalid log level\n");
        usage(argv[0]);
        return -1;
    }

    if (process_name_for_pid(target_pid, process_name, sizeof(process_name)) != 0) {
        fprintf(stderr, "Unable to resolve process ID %ld\n", parsed_pid);
        return -1;
    }

    app_len = strlen(process_name);
    if (app_len == 0 || app_len > RDK_DYNAMIC_LOG_APP_NAME_MAX) {
        fprintf(stderr, "App or module name exceeds packet field size\n");
        return -1;
    }


    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        printf("socket: %s\n",strerror(errno));
        return -1;
    }

    memset(&destAddr, 0,sizeof(destAddr));
    destAddr.sun_family = AF_UNIX;
    int pathLength = 1 + snprintf(destAddr.sun_path, 0, "%s_%s.%ld", RDK_DYNAMIC_SOCKET_PATH_PREFIX, process_name, (long)parsed_pid);

    if (pathLength > sizeof(destAddr.sun_path))
    {
        printf ("long process name that cannot be communicated\n");
        return -1;
    }

    snprintf(destAddr.sun_path, sizeof(destAddr.sun_path), "%s_%s.%ld", RDK_DYNAMIC_SOCKET_PATH_PREFIX, process_name, (long)parsed_pid);

    snprintf(message.signature, RDK_DYNAMIC_LOG_SIGNATURE_LEN, "%s", RDK_DYNAMIC_LOG_SIGNATURE);
    message.log_level = (uint8_t)level;
    snprintf(message.app_name, sizeof(message.app_name), "%s", process_name);
    snprintf(message.component_name, sizeof(message.component_name), "%s", argv[2]);

    if ((numbytes = sendto(sockfd, (const void*) &message, sizeof(message), MSG_DONTWAIT, (struct sockaddr *)&destAddr, sizeof(destAddr))) == -1)
    {
        printf("sendto: %s\n",strerror(errno));
        close(sockfd);
        return -1;
    }

    if ((size_t)numbytes != sizeof(message))
    {
        fprintf(stderr, "sendto sent an incomplete dynamic log packet\n");
        close(sockfd);
        return -1;
    }

    printf("Sent message to update log level of %s for PID %ld (%s)\n", argv[2], parsed_pid, process_name);
    close(sockfd);
    return 0;
}
