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
 * @file rdk_debug_priv.c
 */


/**
* @defgroup rdk_logger
* @{
* @defgroup src
* @{
**/


#include <assert.h>
#include <stdio.h>
/*lint -e(451)*/
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>


#include "rdk_logger_types.h"
#include "rdk_error.h"
#define RDK_DEBUG_DEFINE_STRINGS
#include "rdk_debug_priv.h"
#include "rdk_dynamic_logger.h"
#include "log4c.h"
#include <rdk_utils.h>
#include <log4c/appender_type_rollingfile.h>
#include <log4c/rollingpolicy.h>
#include <log4c/rollingpolicy_type_sizewin.h>

#ifdef SYSTEMD_JOURNAL
#include <systemd/sd-journal.h>
#endif //SYSTEMD_JOURNAL


#ifdef SYSTEMD_SYSLOG_HELPER
#include "syslog_helper_ifc.h"
#endif

log4c_category_t* gRootCat = NULL;
int gRootPriority = 0;
extern int global_count;

static int rdk_logLevel_to_log4c_priority(int level) {
     switch (level) {
         case 0: return LOG4C_PRIORITY_FATAL;   // 000
         case 1: return LOG4C_PRIORITY_ERROR;   // 300
         case 2: return LOG4C_PRIORITY_WARN;    // 400
         case 3: return LOG4C_PRIORITY_NOTICE;  // 500
         case 4: return LOG4C_PRIORITY_INFO;    // 600
         case 5: return LOG4C_PRIORITY_DEBUG;   // 700
         case 6: return LOG4C_PRIORITY_TRACE;   // 800
         default: return LOG4C_PRIORITY_UNKNOWN;
     }
 }

#define LOG4C_PRIORITY_NONE -1

/**
 * Returns 1 if logging has been requested for the corresponding module (mod)
 * and level (lvl) combination. To be used in rdk_dbg_priv_* files ONLY.
 */
#define IS_LOGGING_ENABLED_FOR_LEVEL(module_name, level) \
    (log4c_category_get(module_name) && \
     (rdk_logLevel_to_log4c_priority(level) <= log4c_category_get_priority(log4c_category_get(module_name))))

/** Skip whitespace in a c-style string. */
#define SKIPWHITE(cptr) while ((*cptr != '\0') && isspace(*cptr)) cptr++

#define HOSTADDR_STR_MAX 255


#define MAX_LOGLINE_LENGTH 4096

static int initLogger(char *category);

static const char* dated_format_nocr(const log4c_layout_t* a_layout,
        const log4c_logging_event_t*a_event);
static const char* basic_format_nocr(const log4c_layout_t* a_layout,
        const log4c_logging_event_t*a_event);
static const char* comcast_dated_format_nocr(const log4c_layout_t* a_layout,
         log4c_logging_event_t*a_event);
static int stream_env_overwrite_open(log4c_appender_t * appender);
static int stream_env_append_open(log4c_appender_t * appender);
static int stream_env_append(log4c_appender_t* appender, const log4c_logging_event_t* event);
static int stream_env_plus_stdout_append(log4c_appender_t* appender, const log4c_logging_event_t* event);
static int stream_env_close(log4c_appender_t * appender);

/**
 * Initialize Debug API.
 */
static const log4c_layout_type_t log4c_layout_type_dated_nocr =
{ "dated_nocr", dated_format_nocr, };

static const log4c_layout_type_t log4c_layout_type_basic_nocr =
{ "basic_nocr", basic_format_nocr, };

static const log4c_layout_type_t log4c_layout_type_comcast_dated_nocr =
{ "comcast_dated_nocr", comcast_dated_format_nocr, };

static const log4c_appender_type_t
        log4c_appender_type_stream_env =
        { "stream_env", stream_env_overwrite_open, stream_env_append,
                stream_env_close, };

static const log4c_appender_type_t log4c_appender_type_stream_env_append =
{ "stream_env_append", stream_env_append_open, stream_env_append,
        stream_env_close, };

static const log4c_appender_type_t log4c_appender_type_stream_env_plus_stdout =
{ "stream_env_plus_stdout", stream_env_overwrite_open,
        stream_env_plus_stdout_append, stream_env_close, };

static const log4c_appender_type_t log4c_appender_type_stream_env_append_plus_stdout =
{ "stream_env_append_plus_stdout", stream_env_append_open,
        stream_env_plus_stdout_append, stream_env_close, };

void rdk_dbg_priv_init()
{
    if (initLogger("LOG.RDK"))
    {
        fprintf(stderr, "%s -- initLogger failure?!\n", __FUNCTION__);
    }

    gRootCat = log4c_category_get("LOG.RDK");
}

void rdk_dbg_priv_ext_init(const char* logdir, const char* log_file_name, long maxCount, long maxSize)
{
    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", logdir, log_file_name);

    const char* cat_name = "LOG.RDK";
    log4c_category_t* cat = log4c_category_get(cat_name);
    if (!cat) {
        cat = log4c_category_new(cat_name);
    }

    log4c_appender_t* app = log4c_appender_get(fullpath);
    if (!app) {
        app = log4c_appender_new(fullpath);
    }
    log4c_appender_set_type(app, log4c_appender_type_get("rollingfile"));

    rollingfile_udata_t *rudata = rollingfile_make_udata();
    rollingfile_udata_set_logdir(rudata, logdir);
    rollingfile_udata_set_files_prefix(rudata, log_file_name);

    log4c_rollingpolicy_t *policy = log4c_rollingpolicy_get(cat_name);
    if (!policy) {
        policy = log4c_rollingpolicy_new(cat_name);
    }
    log4c_rollingpolicy_set_type(policy, log4c_rollingpolicy_type_get("sizewin"));

    rollingpolicy_sizewin_udata_t *sizewin_udata = sizewin_make_udata();
    sizewin_udata_set_file_maxsize(sizewin_udata, maxSize);
    sizewin_udata_set_max_num_files(sizewin_udata, maxCount);
    log4c_rollingpolicy_set_udata(policy, sizewin_udata);

    rollingfile_udata_set_policy(rudata, policy);
    log4c_appender_set_udata(app, rudata);

    log4c_layout_t* layout = log4c_layout_get("comcast_dated");
    log4c_appender_set_layout(app, layout);

    log4c_category_set_appender(cat, app);
}

void rdk_dbg_priv_deinit()
{
  gRootCat = NULL;
}

/**
 * Safely force a string to uppercase. I hate this mundane rubbish.
 *
 * @param token String to be forced to uppercase.
 */
static void forceUpperCase(char *token)
{
    while (*token)
    {
        if (islower(*token))
        {
            *token = (char) toupper(*token);
        }
        token++;
    }
}

/**
 * String names that correspond to the various logging types.
 * Note: This array *must* match the RDK_LOG_* enum.
 */
const char *rdk_logLevelStrings[RDK_LOG_NONE] =
{
    "FATAL",
    "ERROR",
    "WARNING",
    "NOTICE",
    "INFO",
    "DEBUG",
    "TRACE",
};

/**
 * Convert a log level name to the correspodning log level enum value.
 *
 * @param name Log level name, which must be uppercase.
 * @param Corresponding enumeration value or -1 on error.
 */
static int logNameToEnum(const char *name)
{
    int i = 0;
    while (i < RDK_LOG_NONE)
    {
        if (strcmp(name, rdk_logLevelStrings[i]) == 0)
        {
            return i;
        }
        i++;
    }

    return -1;
}

static void printTime(const struct tm *pTm, char *pBuff)
{
    sprintf(pBuff,"%02d%02d%02d-%02d:%02d:%02d",pTm->tm_year + 1900 - 2000, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
}

/*****************************************************************************
 *
 * EXPORTED FUNCTIONS
 *
 ****************************************************************************/
void rdk_dbg_priv_config(void)
{
    int mod = 1;
    const char *defaultValue = rdk_logger_envGet("LOG.RDK.DEFAULT");
    if (defaultValue && defaultValue[0] != 0)
    {
        char levelName[32];
        strncpy(levelName, defaultValue, sizeof(levelName)-1);
        levelName[sizeof(levelName)-1] = '\0';
        forceUpperCase(levelName);

        if (!gRootCat)
            gRootCat = log4c_category_get("LOG.RDK");

        if (!gRootCat)
        {
            printf("LOG.RDK.DEFAULT is not defined..\n");
            return;
        }

        if (strcasecmp(levelName, "NONE") == 0)
        {
            log4c_category_set_priority(gRootCat, LOG4C_PRIORITY_NONE);
        }
        else
        {
            int lvl = logNameToEnum(levelName);
            if (lvl >= 0 && lvl < RDK_LOG_NONE)
            {
                log4c_category_set_priority(gRootCat, rdk_logLevel_to_log4c_priority(lvl));
            }
        }
        gRootPriority = log4c_category_get_priority(gRootCat);
    }
    else 
    {
        printf("LOG.RDK.DEFAULT is not defined..\n");
        return;
    }

    for (mod = 1; mod <= global_count; mod++)
    {
        const char* modName = rdk_logger_envGetModFromNum(mod);
        const char* modValue = rdk_logger_envGetValueFromNum(mod);
        if (modName && modValue && modValue[0] != '\0')
        {
            char levelName[32];
            strncpy(levelName, modValue, sizeof(levelName)-1);
            levelName[sizeof(levelName)-1] = '\0';
            forceUpperCase(levelName);

            log4c_category_t* cat = log4c_category_get(modName);
            if (cat)
            {
                if (strcasecmp(levelName, "NONE") == 0)
                {
                    log4c_category_set_priority(cat, LOG4C_PRIORITY_NONE);
                }
                else
                {
                    int lvl = logNameToEnum(levelName);
                    if (lvl >= 0 && lvl < RDK_LOG_NONE)
                    {
                        log4c_category_set_priority(cat, rdk_logLevel_to_log4c_priority(lvl));
                    }
                    else
                    {
                        log4c_category_set_priority(cat, LOG4C_PRIORITY_NONE);
                    }
                }
            }
        }
    }
}

/**
 * @brief Function to check if a specific log level of a module is enabled.
 *
 * @param[in] module The module name or category for which the log level shall be checked (as mentioned in debug.ini).
 * @param[in] level The debug logging level.
 *
 * @return Returns TRUE, if debug log level enabled successfully else returns FALSE.
 */
rdk_logger_Bool rdk_logger_is_logLevel_enabled(const char *module, rdk_LogLevel level)
{
	if(IS_LOGGING_ENABLED_FOR_LEVEL(module, level))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void rdk_dbg_priv_log_msg(rdk_LogLevel level, const char *module_name, const char* format, va_list args)
{
    log4c_category_t* cat = NULL;
    int prio = 0;

    /* Handling process request here. This is not a blocking call and it shall return immediately */
    rdk_dyn_log_process_pending_request();
    cat = log4c_category_get(module_name);
    prio = log4c_category_get_priority(cat);
    if (cat && prio == LOG4C_PRIORITY_NOTSET && gRootCat) {
        log4c_category_set_priority(cat, gRootPriority);
        prio = gRootPriority;
    }
 
    if(!cat)
    {
        cat = gRootCat;
    }

    if(!cat)
    {
        return;
    }

    if (!IS_LOGGING_ENABLED_FOR_LEVEL(module_name, level))
    {
        return;
    }

    switch (level)
    {
        case RDK_LOG_FATAL:
            log4c_category_vlog(cat, LOG4C_PRIORITY_FATAL, format, args);
            break;
        case RDK_LOG_ERROR:
            log4c_category_vlog(cat, LOG4C_PRIORITY_ERROR, format, args);
            break;
        case RDK_LOG_WARN:
            log4c_category_vlog(cat, LOG4C_PRIORITY_WARN, format, args);
            break;
        case RDK_LOG_NOTICE:
            log4c_category_vlog(cat, LOG4C_PRIORITY_NOTICE, format, args);
            break;
        case RDK_LOG_INFO:
            log4c_category_vlog(cat, LOG4C_PRIORITY_INFO, format, args);
            break;
        case RDK_LOG_DEBUG:
            log4c_category_vlog(cat, LOG4C_PRIORITY_DEBUG, format, args);
            break;
        case RDK_LOG_TRACE:
            log4c_category_vlog(cat, LOG4C_PRIORITY_TRACE, format, args);
            break;
        default:
            log4c_category_vlog(cat, LOG4C_PRIORITY_DEBUG, format, args);
            break;
    }
}


void rdk_dbg_priv_reconfig(const char *pModuleName, const char *pLogLevel)
{
    char logTypeName[20] = {'\0'};

    if ((NULL == pModuleName) || (NULL == pLogLevel))
    {
        return;
    }

    strncpy(logTypeName, pLogLevel, sizeof(logTypeName)-1);
    if (logTypeName[0] == '~')
    {
        logTypeName[0] = '!';
    }

    int disable = 0;
    if (logTypeName[0] == '!') {
        disable = 1;
        memmove(logTypeName, logTypeName + 1, strlen(logTypeName));
    }

    int prio = LOG4C_PRIORITY_INFO; // default
    if (strcasecmp(logTypeName, "FATAL") == 0) prio = LOG4C_PRIORITY_FATAL;
    else if (strcasecmp(logTypeName, "ERROR") == 0) prio = LOG4C_PRIORITY_ERROR;
    else if (strcasecmp(logTypeName, "WARNING") == 0) prio = LOG4C_PRIORITY_WARN;
    else if (strcasecmp(logTypeName, "NOTICE") == 0) prio = LOG4C_PRIORITY_NOTICE;
    else if (strcasecmp(logTypeName, "INFO") == 0) prio = LOG4C_PRIORITY_INFO;
    else if (strcasecmp(logTypeName, "DEBUG") == 0) prio = LOG4C_PRIORITY_DEBUG;
    else if (strcasecmp(logTypeName, "TRACE") == 0) prio = LOG4C_PRIORITY_TRACE;
    else if (strcasecmp(logTypeName, "NONE") == 0) prio = LOG4C_PRIORITY_NONE;

    log4c_category_t* cat = log4c_category_get(pModuleName);
    if (cat) {
        if (disable) {
            log4c_category_set_priority(cat, gRootPriority);
        }
        else {
            log4c_category_set_priority(cat, prio);
        }
    }
}


/*************************Copied from ri_log.c******************/

/**
 * Instantiate the logger...
 * @param cat The category string for this instance logging/filtering
 * @return the result of the instantiation
 */
static int initLogger(char *category)
{

    ///> These must be set before calling log4c_init so that the log4crc file
    ///> will configure them
    (void) log4c_appender_type_set(&log4c_appender_type_stream_env);
    (void) log4c_appender_type_set(&log4c_appender_type_stream_env_append);
    (void) log4c_appender_type_set(&log4c_appender_type_stream_env_plus_stdout);
    (void) log4c_appender_type_set(
            &log4c_appender_type_stream_env_append_plus_stdout);
    (void) log4c_layout_type_set(&log4c_layout_type_dated_nocr);
    (void) log4c_layout_type_set(&log4c_layout_type_basic_nocr);
    (void) log4c_layout_type_set(&log4c_layout_type_comcast_dated_nocr);

    if (log4c_init())
    {
        fprintf(stderr, "log4c_init() failed?!");
        return -1;
    }

    return 0;
}


/****************************************************************
 * Dated layout with no ending carriage return / line feed
 */
static const char* dated_format_nocr(const log4c_layout_t* layout,
        const log4c_logging_event_t* event)
{
    struct tm tm;
    char timeBuff[40];
    //localtime_r(&event->evt_timestamp.tv_sec, &tm); /* Use the UTC Time for logging */
    gmtime_r(&event->evt_timestamp.tv_sec, &tm);

    memset(&timeBuff,0,40);

    printTime(&tm,timeBuff);

    (void) snprintf(event->evt_buffer.buf_data, event->evt_buffer.buf_size,
            "%s.%03ld %-8s %s- %s", timeBuff,
            event->evt_timestamp.tv_usec / 1000, log4c_priority_to_string(
                    event->evt_priority), event->evt_category,
            event->evt_msg);
    if (event->evt_buffer.buf_size > 0 && event->evt_buffer.buf_data != NULL)
    {
        event->evt_buffer.buf_data[event->evt_buffer.buf_size - 1] = 0;
    }
    return event->evt_buffer.buf_data;
}

/****************************************************************
 * Basic layout with no ending carriage return / line feed
 */
static const char* basic_format_nocr(const log4c_layout_t* layout,
        const log4c_logging_event_t* event)
{
    (void) snprintf(event->evt_buffer.buf_data, event->evt_buffer.buf_size, "%-8s %s - %s",
            log4c_priority_to_string(event->evt_priority),
            event->evt_category, event->evt_msg);

    if (event->evt_buffer.buf_size > 0 && event->evt_buffer.buf_data != NULL)
    {
        event->evt_buffer.buf_data[event->evt_buffer.buf_size - 1] = 0;
    }

    return event->evt_buffer.buf_data;
}

#define COMCAST_DATAED_BUFF_SIZE    40

static const char* comcast_dated_format_nocr(const log4c_layout_t* layout,
        log4c_logging_event_t*event)
{
    struct tm tm;
    int n = -1;
    char timeBuff[COMCAST_DATAED_BUFF_SIZE] = {0};
    //localtime_r(&event->evt_timestamp.tv_sec, &tm);  /* Use the UTC Time for logging */
    gmtime_r(&event->evt_timestamp.tv_sec, &tm);

    printTime(&tm,timeBuff);

    /** Get the last part of the cagetory as "module" */
    char *p= (char *)(event->evt_category);
    if (NULL == p)
    {
        p = (char*)"UNKNOWN";
    }
    else
    {
        int len = strlen(p);
        if ( len > 0 && *p != '.' && *(p+len-1) !='.')
        {
            p = p + len - 1;
            while (p != (char *)(event->evt_category) && *p != '.') p--;
            if (*p == '.') p+=1;

        }
        else
        {
            p = (char*)"UNKNOWN";
        }
    }

    n = snprintf(event->evt_buffer.buf_data, event->evt_buffer.buf_size,
            "%s.%06ld [mod=%s, lvl=%s] [tid=%ld] %s",timeBuff,
            event->evt_timestamp.tv_usec,
            p, log4c_priority_to_string(event->evt_priority), syscall(SYS_gettid),
            event->evt_msg);
    if (n > -1 && n > event->evt_buffer.buf_size && event->evt_buffer.buf_maxsize == 0) {
        event->evt_buffer.buf_size = n + COMCAST_DATAED_BUFF_SIZE + 1;
        event->evt_buffer.buf_data = (char *) realloc (event->evt_buffer.buf_data, event->evt_buffer.buf_size);
        //TODO realloc error check
        n = snprintf(event->evt_buffer.buf_data, event->evt_buffer.buf_size,
            "%s.%06ld [mod=%s, lvl=%s] [tid=%ld] %s",timeBuff,
            event->evt_timestamp.tv_usec,
            p, log4c_priority_to_string(event->evt_priority), syscall(SYS_gettid),
            event->evt_msg);
    }
    if (event->evt_buffer.buf_size > 0 && event->evt_buffer.buf_data != NULL)
    {
        event->evt_buffer.buf_data[event->evt_buffer.buf_size - 1] = 0;
    }
    return event->evt_buffer.buf_data;
}
/****************************************************************
 * Stream layout that will parse environment variables from the
 * stream name (env vars have a leading "$(" and end with )"
 */
static int stream_env_open(log4c_appender_t* appender, int append)
{
    FILE* fp = (FILE*)log4c_appender_get_udata(appender);
    char* name = strdup(log4c_appender_get_name(appender));
    int nameLen = strlen(name);
    char* temp = name;
    char *varBegin, *varEnd;
    char* envVar;
    const int MAX_VAR_LEN = 1024;
    char newName[MAX_VAR_LEN+1];
    int newNameLen = 0;

    if (fp)
    {
        free(name); /*RDKB-7467, CID-24968, free unused resources*/
        return 0;
    }

    newName[0] = '\0';

    ///> Parse any environment variables
    while ((varBegin = strchr(temp,'$')) != NULL)
    {
        ///> Search for opening and closing parens
        if (((varBegin - name + 1) >= nameLen) || (*(varBegin+1) != '('))
        goto parse_error;

        ///> Append characters up to this point to the new name
        strncat(newName, temp, varBegin-temp);
        newNameLen += varBegin-temp;
        if (newNameLen > MAX_VAR_LEN)
        goto length_error;

        varBegin += 2; ///> start of env var name

        if ((varEnd = strchr(varBegin,')')) == NULL)
        goto parse_error;

        *varEnd = '\0';
        if ((envVar = getenv(varBegin)) == NULL)
        goto parse_error;

        ///> Append env var value to the new name
        if((newNameLen + strlen(envVar)) < MAX_VAR_LEN){
           strncat(newName, envVar, strlen(envVar));
           newNameLen += strlen(envVar);
        }
        else{
           goto length_error;
        }
        temp = varEnd + 1;
    }

    ///> Append remaining characters
    if (newNameLen + (name + nameLen) - temp > MAX_VAR_LEN)
        goto length_error;
    strncat(newName, temp, (name + nameLen) - temp);
    newNameLen += (name + nameLen) - temp;

    free(name);

    if (!strcmp(newName,"stderr"))
    fp = stderr;
    else if (!strcmp(newName,"stdout"))
    fp = stdout;
    else if (append)
    {
        printf("****Opening %s in append mode\n", newName);
        if ((fp = fopen(newName, "a")) == NULL)
        return -1;
    }
    else
    {
        printf("****Opening %s in write mode\n", newName);
        if ((fp = fopen(newName, "w")) == NULL)
        return -1;
    }

    /**> unbuffered mode */
    setbuf(fp, NULL);

    (void)log4c_appender_set_udata(appender, fp);
    return 0;

    parse_error:
    fprintf(stderr, "*(*(*(*( log4c appender stream_env, %s -- Illegal env var name or format! %s\n",
            __FUNCTION__, name);
    (void)fflush(stderr);
    free(name);
    return -1;

    length_error:
    fprintf(stderr, "*(*(*(*( log4c appender stream_env, %s -- Path is too long! %s\n",
            __FUNCTION__, name);
    (void)fflush(stderr);
    free(name);
    return -1;
}

static int stream_env_overwrite_open(log4c_appender_t* appender)
{
    return stream_env_open(appender, 0);
}

static int stream_env_append_open(log4c_appender_t* appender)
{
    return stream_env_open(appender, 1);
}

#ifdef SYSTEMD_JOURNAL
static int stream_env_append_get_priority(int log4c_pr)
{
    int priority;
    switch(log4c_pr)
    {
    case LOG4C_PRIORITY_FATAL:
        priority = LOG_EMERG;
        break;
    case LOG4C_PRIORITY_ERROR:
        priority = LOG_ERR;
        break;
    case LOG4C_PRIORITY_WARN:
        priority = LOG_WARNING;
        break;
    case LOG4C_PRIORITY_NOTICE:
        priority = LOG_NOTICE;
        break;
    case LOG4C_PRIORITY_INFO:
        priority = LOG_INFO;
        break;
    case LOG4C_PRIORITY_DEBUG:
    case LOG4C_PRIORITY_TRACE:
    default:
        priority = LOG_DEBUG;
        break;
    }
    return priority;
}
#endif

static int stream_env_append(log4c_appender_t* appender,
        const log4c_logging_event_t* event)
{
    int retval=0;
    FILE* fp = (FILE*)log4c_appender_get_udata(appender);

#if defined(SYSTEMD_SYSLOG_HELPER)
    send_logs_to_syslog(event->evt_rendered_msg);
#elif defined(SYSTEMD_JOURNAL)
    if (fp == stdout || fp == stderr)
    {
        retval = sd_journal_print(stream_env_append_get_priority(event->evt_priority), "%s",event->evt_rendered_msg);
    }
    else
    {
        retval = fprintf(fp, "%s", event->evt_rendered_msg);
        (void)fflush(fp);
    }
#else
    retval = fprintf(fp, "%s", event->evt_rendered_msg);
    (void)fflush(fp);
#endif

    //free((void *)event->evt_rendered_msg);

    return retval;
}

static int stream_env_plus_stdout_append(log4c_appender_t* appender,
        const log4c_logging_event_t* event)
{
    int retval=0;
    FILE* fp = (FILE*)log4c_appender_get_udata(appender);

#if defined(SYSTEMD_SYSLOG_HELPER)
        send_logs_to_syslog(event->evt_rendered_msg);
#elif defined(SYSTEMD_JOURNAL)
    if (fp != stdout || fp != stderr)
    {
       retval = fprintf(fp, "%s", event->evt_rendered_msg);
    }
    else
    {
       retval = sd_journal_print(stream_env_append_get_priority(event->evt_priority), "%s",event->evt_rendered_msg);
    }
    (void)fflush(fp);
#else
    retval = fprintf(fp, "%s", event->evt_rendered_msg);
    fprintf(stdout, "%s", event->evt_rendered_msg);
    (void)fflush(fp);
    (void)fflush(stdout);
#endif
    //free((void *)event->evt_rendered_msg);

    return retval;
}


static int stream_env_close(log4c_appender_t* appender)
{
    FILE* fp = (FILE*)log4c_appender_get_udata(appender);

    if (!fp || fp == stdout || fp == stderr)
    return 0;

    return fclose(fp);
}

