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
 * @file rdk_logger.h
 * The header file provides RDK Logger APIs.
 */

/**
 * @defgroup RDKLOGGER RDK Logger
 * RDK Logger provides common logging capability for all RDK components.
 * It is a common logging library which is based on MPEOS logging & uses log4c for
 * formatting and supports multiple log levels.
 *
 * @par RDK Logger Capabilities
 *
 * - Abstracts logging client from underlying logging utility.
 * - Dynamically enables/disables logging level at run time.
 * - Provides logging format that complies with the existing OCAP format (e.g. <timestamp> [mod=*, level=*]).
 * - Controls log level independently for each component/module.
 * - Enables logging globally via single configuration value.
 * - Controls initial log level for each component/module from configuration file (debug.ini) at startup.
 * - The debug.ini is the main configuration file for RDK Logger and it is intended to be used
 * globally in multi-process environment.
 * - Prints formatted data to stdout.
 *
 * @par How to add the RDK logger functionality to the new module?
 * @n Include rdk_logger.h header file and make use of RDK_LOG for printing logs. Initialize RDK Logger by
 * calling rdk_logger_init() in the respective module/component. Build new module/component by linking
 * "librdkloggers.so" along with "liblog4c.so" and "libglib-2.0.so" shared object.
 * @code
 * Example: -lrdkloggers -L ../opensource/lib -llog4c -lglib-2.0
 * @endcode
 *
 * @par RDK Logger Usage
 * @code
 * RDK_LOG (rdk_LogLevel level, const char *module, const char *format,...)
 * @endcode
 * - "level" is Log level of the log message (FATAL, ERROR etc). Apart from this there are
 *    special log levels like ALL, NONE, TRACE, !TRACE are supported.
 * - "module" is the Module to which this message belongs to (Use module name same as mentioned in debug.ini)
 * - "format" is a printf style string containing the log message.
 *
 * @par Sample code for Logging:
 * For example, add a debug messages for "SI" module
 * @code
 * RDK_LOG (RDK_LOG_NOTICE, "LOG.RDK.SI","<%s: %s>: Sending PMT_ACQUIRED event\n", PSIMODULE, __FUNCTION__);
 * @endcode
 *
 * @par Sample logging output:
 * @code
 * 131011-21:21:49.578394 [mod=INBSI, lvl=NOTICE] [tid=4141] <SITP_PSI: NotifyTableChanged>: Sending PMT_ACQUIRED event
 * @endcode
 *
 * In this way, user make use of the RDK logger in the respective modules and control the logging levels through configuration file.
 * Here, No need to build RDK logger again for the addition of new components/module.
 * @ingroup RDKLOGGER
 *
 * @par RDK Logging architecture
 *
 * @image html rdk_logger_architecture.jpg
 *
 * @par Logging Levels supported by RDK Logger.
 * Code | Description
 * -----|------------
 * RDK_LOG_FATAL    | Any error that is forcing a shutdown of the service or application to prevent data loss (or further data loss), reserve these only for the most heinous errors and situations where there is guaranteed to have been data corruption or loss.
 * RDK_LOG_ERROR    | Any error which is fatal to the operation but not the service (cant open a file, missing data, etc)
 * RDK_LOG_WARN     | Anything that can potentially cause application oddities, but for which the application automatically recoverring.
 * RDK_LOG_NOTICE   | Anything that largely superfluous for application-level logging.
 * RDK_LOG_INFO     | Generally useful information to log (service start/stop, configuration assumptions, etc).
 * RDK_LOG_DEBUG    | Information that is diagnostically helpful to people more than just developers.
 * RDK_LOG_TRACE    | Only when it would be "tracing" the code and trying to find one part of a function specifically.
 *
 * @par How log files are upload to server
 * @image html rdk_logupload.jpg
 *
 * @defgroup RDKLOGGER_LOGGER_API RDK Logger APIs
 * Describe the details about RDK Logger APIs specifications.
 * @ingroup RDKLOGGER
 *
 * @defgroup RDKLOGGER_UTILS_API RDK Logger Utils APIs
 * Describe the details about RDK Logger utils API specifications.
 * @ingroup RDKLOGGER
 */

/**
* @defgroup rdk_logger
* @{
* @defgroup include
* @{
**/


#ifndef _RDK_LOGGER_H_
#define _RDK_LOGGER_H_

#include <stdio.h>
#include "rdk_logger_types.h"
#include "rdk_error.h"
#ifdef __cplusplus
extern "C"
{
#endif


/**
* Macros for ease of applications using RDK_LOGGER apis
*/

/**
 * Define the default location of configuration file location
 */
#define DEBUG_INI_NAME "/etc/debug.ini"

/**
 * Support for overriding debug.ini file location
 */
#ifndef DEBUG_INI_OVERRIDE_PATH
#define DEBUG_INI_OVERRIDE_PATH "/nvram/debug.ini"
#endif

/**
 * Support for Init function
 */
#define RDK_LOGGER_INIT()   (0 == access(DEBUG_INI_OVERRIDE_PATH, F_OK)) \
                                ? rdk_logger_init(DEBUG_INI_OVERRIDE_PATH) \
                                : rdk_logger_init(DEBUG_INI_NAME);
/**
 * Use RDK_LOG debug message as.
 * RDK_LOG (rdk_LogLevel level, const char *module, const char *format,...)
 * @param level Log level of the log message
 * @param module Module in which this message belongs to (Use module name same as mentioned in debug.ini)
 * @param format Printf style string containing the log message.
 */
#define RDK_LOG rdk_logger_msg_printf
#define RDK_LOG1 rdk_logger_msg_vsprintf
/**
 * Define the max length for the log file capture
 */
#define RDK_LOGGER_EXT_FILENAME_SIZE 32

/**
 * Define the max length for the log capture path
 */
#define RDK_LOGGER_EXT_LOGDIR_SIZE   32

/**
 * To allow compatibility of mutiple legacy RDK components using loglevels RDK_LOG_TRACE1..RDK_LOG_TRACE9
 */
#define RDK_LOG_TRACE1 RDK_LOG_TRACE
#define RDK_LOG_TRACE2 RDK_LOG_TRACE
#define RDK_LOG_TRACE3 RDK_LOG_TRACE
#define RDK_LOG_TRACE4 RDK_LOG_TRACE
#define RDK_LOG_TRACE5 RDK_LOG_TRACE
#define RDK_LOG_TRACE6 RDK_LOG_TRACE
#define RDK_LOG_TRACE7 RDK_LOG_TRACE
#define RDK_LOG_TRACE8 RDK_LOG_TRACE
#define RDK_LOG_TRACE9 RDK_LOG_TRACE

/**
 * An access function macro to check logging is enabled or not
 */
#define rdk_dbg_enabled rdk_logger_is_logLevel_enabled

/**
 * To allow compatibility of mutiple legacy RDK components using onboard API 
 */
#define rdk_log_onboard rdk_logger_log_onboard

/**
 * @enum rdk_LogLevel
 * @brief These values represent the logging 'levels' or 'types', they are each
 * independent.
 */
typedef enum
{
    RDK_LOG_FATAL = 0,
    RDK_LOG_ERROR,
    RDK_LOG_WARN,
    RDK_LOG_NOTICE,
    RDK_LOG_INFO,
    RDK_LOG_DEBUG,
    RDK_LOG_TRACE,
    RDK_LOG_NONE
} rdk_LogLevel;

/**
 * @enum rdk_LogAppenderType
 * @brief Defines the destination type where log events are written.
 *
 * - RDK_LOG_OUTPUT_STDOUT:     Write logs to the process stdout/stderr stream (typically mapped via stream_env).
 * - RDK_LOG_OUTPUT_FILE: Write logs to rolling files on disk (uses rollingfile appender).
 * - RDK_LOG_OUTPUT_SOCKET: Write logs to socket (uses socket appender).
 * - RDK_LOG_OUTPUT_SYSLOG:     Forward logs to the local syslog daemon.
 */
typedef enum
{
    RDK_LOG_OUTPUT_STDOUT = 0,
    RDK_LOG_OUTPUT_FILE,
    RDK_LOG_OUTPUT_SOCKET,
    RDK_LOG_OUTPUT_SYSLOG
} rdk_LogAppenderType;

/**
 * @enum rdk_LogLayout
 * @brief Defines the layout/format used to render log messages.
 *
 * - RDK_LOG_LAYOUT_PLAINTEXT:          Simple layout (priority, category, message).
 * - RDK_LOG_LAYOUT_TIMESTAMPED:          Timestamped layout including date/time and milliseconds.
 * - RDK_LOG_LAYOUT_COMCAST:  Comcast-specific dated layout with module, level and thread id info.
 */
typedef enum
{
    RDK_LOG_LAYOUT_PLAINTEXT = 0,
    RDK_LOG_LAYOUT_TIMESTAMPED,
    RDK_LOG_LAYOUT_COMCAST
} rdk_LogLayout;

typedef struct rdk_logger_ext_config_t
 {
     char moduleName[32];
     char fileName[RDK_LOGGER_EXT_FILENAME_SIZE];
     char logdir[RDK_LOGGER_EXT_LOGDIR_SIZE];
     long maxBytesPerFile;
     long maxRotationCount;
     rdk_LogAppenderType appender_type;
     rdk_LogLevel loglevel;
     rdk_LogLayout layout;
 }rdk_logger_ext_config_t;

/**
 * @brief Set the default log level for a category.
 * @param category_name The log category name (e.g., "LOG.RDK").
 * @param log_level The desired log level (e.g., RDK_LOG_DEBUG).
 */
void rdk_logger_set_default_loglevel(const char* category_name, rdk_LogLevel log_level);

/**
 * @brief Set the layout for an appender.
 * @param appender_name The appender name (e.g., "stream_env", "rollingfile").
 * @param layout The desired layout (e.g., RDK_LOG_LAYOUT_PLAINTEXT, RDK_LOG_LAYOUT_TIMESTAMPED, RDK_LOG_LAYOUT_COMCAST).
 */
void rdk_logger_set_default_layout(const char* appender_name, rdk_LogLayout layout);

/**
 * @brief Set the appender type for an appender.
 * @param appender_name The appender name (e.g., "stream_env", "rollingfile").
 * @param appender_type The desired appender type (e.g., FileOutput, StdOut).
 */
void rdk_logger_set_default_appender_type(const char* category_name, const char* logdir, const char* fileName, rdk_LogAppenderType appender_type, long maxRotationCount, long maxBytesPerFile);

/**
 * @brief Initialize the RDK Logger.
 * @param debugConfigFile Path to the debug.ini configuration file.
 * @return RDK_SUCCESS on success, error code otherwise.
 */
rdk_Error rdk_logger_init(const char* debugConfigFile);

/**
 * @brief Extended initialization for RDK Logger.
 * @param config Pointer to rdk_logger_ext_config_t structure.
 * @return RDK_SUCCESS on success, error code otherwise.
 */
rdk_Error rdk_logger_ext_init(const rdk_logger_ext_config_t* config);

/**
 * @brief Deinitialize the RDK Logger.
 * @return RDK_SUCCESS on success, error code otherwise.
 */
rdk_Error rdk_logger_deinit(void);

/**
 * @brief Log a message with printf-style formatting.
 * @param level Log level.
 * @param module Module name.
 * @param format Printf-style format string.
 */
void rdk_logger_msg_printf(rdk_LogLevel level, const char *module, const char *format, ...) __attribute__ ((format (printf, 3, 4)));

/**
 * @brief Log a message using a va_list.
 * @param level Log level.
 * @param module Module name.
 * @param format Printf-style format string.
 * @param args va_list of arguments.
 */
void rdk_logger_msg_vsprintf(rdk_LogLevel level, const char *module, const char *format, va_list args);

/**
 * @brief Check if a log level is enabled for a module.
 * @param module Module name.
 * @param level Log level.
 * @return TRUE if enabled, FALSE otherwise.
 */
rdk_logger_Bool rdk_logger_is_logLevel_enabled(const char *module, rdk_LogLevel level);

/**
 * @brief Enable or disable a log level for a module.
 * @param module Module name.
 * @param logLevel Log level.
 * @param enableLogLvl TRUE to enable, FALSE to disable.
 * @return TRUE if successful, FALSE otherwise.
 */
rdk_logger_Bool rdk_logger_enable_logLevel(const char *module, rdk_LogLevel logLevel, rdk_logger_Bool enableLogLvl);

/**
 * @brief Log a message for onboard logging.
 * @param module Module name.
 * @param msg Message to log.
 */
void rdk_logger_log_onboard(const char *module, const char *msg, ...) __attribute__ ((format (printf, 2, 3)));

/**
 * @brief legacy method to Log a message with printf-style formatting;
 * @param level Log level.
 * @param module Module name.
 * @param format Printf-style format string.
 */
void rdk_dbg_MsgRaw(rdk_LogLevel level, const char *module, const char *format, ...) __attribute__ ((format (printf, 3, 4)));

/**
 * @brief legacy method to Log a message using a va_list.
 * @param level Log level.
 * @param module Module name.
 * @param format Printf-style format string.
 * @param args va_list of arguments.
 */
void rdk_dbg_MsgRaw1(rdk_LogLevel level, const char *module, const char *format, va_list args);

/**
 * @brief Convert a log level string to rdk_LogLevel enum.
 * @param level Log level as string (e.g., "INFO", "DEBUG").
 * @return Corresponding rdk_LogLevel value, or RDK_LOG_NONE if invalid.
 */
rdk_LogLevel rdk_logger_level_from_string(const char* level);

/** @} */ //end of Doxygen tag RDKLOGGER_UTILS_API

#ifdef __cplusplus
}
#endif

#endif /* _RDK_LOGGER_H_ */
