/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @file rdk_dynamic_logger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "rdk_dynamic_logger.h"
#include "rdk_dynamic_logger_parser.h"
#include "rdk_debug_priv.h"


static int g_dl_socket = -1;
static pid_t g_dl_socket_pid = -1;
static pthread_mutex_t g_dl_request_mutex = PTHREAD_MUTEX_INITIALIZER;
extern char *__progname;

static char * rdk_dyn_log_logLevelToString(rdk_LogLevel log_level)
{
    switch(log_level){
        case RDK_LOG_FATAL:  return "FATAL";
        case RDK_LOG_ERROR:  return "ERROR";
        case RDK_LOG_WARN:   return "WARNING";
        case RDK_LOG_NOTICE: return "NOTICE";
        case RDK_LOG_INFO:   return "INFO";
        case RDK_LOG_DEBUG:  return "DEBUG";
        case RDK_LOG_TRACE:  return "TRACE";
        case RDK_LOG_NONE:   return "NONE";
    }
    return "NONE";
}

static void rdk_dyn_log_validate_component_name(const unsigned char *buf, size_t length)
{
    unsigned char log_level = 0;
    char comp_name[64] = {0};
    rdk_LogLevel loggingLevel;

    if (!rdk_dyn_log_parse_request(buf, length, __progname, comp_name, sizeof(comp_name), &log_level))
        return;
static void rdk_dyn_log_validate_component_name(const rdk_dynamic_log_message_t *message)
{
    if (0 != memcmp(message->signature, RDK_DYNAMIC_LOG_SIGNATURE, RDK_DYNAMIC_LOG_SIGNATURE_LEN)) {
        return;
    }

    if (__progname == NULL || 0 != strcmp(message->app_name, __progname)) {
        /* The received msg is not intended for this process */
        return;
    }

    rdk_LogLevel loggingLevel = (rdk_LogLevel) message->log_level;

    loggingLevel = (rdk_LogLevel) log_level;
    if((loggingLevel >= RDK_LOG_FATAL) && (loggingLevel <= RDK_LOG_NONE))
    {
        rdk_dbg_priv_log_reconfig(comp_name, loggingLevel);
        fprintf(stderr, "Log level change request to %s (%u) for the component %s, is success\n", rdk_dyn_log_logLevelToString(loggingLevel), loggingLevel, comp_name);
        rdk_dbg_priv_set_level(message->component_name, loggingLevel);
        fprintf(stderr, "Log level change request to %s (%u) for the component %s, is success\n", rdk_dyn_log_logLevelToString(loggingLevel), loggingLevel, message->component_name);
    }
    else
    {
        fprintf(stderr, "Log level change request with Invalid input (%u)\n", loggingLevel);
    }
}

void rdk_dyn_log_process_pending_request()
{
    char buf[128] = {0};
    struct timeval tv;
    int numbytes, ret;
    rdk_dynamic_log_message_t message = {0};
    struct sockaddr_un sender_addr;
    struct timeval tv;
    int numbytes, ret;
    size_t app_len, comp_len, comp_len_offset;
    socklen_t addr_len;
    fd_set rfds;

    if (pthread_mutex_trylock(&g_dl_request_mutex) != 0)
        return;

    /* After fork/daemonize the inherited socket is named after the parent PID; rebind for ours. */
    if (g_dl_socket != -1 && g_dl_socket_pid != getpid()) {
        close(g_dl_socket);
        g_dl_socket = -1;
        pthread_mutex_unlock(&g_dl_request_mutex);
        rdk_dyn_log_init();
        return;
    }

    if(-1 == g_dl_socket) {
        pthread_mutex_unlock(&g_dl_request_mutex);
        return;
    }

    while(1) {
        FD_ZERO(&rfds);
        FD_SET(g_dl_socket, &rfds);

        tv.tv_sec = 0;
        tv.tv_usec = 0;
        ret = select(g_dl_socket+1,&rfds,NULL,NULL,&tv);
        if(ret <= 0)
            break;

        if ((numbytes=recvfrom(g_dl_socket, buf, sizeof(buf), MSG_TRUNC, NULL, NULL)) == -1) {
        numbytes = recvfrom(g_dl_socket, &message, sizeof(message), MSG_DONTWAIT | MSG_TRUNC, (struct sockaddr *)&sender_addr, &addr_len);
        if (numbytes == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            fprintf(stderr,"%s recvfrom failed %s\n",__func__,strerror(errno));
            pthread_mutex_unlock(&g_dl_request_mutex);
            return;
        }

        /*
         * msg_format {
         *    char    m_signature[4];
         *    uchar8  m_msgLength;
         *    uchar8  m_logLevel;
         *    uchar8  m_appLength;
         *    char    m_appname[m_appLength];
         *    uchar8  m_compNameLength;
         *    char    m_compName[m_compNameLength];
         * }
         *
         * Ensure that the we handle msgs only from localhost
         */
        if((numbytes >= DL_SIGNATURE_LEN + 1) &&
                ((size_t)numbytes <= sizeof(buf)) &&
                ((size_t)numbytes == (size_t)(unsigned char)buf[4] + DL_SIGNATURE_LEN + 1)) {
            rdk_dyn_log_validate_component_name((const unsigned char *)buf, (size_t)numbytes);
        if ((numbytes != (int)sizeof(message))  ||
            (memchr(message.app_name, '\0', sizeof(message.app_name)) == NULL) ||
            (memchr(message.component_name, '\0', sizeof(message.component_name)) == NULL))
            continue;

        if (0 == memcmp(message.signature, RDK_DYNAMIC_LOG_SIGNATURE, RDK_DYNAMIC_LOG_SIGNATURE_LEN))
        {
            rdk_dyn_log_validate_component_name(&message);
        }
    }
    pthread_mutex_unlock(&g_dl_request_mutex);
}

void rdk_dyn_log_init()
{
    struct sockaddr_un myAddr;
    socklen_t addrLen = sizeof (struct sockaddr_un);
    int bind_result = -1;

    memset(&myAddr, 0, sizeof(myAddr));

    if (__progname == NULL)
        return;

    pthread_mutex_lock(&g_dl_request_mutex);

    if ((g_dl_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
        fprintf(stderr, "%s() Socket creation failed. %s\n",__func__,strerror(errno));
        pthread_mutex_unlock(&g_dl_request_mutex);
        return;
    }

    myAddr.sun_family = AF_UNIX;

    // The path is, "/tmp/.logger_<progname>.<pid>"
    int pathLength = 1 + snprintf(myAddr.sun_path, 0, "%s_%s.%ld", RDK_DYNAMIC_SOCKET_PATH_PREFIX, __progname, (long)getpid());
    if (pathLength > sizeof(myAddr.sun_path))
    {
        fprintf (stderr, "long process name that cannot be communicated\n");
        close(g_dl_socket);
        g_dl_socket = -1;
        pthread_mutex_unlock(&g_dl_request_mutex);
        return;
    }

    snprintf(myAddr.sun_path, sizeof(myAddr.sun_path), "%s_%s.%ld", RDK_DYNAMIC_SOCKET_PATH_PREFIX, __progname, (long)getpid());

    bind_result = bind(g_dl_socket, (struct sockaddr *)&myAddr, sizeof(myAddr));
    if (-1 == bind_result)
    {
        fprintf(stderr, "%s() bind error %s\n",__func__,strerror(errno));
        close(g_dl_socket);
        g_dl_socket = -1;
        pthread_mutex_unlock(&g_dl_request_mutex);
        return;
    }

    g_dl_socket_pid = getpid();
    pthread_mutex_unlock(&g_dl_request_mutex);
    return;
}

void rdk_dyn_log_deinit()
{
    pthread_mutex_lock(&g_dl_request_mutex);
    if (g_dl_socket != -1)
    {
        close(g_dl_socket);
        g_dl_socket = -1;
    }
    pthread_mutex_unlock(&g_dl_request_mutex);
}
