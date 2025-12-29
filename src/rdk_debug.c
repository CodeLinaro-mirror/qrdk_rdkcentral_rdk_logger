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
 * @file rdk_debug.c
 * This source file contains the APIs for RDK debug.
 */

/**
* @defgroup rdk_logger
* @{
* @defgroup src
* @{
**/

#include <rdk_logger.h>
#include <rdk_debug_priv.h>
#include <string.h>
#include <stdarg.h>

/**
 * @brief Send a debugging message to the debugging window. It is appended to the log output based
 * on configurations set in the environment file.
 *
 * @param[in] level The debug logging level.
 * @param[in] module The name of the module for which this message belongs to, it is mentioned in debug.ini.
 * @param[in] format Printf style string containing the log message.
 */
void rdk_logger_msg_printf(rdk_LogLevel level, const char *module, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    rdk_dbg_priv_log_msg(level, module, format, args);
    va_end(args);
}

/**
 * @brief Send a debugging message to the debugging window. It is appended to the log output based
 * on configurations set in the environment file.
 *
 * @param[in] level The debug logging level.
 * @param[in] module The name of the module for which this message belongs to, it is mentioned in debug.ini.
 * @param[in] format Printf style string containing the log message.
 */
void rdk_dbg_MsgRaw(rdk_LogLevel level, const char *module, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    rdk_dbg_priv_log_msg(level, module, format, args);
    va_end(args);
}

void rdk_logger_msg_vsprintf(rdk_LogLevel level, const char *module, const char *format, va_list args)
{
    rdk_dbg_priv_log_msg(level, module, format, args);
}

void rdk_dbg_MsgRaw1(rdk_LogLevel level, const char *module, const char *format, va_list args)
{
    rdk_dbg_priv_log_msg(level, module, format, args);
}

/**
 * @brief Function to sets a specific log level of a module.
 *
 * @param[in] module The module name or category for for which the log level shall be checked
 * @param[in] level The debug logging level.
 *
 * @return Returns true, if debug log level enabled successfully else returns false.
 */
bool rdk_logger_set_logLevel(const char *module, rdk_LogLevel logLevel)
{
    return rdk_dbg_priv_log_reconfig(module, logLevel);
}

/**
 * @brief Enable or disable a log level for a module.
 * @param module Module name.
 * @param logLevel Log level.
 * @param enableLogLvl TRUE to enable, FALSE to disable.
 * @return TRUE if successful, FALSE otherwise.
 */
bool rdk_logger_enable_logLevel(const char *module, rdk_LogLevel logLevel, rdk_logger_Bool enableLogLvl)
{
    (void) enableLogLvl;
    return rdk_dbg_priv_log_reconfig(module, logLevel);
}

/**
 * @brief Convert a log level string to rdk_LogLevel enum.
 * @param level Log level as string (e.g., "INFO", "DEBUG").
 * @return Corresponding rdk_LogLevel value, or RDK_LOG_NONE if invalid.
 */
rdk_LogLevel rdk_logger_level_from_string(const char* level)
{
    if (!level) return RDK_LOG_NONE;
    if (strcasecmp(level, "FATAL") == 0)   return RDK_LOG_FATAL;
    if (strcasecmp(level, "ERROR") == 0)   return RDK_LOG_ERROR;
    if (strcasecmp(level, "WARNING") == 0) return RDK_LOG_WARN;
    if (strcasecmp(level, "WARN") == 0)    return RDK_LOG_WARN;
    if (strcasecmp(level, "NOTICE") == 0)  return RDK_LOG_NOTICE;
    if (strcasecmp(level, "INFO") == 0)    return RDK_LOG_INFO;
    if (strcasecmp(level, "DEBUG") == 0)   return RDK_LOG_DEBUG;
    if (strcasecmp(level, "TRACE") == 0)   return RDK_LOG_TRACE;
    if (strcasecmp(level, "NONE") == 0)    return RDK_LOG_NONE;
    return RDK_LOG_NONE;
}
