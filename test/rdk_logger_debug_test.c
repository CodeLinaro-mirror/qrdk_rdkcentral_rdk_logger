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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rdk_logger.h"
int rdk_logger_debug_test()
{
    char buffer[55]; // Assume this is filled with 55 bytes of data
    for(int j=0; j<55; j++) buffer[j] = 'A' + (j % 26);

    for (int i = 0; i < 5; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO,  "LOG.RDK.AA", "Pre %s info %d\n", buffer, i);
        rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.AA", "Pre error %d\n", i);
        rdk_logger_msg_printf(RDK_LOG_INFO,  "LOG.RDK.BB", "Pre info  %d\n", i);
        rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.BB", "Pre error %d\n", i);
        rdk_logger_msg_printf(RDK_LOG_INFO,  "LOG.RDK.CC", "Pre info  %d\n", i);
        rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.CC", "Pre error %d\n", i);
    }

    printf ("2nd loop\n");
    rdk_logger_ext_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.pModuleName = "LOG.RDK.BB";
    cfg.loglevel = RDK_LOG_TRACE;
    cfg.output = RDKLOG_OUTPUT_SYSLOG;
    cfg.format = RDKLOG_FORMAT_PLAINTEXT;
    cfg.pFilePolicy = NULL;
    rdk_Error ret = rdk_logger_ext_init(&cfg);

    cfg.pModuleName = "LOG.RDK.CC";
    cfg.format = RDKLOG_FORMAT_DETAIL_WITH_TS;
    ret = rdk_logger_ext_init(&cfg);

    for (int i = 0; i < 5; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO,  "LOG.RDK.AA", "Post %s info %d\n", buffer, i);
        rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.AA", "Post error %d\n", i);
        rdk_logger_msg_printf(RDK_LOG_INFO,  "LOG.RDK.BB", "Post info  %d\n", i);
        rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.BB", "Post error %d\n", i);
        rdk_logger_msg_printf(RDK_LOG_INFO,  "LOG.RDK.CC", "Post info  %d\n", i);
        rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.CC", "Post error %d\n", i);
    }
    return 0;
}
