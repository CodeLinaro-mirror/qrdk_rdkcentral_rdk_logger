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
#include <string.h>
#define BUF_LEN 256
static int isLogInited = 0;

/**
 * @brief Initialize a log appender with specified configuration.
 *
 * Creates and configures a log appender with the specified type, layout, and file policy.
 * For file appenders, this function sets up rolling file policies with size-based rotation.
 * The appender is opened and ready for use after successful initialization.
 *
 * @param[in] app The appender type (RDK_LOG_OUTPUT_FILE, RDK_LOG_OUTPUT_STDOUT, etc.)
 * @param[in] layout The log layout format (RDK_LOG_LAYOUT_PLAINTEXT, RDK_LOG_LAYOUT_TIMESTAMPED, RDK_LOG_LAYOUT_COMCAST)
 * @param[in] pPolicy Pointer to file policy structure containing log directory, filename, max file size, and rotation count.
 *                    Required for RDK_LOG_OUTPUT_FILE, can be NULL for other appender types.
 *
 * @return RDK_SUCCESS on success, -1 on failure.
 *
 * @note This function must be called before rdk_logger_set_appender() to create the appender.
 * @note For file appenders, ensure the log directory exists and is writable.
 */
rdk_Error rdk_logger_appender_init(rdk_LogAppenderType app, rdk_LogLayout layout, rdk_LogFilePolicy *pPolicy)
{
    rdk_Error ret = rdk_dbg_priv_appender_init(app, layout, pPolicy);
    return ret;
}

/**
 * @brief Associate a log appender with a category.
 *
 * Links a previously created appender to a log category, enabling logging for that category
 * to use the specified appender. The appender must be created first using rdk_logger_appender_init().
 *
 * @param[in] pCategoryName The name of the log category (e.g., "LOG.RDK.TR69").
 *                          If NULL, defaults to "LOG.RDK".
 * @param[in] app The appender type that was used in rdk_logger_appender_init()
 * @param[in] pPolicy Pointer to the same file policy structure used in rdk_logger_appender_init().
 *                    Required for RDK_LOG_OUTPUT_FILE to locate the correct appender, can be NULL for other types.
 *
 * @return RDK_SUCCESS on success, -1 on failure.
 *
 * @note The appender must be initialized using rdk_logger_appender_init() before calling this function.
 * @note The same pPolicy parameter values must be used as in the corresponding rdk_logger_appender_init() call.
 */
rdk_Error rdk_logger_set_appender(const char* pCategoryName, rdk_LogAppenderType app, rdk_LogFilePolicy *pPolicy)
{
    rdk_Error ret = rdk_dbg_priv_set_appender(pCategoryName, app, pPolicy);

    return ret;
}
/**
 * @brief Set the log level for a category.
 * @param category_name The log category name (e.g., "LOG.RDK.TEST").
 * @param log_level The desired log level (e.g., RDK_LOG_DEBUG).
 * @return RDK_SUCCESS on success, -1 on failure.
 */
rdk_Error rdk_logger_set_loglevel(const char* category_name, rdk_LogLevel log_level)
{
    rdk_Error ret = rdk_dbg_priv_set_log_level(category_name, log_level);
    return ret;
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

/**
 * @brief Initialize RDK logger with extended configuration.
 *
 * This function provides a comprehensive logger initialization interface that combines
 * the standard RDK logger initialization with extended configuration options. It first
 * calls RDK_LOGGER_INIT() to perform basic logger setup, then applies the extended
 * configuration parameters for specific module logging requirements.
 *
 * @param[in] config Pointer to extended logger configuration structure containing:
 *                   - pCategoryName: Log category/module name (required, cannot be NULL)
 *                   - loglevel: Default log level for the category
 *                   - appender: Type of log appender (FILE, STDOUT, SYSLOG, SOCKET)
 *                   - layout: Message layout format (PLAINTEXT, TIMESTAMPED, COMCAST)
 *                   - pFilePolicy: File policy configuration (required for file appenders, NULL for others)
 *
 * @return RDK_SUCCESS on successful initialization, -1 on error.
 *
 * @note This function must be called after the basic RDK logger system is available.
 * @note If RDK_LOGGER_INIT() fails, the extended configuration is not applied.
 * @note For file appenders, ensure the log directory exists and has write permissions.
 * @note This function internally calls rdk_dbg_priv_ext_init() for the actual configuration.
 */
rdk_Error rdk_logger_ext_init(const rdk_logger_ext_config_t* config)
{
    rdk_Error ret;
    ret = RDK_LOGGER_INIT();
    if (ret == RDK_SUCCESS)
    {
        ret = rdk_dbg_priv_ext_init(config->pCategoryName, config->loglevel, config->appender, config->layout, config->pFilePolicy);
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
