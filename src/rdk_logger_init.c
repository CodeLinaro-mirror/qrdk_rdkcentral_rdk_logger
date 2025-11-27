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
 * @file rdk_logger_init.c
 * This source file contains the APIs for RDK logger initializer.
 */

/**
* @defgroup rdk_logger
* @{
* @defgroup src
* @{
**/


#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rdk_logger.h"
#include "rdk_error.h"
#include "rdk_debug_priv.h"
#include "rdk_dynamic_logger.h"
#include "rdk_utils.h"

#define BUF_LEN 256
static int isLogInited = 0;

/**
 * @brief Set the default log level for a category.
 * @param category_name The log category name (e.g., "LOG.RDK").
 * @param log_level The desired log level (e.g., RDK_LOG_DEBUG).
 */
void rdk_logger_set_default_loglevel(const char* category_name, rdk_LogLevel log_level)
{
    set_default_log_level(category_name, log_level);
}

/**
 * @brief Set the layout for an appender.
 * @param appender_name The appender name (e.g., "stream_env", "rollingfile").
 * @param layout The desired layout (e.g., LAYOUT_DATED).
 */
void rdk_logger_set_default_layout(const char* appender_name, rdk_LogLayout layout)
{
    log4c_appender_t* app = log4c_appender_get(appender_name);
    if(!app)
        app = log4c_appender_new(appender_name);
    if (app)
    {
        set_default_layout(app, layout);
    }
}

/**
 * @brief Set the appender type for an appender.
 * @param appender_name The appender name (e.g., "stream_env", "rollingfile").
 * @param appender_type The desired appender type (e.g., StdOut, FileOutput).
 */
void rdk_logger_set_default_appender_type(const char* category_name, const char* logdir, const char* fileName, rdk_LogAppenderType appender_type)
{
    log4c_category_t* cat = log4c_category_get(category_name);
    char fullpath[512];
    if (!cat) 
        cat = log4c_category_new(category_name);

    if (appender_type == RDK_LOG_OUTPUT_FILE)
    {
        if (!logdir || !fileName)
        {
            fprintf(stderr, "Error: logdir and log_file_name required for FileOutput\n");
            return;
        }
        snprintf(fullpath, sizeof(fullpath), "%s/%s", logdir, fileName);
    }
    else
    {
        strncpy(fullpath, "stdout", sizeof(fullpath)-1);
        fullpath[sizeof(fullpath)-1] = '\0';
    }
    log4c_appender_t* app = log4c_appender_get(fullpath);
    if(!app)
        app = log4c_appender_new(fullpath);
    if (app)
    {
        set_default_appender_type(cat, logdir, fileName, app, appender_type);
    }
}

/**
 * @brief Initialize the logger. Sets up the environment variable storage by parsing
 * debug configuration file then Initialize the debug support to the underlying platform.
 *
 * @note  Requests not to send SIGPIPE on errors on stream oriented sockets
 * when the other end breaks the connection. The EPIPE error is still returned.
 *
 * @param[in] debugConfigFile The character pointer variable of debug configuration file.
 *
 * @return Returns 0 if initialization of RDK logger module is successful, else it returns -1.
 */
rdk_Error rdk_logger_init(const char* debugConfigFile)
{
    rdk_Error ret;

    if (0 == isLogInited)
    {
        if (NULL == debugConfigFile)
        {
            debugConfigFile = DEBUG_CONF_FILE;
        }

        /* Read the config file & populate pre-configured log levels */
        ret = rdk_logger_parse_config(debugConfigFile);
        if ( RDK_SUCCESS != ret)
        {
            printf("%s:%d Adding debug config file %s failed\n", __FUNCTION__, __LINE__, debugConfigFile);
            return ret;
        }
        /* Perform Logger Internal Init */
        rdk_dbg_init();

        /* Perform Dynamin Logger Internal Init */
        rdk_dyn_log_init();

        /**
         * Requests not to send SIGPIPE on errors on stream oriented
         * sockets when the other end breaks the connection. The EPIPE
         * error is still returned.
         */
        signal(SIGPIPE, SIG_IGN);
        isLogInited = 1;
    }
    return RDK_SUCCESS;
}

rdk_Error rdk_logger_ext_init(const rdk_logger_ext_config_t* config)
{
    rdk_Error ret;
    ret = RDK_LOGGER_INIT();
    if (ret == RDK_SUCCESS)
    {
        rdk_dbg_priv_ext_init(config->moduleName, config->logdir, config->fileName, config->maxRotationCount, config->maxBytesPerFile, config->appender_type, config->loglevel, config->layout);
    }
    return ret;
 }

/**
 * @brief Cleanup the logger instantiation.
 *
 * @return Returns 0 if the call is successful else return -1.
 */
rdk_Error rdk_logger_deinit()
{
    if(isLogInited)
    {
        rdk_dyn_log_deinit();
        rdk_logger_release_config();
        //isLogInited = 0;
    }

    return RDK_SUCCESS;
}
