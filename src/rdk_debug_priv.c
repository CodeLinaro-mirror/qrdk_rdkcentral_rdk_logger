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

#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
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

#include "rdk_debug_priv.h"
#include "rdk_dynamic_logger.h"
#include "log4c.h"
#include <log4c/appender_type_rollingfile.h>
#include <log4c/rollingpolicy.h>
#include <log4c/rollingpolicy_type_sizewin.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYSTEMD
#include <systemd/sd-journal.h>
#endif //HAVE_SYSTEMD

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif //HAVE_SYSLOG_H


log4c_category_t* gRootCat = NULL;
int gRootPriority = LOG4C_PRIORITY_WARN;
const char* gRootCatName = "LOG.RDK";
static pthread_mutex_t gLoggingMutex = PTHREAD_MUTEX_INITIALIZER;

/* 1024 is allowed; but lets leave some space for timestamp  */
#define LOG4C_MSG_BUFFER_SIZE   980

/* Define the priority as -ve to avoid printing */
#define LOG4C_PRIORITY_NONE     -1

/* Pattern-based duplicate log detection */
#define MAX_LOG_HASH_SIZE       256
#define MAX_PATTERN_LENGTH      3  /* Support patterns up to length 3 (ABC ABC ABC...) */
#define HISTORY_BUFFER_SIZE     6  /* Need 2*MAX_PATTERN_LENGTH to detect patterns */

typedef struct {
    char module_name[64];
    char message[LOG4C_MSG_BUFFER_SIZE];
    rdk_LogLevel level;
} log_entry_t;

typedef struct {
    /* History buffer to store recent messages (circular buffer) */
    log_entry_t history[HISTORY_BUFFER_SIZE];
    int history_head;    /* Next write position in circular buffer (0 to HISTORY_BUFFER_SIZE-1) */
    int history_count;   /* Number of messages in history (0 to HISTORY_BUFFER_SIZE) */
    
    /* Pattern tracking state */
    int pattern_length;         /* 0=inactive, 1-3=active pattern of that length */
    int next_expected_index;    /* Which entry in pattern we expect next (0 to pattern_length-1) */
    unsigned int repeat_count;  /* How many complete pattern cycles we've seen */
    time_t first_timestamp;
    time_t last_timestamp;
    
    /* Pattern storage (stores the detected pattern) */
    log_entry_t pattern[MAX_PATTERN_LENGTH];
} pattern_tracker_t;

static pattern_tracker_t g_pattern_tracker = {0};
static pthread_mutex_t g_duplicate_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_duplicate_suppression_initialized = false;

/* Debug flag file - create this file to enable debug logging at runtime */
#define DUPLICATE_DEBUG_FLAG_FILE "/tmp/rdk_logger_debug"

/* Check if debug mode is enabled by checking for flag file */
static inline bool is_duplicate_debug_enabled(void)
{
    static time_t last_check = 0;
    static bool last_result = false;
    time_t now = time(NULL);
    
    /* Check file existence every 5 seconds to avoid excessive file system calls */
    if (now - last_check >= 5)
    {
        last_check = now;
        last_result = (access(DUPLICATE_DEBUG_FLAG_FILE, F_OK) == 0);
    }
    return last_result;
}

#define DUP_DEBUG_LOG(fmt, ...) \
    do { \
        if (is_duplicate_debug_enabled()) { \
            fprintf(stderr, "[DUP_SUPPRESS_DEBUG] " fmt "\n", ##__VA_ARGS__); \
        } \
    } while(0)

/* Check if two log entries match (module, level, and message) */
static inline bool log_entries_match(const log_entry_t* a, const log_entry_t* b)
{
    /* Safety: NULL pointer checks */
    if (!a || !b)
    {
        return false;
    }
    
    return (a->level == b->level &&
            strcmp(a->module_name, b->module_name) == 0 &&
            strcmp(a->message, b->message) == 0);
}

/* Flush pattern summary if pattern is active */
static void flush_pattern_summary(log4c_category_t* cat, int log4cPriority)
{
    /* Safety: NULL check for category and validate pattern_length */
    if (!cat || g_pattern_tracker.pattern_length <= 0 || 
        g_pattern_tracker.pattern_length > MAX_PATTERN_LENGTH ||
        g_pattern_tracker.repeat_count == 0)
    {
        return;
    }
    
    if (g_pattern_tracker.repeat_count > 0)
    {
        double duration = difftime(g_pattern_tracker.last_timestamp, g_pattern_tracker.first_timestamp);
        unsigned int suppressed_messages = g_pattern_tracker.repeat_count * g_pattern_tracker.pattern_length;
        
        DUP_DEBUG_LOG("Flushing pattern: length=%d, cycles=%u, total_suppressed=%u, duration=%.0f seconds",
                     g_pattern_tracker.pattern_length, g_pattern_tracker.repeat_count, 
                     suppressed_messages, duration);
        
        if (g_pattern_tracker.pattern_length == 1)
        {
            log4c_category_log(cat, log4cPriority, 
                              "[PATTERN] Previous message repeated %u times (suppressed for %.0f seconds)\n",
                              suppressed_messages, duration);
        }
        else
        {
            log4c_category_log(cat, log4cPriority, 
                              "[PATTERN] Previous %d-message pattern repeated %u times (%u messages suppressed for %.0f seconds)\n",
                              g_pattern_tracker.pattern_length, g_pattern_tracker.repeat_count, 
                              suppressed_messages, duration);
        }
    }
    
    /* Reset pattern tracking state */
    g_pattern_tracker.pattern_length = 0;
    g_pattern_tracker.next_expected_index = 0;
    g_pattern_tracker.repeat_count = 0;
}

/* Add a log entry to the circular history buffer */
static void add_to_history(const char* module_name, const char* message, rdk_LogLevel level)
{
    /* Safety: Check for NULL message pointer */
    if (!message)
    {
        DUP_DEBUG_LOG("WARNING: add_to_history called with NULL message");
        return;
    }
    
    int pos = g_pattern_tracker.history_head;
    log_entry_t* entry = &g_pattern_tracker.history[pos];
    const char* mod_name = (module_name && *module_name) ? module_name : "";
    
    strncpy(entry->module_name, mod_name, sizeof(entry->module_name) - 1);
    entry->module_name[sizeof(entry->module_name) - 1] = '\0';
    
    strncpy(entry->message, message, sizeof(entry->message) - 1);
    entry->message[sizeof(entry->message) - 1] = '\0';
    
    entry->level = level;
    
    /* Move head forward */
    g_pattern_tracker.history_head = (pos + 1) % HISTORY_BUFFER_SIZE;
    
    /* Increment count if not full */
    if (g_pattern_tracker.history_count < HISTORY_BUFFER_SIZE)
    {
        g_pattern_tracker.history_count++;
    }
}

/* Get history entry at offset (0 = most recent, 1 = second most recent, etc.) */
static log_entry_t* get_history(int offset)
{
    /* Safety: Validate offset bounds */
    if (offset < 0 || offset >= g_pattern_tracker.history_count || offset >= HISTORY_BUFFER_SIZE)
    {
        return NULL;  /* Invalid offset or not enough history */
    }
    
    /* Calculate position: most recent is head-1, second most recent is head-2, etc. */
    int pos = (g_pattern_tracker.history_head - 1 - offset + HISTORY_BUFFER_SIZE) % HISTORY_BUFFER_SIZE;
    return &g_pattern_tracker.history[pos];
}

/* Store a log entry in the pattern array at specified index */
static void store_pattern_entry(int index, const log_entry_t* entry)
{
    /* Safety: Validate index and entry pointer */
    if (index < 0 || index >= MAX_PATTERN_LENGTH || !entry)
    {
        DUP_DEBUG_LOG("ERROR: store_pattern_entry invalid params: index=%d, entry=%p", index, (void*)entry);
        return;
    }
    
    log_entry_t* dest = &g_pattern_tracker.pattern[index];
    
    strncpy(dest->module_name, entry->module_name, sizeof(dest->module_name) - 1);
    dest->module_name[sizeof(dest->module_name) - 1] = '\0';
    
    strncpy(dest->message, entry->message, sizeof(dest->message) - 1);
    dest->message[sizeof(dest->message) - 1] = '\0';
    
    dest->level = entry->level;
}

/* Try to detect a repeating pattern from current message and history
 * Returns detected pattern length (1-3), or 0 if no pattern detected
 * current_entry: the new message that just arrived
 */
static int try_detect_pattern(const log_entry_t* current_entry)
{
    /* Safety: NULL check for current_entry */
    if (!current_entry)
    {
        DUP_DEBUG_LOG("ERROR: try_detect_pattern called with NULL current_entry");
        return 0;
    }
    
    /* Try pattern length 1: A A  
     * Need: current matches history[0]
     */
    log_entry_t* h0 = get_history(0);
    if (h0 && log_entries_match(current_entry, h0))
    {
        DUP_DEBUG_LOG("Pattern detected: length=1 (A A...)");
        /* Store pattern */
        store_pattern_entry(0, h0);
        return 1;
    }
    
    /* Try pattern length 2: A B A B
     * Need: current matches history[1], and history[0] matches history[2]
     * History layout: ... [hist2=A] [hist1=B] [hist0=A] [current=B]
     * Additional safety: Verify h0 and h1 are different to avoid false match with single repeats
     */
    log_entry_t* h1 = get_history(1);
    log_entry_t* h2 = get_history(2);
    if (h1 && h2 && 
        log_entries_match(current_entry, h1) &&
        log_entries_match(h0, h2) &&
        !log_entries_match(h0, h1))  /* Safety: Ensure A != B */
    {
        DUP_DEBUG_LOG("Pattern detected: length=2 (A B A B...)");
        /* Store pattern: pattern[0]=A, pattern[1]=B */
        store_pattern_entry(0, h0);  /* A */
        store_pattern_entry(1, current_entry); /* B */
        return 2;
    }
    
    /* Try pattern length 3: A B C A B C
     * Need: current matches history[2], history[0] matches history[3], history[1] matches history[4]
     * History layout: [hist4=A] [hist3=B] [hist2=C] [hist1=A] [hist0=B] [current=C]
     * Additional safety: Ensure A, B, C are all different
     */
    log_entry_t* h3 = get_history(3);
    log_entry_t* h4 = get_history(4);
    if (h2 && h3 && h4 &&
        log_entries_match(current_entry, h2) &&
        log_entries_match(h0, h3) &&
        log_entries_match(h1, h4) &&
        !log_entries_match(h0, h1) &&  /* Safety: B != A */
        !log_entries_match(h1, current_entry) &&  /* Safety: A != C */
        !log_entries_match(h0, current_entry))    /* Safety: B != C */
    {
        DUP_DEBUG_LOG("Pattern detected: length=3 (A B C A B C...)");
        /* Store pattern as [A, B, C] which is [hist1, hist0, current] */
        store_pattern_entry(0, h1);          /* A */
        store_pattern_entry(1, h0);          /* B */
        store_pattern_entry(2, current_entry); /* C */
        return 3;
    }
    
    return 0; /* No pattern detected */
}

/**
 * Declare format/layout APIs.
 */
static const char* rdk_plaintext(const log4c_layout_t* a_layout, const log4c_logging_event_t* a_event);
static const char* rdk_with_ts(const log4c_layout_t* a_layout, const log4c_logging_event_t* a_event);
static const char* rdk_detail_with_ts(const log4c_layout_t* a_layout, const log4c_logging_event_t* a_event);
static const char* rdk_detail_without_ts(const log4c_layout_t* a_layout, const log4c_logging_event_t* a_event);
static const char* rdk_detail_format_handler(const log4c_layout_t* layout, const log4c_logging_event_t* event, bool isTimed);

/**
 * Initialize Layout API.
 */
static const log4c_layout_type_t log4c_layout_type_rdk_plaintext = {"format_plaintext", rdk_plaintext};
static const log4c_layout_type_t log4c_layout_type_rdk_with_ts = {"format_with_ts", rdk_with_ts};
static const log4c_layout_type_t log4c_layout_type_rdk_detail_with_ts = {"format_detail_with_ts", rdk_detail_with_ts};
static const log4c_layout_type_t log4c_layout_type_rdk_detail_without_ts = {"format_detail_without_ts", rdk_detail_without_ts};

// For backward Compatibility
static const log4c_layout_type_t log4c_layout_type_comcast_dated  = {"comcast_dated_nocr", rdk_detail_with_ts};

/**
 * Initialize Appender API.
 */
static int to_console_open(log4c_appender_t * appender);
static int to_console_append(log4c_appender_t* appender, const log4c_logging_event_t* event);
static int to_console_close(log4c_appender_t * appender);

static int to_syslog_open(log4c_appender_t * appender);
static int to_syslog_append(log4c_appender_t* appender, const log4c_logging_event_t* event);
static int to_syslog_close(log4c_appender_t * appender);

static int to_journal_open(log4c_appender_t * appender);
static int to_journal_append(log4c_appender_t* appender, const log4c_logging_event_t* event);
static int to_journal_close(log4c_appender_t * appender);

static const log4c_appender_type_t log4c_appender_type_to_console = {"to_console",
                                                                     to_console_open,
                                                                     to_console_append,
                                                                     to_console_close
                                                                    };

static const log4c_appender_type_t log4c_appender_type_to_syslog = {"to_syslog",
                                                                     to_syslog_open,
                                                                     to_syslog_append,
                                                                     to_syslog_close
                                                                    };

static const log4c_appender_type_t log4c_appender_type_to_journal = {"to_journal",
                                                                     to_journal_open,
                                                                     to_journal_append,
                                                                     to_journal_close
                                                                    };

static int rdk_logLevel_to_log4c_priority(rdk_LogLevel level)
{
     switch (level)
     {
        case RDK_LOG_FATAL:  return LOG4C_PRIORITY_FATAL;   // 000
        case RDK_LOG_ERROR:  return LOG4C_PRIORITY_ERROR;   // 300
        case RDK_LOG_WARN:   return LOG4C_PRIORITY_WARN;    // 400
        case RDK_LOG_NOTICE: return LOG4C_PRIORITY_NOTICE;  // 500
        case RDK_LOG_INFO:   return LOG4C_PRIORITY_INFO;    // 600
        case RDK_LOG_DEBUG:  return LOG4C_PRIORITY_DEBUG;   // 700
        case RDK_LOG_TRACE:  return LOG4C_PRIORITY_TRACE;   // 800
        case RDK_LOG_NONE:   return LOG4C_PRIORITY_UNKNOWN; // 1000
     }
     return LOG4C_PRIORITY_UNKNOWN;
}

#if defined(HAVE_SYSTEMD) || defined(HAVE_SYSLOG_H)
static int get_syslog_priority(int log4c_pr)
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
#endif /* HAVE_SYSTEMD */

static void trim(char *instr, char* outstr)
{
    char *ptr = instr;
    char *endptr = instr + strlen(instr)-1;
    int length;

    /* Advance pointer to first non-whitespace char */
    while (isspace(*ptr))
        ptr++;

    if (ptr > endptr)
    {
        /*
         * avoid breaking things when there are
         * no non-space characters in instr (JIRA OCORI-2028)
         */
        outstr[0] = '\0';
        return;
    }

    /* Move end pointer toward the front to first non-whitespace char */
    while (isspace(*endptr))
        endptr--;

    length = endptr + 1 - ptr;
    strncpy(outstr,ptr,length);
    outstr[length] = '\0';

}

rdk_Error rdk_logger_parse_config( const char * path)
{
    FILE* f = NULL;
    const int line_buf_len = 256;
    char lineBuffer[line_buf_len];

    if (!path)
    {
        printf("Invalid config file\n");
        return RDK_FAILURE;
    }


    /* Open the env file */
    if ((f = fopen( path,"r")) == NULL)
    {
        printf("**    ERROR!  Could not open configuration file (%s)!    **\n", path);
        return RDK_FAILURE;
    }

    memset(lineBuffer, 0, line_buf_len);
    /* Read each line of the file */
    while (fgets(lineBuffer, line_buf_len, f) != NULL)
    {
        char name[line_buf_len];
        char value[line_buf_len];
        char trimname[line_buf_len];
        char trimvalue[line_buf_len];
        char *equals;
        int length;

        /* Ignore comment lines */
        if (lineBuffer[0] == '#')
            continue;

        /* Ignore lines that do not have an '=' char */
        if ((equals = strchr(lineBuffer,'=')) == NULL)
            continue;

        /* Read the property and store in the cache */
        length = equals - lineBuffer;
        strncpy(name, lineBuffer,length);
        name[length] = '\0'; /* have to null-term */

        length = lineBuffer + strlen(lineBuffer) - equals + 1;
        strncpy(value, equals+1, length);
        value[length] = '\0' ;

        /* Trim all whitespace from name and value strings */
        trim(name, trimname);
        trim(value, trimvalue);

#define COMP_SIGNATURE "LOG.RDK."
#define COMP_SIGNATURE_LEN 8
        if (strcmp("LOG.RDK.DEFAULT", trimname) == 0)
        {
            if (!gRootCat)
                gRootCat = log4c_category_get("LOG.RDK");

            if (gRootCat)
            {
                if (strcasecmp(trimvalue, "NONE") == 0)
                {
                    log4c_category_set_priority(gRootCat, LOG4C_PRIORITY_NONE);
                }
                else
                {
                    int lvl = rdk_logger_level_from_string(trimvalue);
                    if ((lvl >= RDK_LOG_FATAL) && (lvl < RDK_LOG_NONE))
                    {
                        log4c_category_set_priority(gRootCat, rdk_logLevel_to_log4c_priority(lvl));
                    }
                }
                gRootPriority = log4c_category_get_priority(gRootCat);
            }
        }
        else if (strncmp(COMP_SIGNATURE, trimname, COMP_SIGNATURE_LEN) == 0)
        {
            log4c_category_t* cat = log4c_category_get(trimname);
            if (cat)
            {
                if (strcasecmp(trimvalue, "NONE") == 0)
                {
                    log4c_category_set_priority(cat, LOG4C_PRIORITY_NONE);
                }
                else
                {
                    int lvl = rdk_logger_level_from_string(trimvalue);
                    if ((lvl >= RDK_LOG_FATAL) && (lvl < RDK_LOG_NONE))
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

    fclose( f);
    return RDK_SUCCESS;
}

void rdk_dbg_priv_init(void)
{
    ///> These must be set before calling log4c_init so that the log4crc file
    ///> will configure them
    (void) log4c_layout_type_set(&log4c_layout_type_rdk_plaintext);
    (void) log4c_layout_type_set(&log4c_layout_type_rdk_with_ts);
    (void) log4c_layout_type_set(&log4c_layout_type_rdk_detail_with_ts);
    (void) log4c_layout_type_set(&log4c_layout_type_rdk_detail_without_ts);


    (void) log4c_appender_type_set(&log4c_appender_type_to_console);
    (void) log4c_appender_type_set(&log4c_appender_type_to_syslog);
    (void) log4c_appender_type_set(&log4c_appender_type_to_journal);


    if (log4c_init())
        fprintf(stderr, "log4c_init() failed?!");

    /* Register this for legacy Components */
    log4c_layout_t* legacy = log4c_layout_get("comcast_dated");
    if (NULL != legacy)
        (void) log4c_layout_set_type(legacy, &log4c_layout_type_comcast_dated);

    return;
}

rdk_Error rdk_dbg_priv_config(const char* debugConfigFile)
{
    rdk_Error ret = RDK_SUCCESS;
    if (debugConfigFile)
    {
        gRootCat = log4c_category_get(gRootCatName);
        if (!gRootCat)
        {
            gRootCat = log4c_category_new(gRootCatName);
            /* Get the root category */
            if (!gRootCat)
            {
                fprintf(stderr, "RDK Root Category Creation failed?!");
            }
        }
        /* Read the config file & populate pre-configured log levels */
        ret = rdk_logger_parse_config(debugConfigFile);
        /* Get the Root Priority */
        if (gRootCat)
            gRootPriority = log4c_category_get_priority(gRootCat);
    }
    else
    {
        fprintf(stderr, "Invalid conf file!");
        ret = RDK_FAILURE;
    }

    return ret;
}

rdk_Error rdk_dbg_priv_ext_init (const rdk_logger_ext_config_t* config)
{
    rdk_Error ret = RDK_SUCCESS;
    if (!config)
    {
        fprintf(stderr, "Error: config parameter is NULL\n");
        return RDK_FAILURE;
    }

    log4c_category_t* cat = NULL;
    log4c_appender_t* app = NULL;
    pthread_mutex_lock(&gLoggingMutex);
    if (config->pModuleName)
    {
        cat = log4c_category_get(config->pModuleName);
    }
    else
    {
        cat = gRootCat;
    }

    if (cat)
    {
        if ((RDKLOG_OUTPUT_FILE == config->output) && (config->pFilePolicy == NULL))
        {
            fprintf(stderr, "Error: file appender requires non-NULL file policy\n");
            app = NULL;
        }
        else
        {
            char app_name[128] = "";
            const char* cat_name = config->pModuleName ? config->pModuleName : gRootCatName;

            if (RDKLOG_OUTPUT_FILE == config->output)
            {
                snprintf(app_name, sizeof(app_name), "%s.file", cat_name);
            }
            else
            {
                snprintf(app_name, sizeof(app_name), "%s.print", cat_name);
            }

            app = log4c_appender_get(app_name);

            if (app)
            {
                if (RDKLOG_OUTPUT_CONSOLE == config->output)
                {
                    log4c_appender_set_type(app, log4c_appender_type_get("to_console"));
                }
                else if (RDKLOG_OUTPUT_SYSLOG == config->output)
                {
                    log4c_appender_set_type(app, log4c_appender_type_get("to_syslog"));
                }
                else if (RDKLOG_OUTPUT_JOURNAL == config->output)
                {
                    /* Set the Appender Type */
                    log4c_appender_set_type(app, log4c_appender_type_get("to_journal"));
                }
                else if (RDKLOG_OUTPUT_FILE == config->output)
                {
                    rdk_LogOutput_File *pPolicy = config->pFilePolicy;
                    /* Close the Appender to complete the existing file writing;
                     * Also, if the appender is NOT opened yet, log4c will take care of
                     * returning without no-op */
                    log4c_appender_close(app);

                    rollingfile_udata_t *oldData = log4c_appender_get_udata(app);
                    if (oldData)
                    {
                        free(oldData);
                        log4c_appender_set_udata(app, NULL);
                    }

                    /* Set the Appender Type */
                    log4c_appender_set_type(app, log4c_appender_type_get("rollingfile"));

                    rollingfile_udata_t *rudata = rollingfile_make_udata();
                    if (rudata)
                    {
                        rollingfile_udata_set_logdir(rudata, pPolicy->fileLocation);
                        rollingfile_udata_set_files_prefix(rudata, pPolicy->fileName);
                        if (pPolicy->fileCountMax > 0)
                        {
                            long maxBytes = (pPolicy->fileSizeMax > 0) ? pPolicy->fileSizeMax : ROLLINGPOLICY_SIZE_DEFAULT_MAX_FILE_SIZE;
                            char policy_name[128];
                            snprintf(policy_name, sizeof(policy_name), "%s.policy", app_name);

                            log4c_rollingpolicy_t *policy = log4c_rollingpolicy_get(policy_name);
                            if (policy)
                            {
                                log4c_rollingpolicy_set_type(policy, log4c_rollingpolicy_type_get("sizewin"));
                                rollingpolicy_sizewin_udata_t *sizewin_udata = sizewin_make_udata();
                                if (sizewin_udata)
                                {
                                    sizewin_udata_set_file_maxsize(sizewin_udata, maxBytes);
                                    sizewin_udata_set_max_num_files(sizewin_udata, pPolicy->fileCountMax);
                                    log4c_rollingpolicy_set_udata(policy, sizewin_udata);
                                }
                                rollingfile_udata_set_policy(rudata, policy);
                            }
                        }
                        log4c_appender_set_udata(app, rudata);
                    }
                }
            }
        }

        if (app)
        {
            log4c_layout_t* layoutObj = NULL;
            switch (config->format)
            {
                case RDKLOG_FORMAT_PLAINTEXT:
                {
                    layoutObj = log4c_layout_get("rdk_plaintext");
                    break;
                }
                case RDKLOG_FORMAT_WITH_TS:
                {
                    layoutObj = log4c_layout_get("rdk_with_ts");
                    break;
                }
                case RDKLOG_FORMAT_DETAIL_WITH_TS:
                {
                    layoutObj = log4c_layout_get("rdk_detail_with_ts");
                    break;
                }
                case RDKLOG_FORMAT_DETAIL_WITHOUT_TS:
                {
                    layoutObj = log4c_layout_get("rdk_detail_without_ts");
                    break;
                }
                default:
                {
                    layoutObj = log4c_layout_get("rdk_plaintext");
                    break;
                }
            }
            if (layoutObj)
            {
                log4c_appender_set_layout(app, layoutObj);
            }

            log4c_category_set_appender(cat, app);
            log4c_category_set_additivity(cat, 0);

            log4c_priority_level_t prio = rdk_logLevel_to_log4c_priority(config->loglevel);
            log4c_category_set_priority(cat, prio);
        }
        else
        {
            fprintf(stderr, "Failed to get or create log appender\n");
            ret = RDK_FAILURE;
        }
    }
    else
    {
        fprintf(stderr, "Failed to get or create log category\n");
        ret = RDK_FAILURE;
    }

    pthread_mutex_unlock(&gLoggingMutex);
    return ret;
}

void rdk_dbg_priv_deinit()
{
  gRootCat = NULL;
}

/**
 * Format the time into a fixed-length ISO 8601-style timestamp.
 *
 */
static void printTime(const struct tm *pTm, char *pBuff)
{
    sprintf(pBuff,"%02d-%02d-%02dT%02d:%02d:%02d",pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
}

/**
 * @brief Function to check if a specific log level of a module is enabled.
 *
 * @param[in] module The module name or category for which the log level shall be checked (as mentioned in debug.ini).
 * @param[in] level The debug logging level.
 *
 * @return Returns true, if debug log level enabled successfully else returns false.
 */
bool rdk_logger_is_logLevel_enabled(const char *module, rdk_LogLevel level)
{
    bool isEnabled = false;
    log4c_category_t* cat = NULL;
    if (RDK_LOG_NONE == level)
    {
        return false;
    }
    else
    {
    
        pthread_mutex_lock(&gLoggingMutex);
        cat = log4c_category_get(module);
        if (cat)
        {
            int log4cPriority = rdk_logLevel_to_log4c_priority(level);
            if (log4c_category_is_priority_enabled(cat, log4cPriority))
            {
                isEnabled = true;
            }
        }
        else
        {
            /* Default case, return false */
            isEnabled = false;
        }
        pthread_mutex_unlock(&gLoggingMutex);
    }
    return isEnabled;
}

void rdk_dbg_priv_log_msg(rdk_LogLevel level, const char *module_name, const char* format, va_list args)
{
    log4c_category_t* cat = NULL;
    int prio = 0;

    /* Initialize duplicate suppression feature flag on first call */
    if (!g_duplicate_suppression_initialized)
    {
        /* Safety: Ensure global state is properly initialized (should be zero-initialized already) */
        memset(&g_pattern_tracker, 0, sizeof(g_pattern_tracker));
        
        g_duplicate_suppression_initialized = true;
        DUP_DEBUG_LOG("=== Pattern-Based Duplicate Suppression INITIALIZED ===");
        DUP_DEBUG_LOG("Max pattern length: %d, Buffer size: %d bytes, Debug flag file: %s", 
                     MAX_PATTERN_LENGTH, LOG4C_MSG_BUFFER_SIZE, DUPLICATE_DEBUG_FLAG_FILE);
        DUP_DEBUG_LOG("To enable debug logs: touch %s", DUPLICATE_DEBUG_FLAG_FILE);
    }

    /* Handling process request here. This is not a blocking call and it shall return immediately */
    rdk_dyn_log_process_pending_request();

    /* Check the incoming log level; dont print when it is RDK_LOG_NONE */
    if (RDK_LOG_NONE == level)
    {
        return;
    }

    pthread_mutex_lock(&gLoggingMutex);
    cat = log4c_category_get(module_name);
    if(!cat)
    {
        cat = gRootCat;
    }
    else
    {
        prio = log4c_category_get_priority(cat);
        if (LOG4C_PRIORITY_NOTSET ==  prio)
        {
            log4c_category_set_priority(cat, gRootPriority);
            prio = gRootPriority;
        }
    }

    /* To Ensure that we dont log at all when category is invalid */
    if(cat)
    {
        va_list localArg;
        char logMsg[LOG4C_MSG_BUFFER_SIZE] = "";
        int n = 0;
        int log4cPriority = rdk_logLevel_to_log4c_priority(level);
        time_t current_time = time(NULL);
        bool is_duplicate = false;

        /* Safety: Check if time() failed */
        if (current_time == (time_t)(-1))
        {
            current_time = 0;  /* Fallback to epoch time if time() fails */
            DUP_DEBUG_LOG("WARNING: time() call failed, using fallback time");
        }

        va_copy(localArg, args);
        n = vsnprintf(logMsg, LOG4C_MSG_BUFFER_SIZE, format, localArg);
        va_end(localArg);

        /* Pattern-based duplicate detection (only for messages that fit in buffer) */
        if (n <= LOG4C_MSG_BUFFER_SIZE && n > 0)
        {
            const char* mod_name = (module_name && *module_name) ? module_name : "";
            log_entry_t current_entry;
            
            /* Prepare current log entry */
            strncpy(current_entry.module_name, mod_name, sizeof(current_entry.module_name) - 1);
            current_entry.module_name[sizeof(current_entry.module_name) - 1] = '\0';
            strncpy(current_entry.message, logMsg, sizeof(current_entry.message) - 1);
            current_entry.message[sizeof(current_entry.message) - 1] = '\0';
            current_entry.level = level;
            
            pthread_mutex_lock(&g_duplicate_mutex);
            
            /* Check if we have an active pattern */
            if (g_pattern_tracker.pattern_length > 0)
            {
                /* Safety: Validate pattern state before accessing arrays */
                if (g_pattern_tracker.pattern_length > MAX_PATTERN_LENGTH ||
                    g_pattern_tracker.next_expected_index < 0 ||
                    g_pattern_tracker.next_expected_index >= g_pattern_tracker.pattern_length)
                {
                    /* Corrupted state - reset pattern tracking */
                    DUP_DEBUG_LOG("ERROR: Corrupted pattern state detected, resetting (len=%d, idx=%d)",
                                 g_pattern_tracker.pattern_length, g_pattern_tracker.next_expected_index);
                    g_pattern_tracker.pattern_length = 0;
                    g_pattern_tracker.next_expected_index = 0;
                    g_pattern_tracker.repeat_count = 0;
                    pthread_mutex_unlock(&g_duplicate_mutex);
                }
                else
                {
                /* Pattern is active - check if current message continues it */
                int expected_idx = g_pattern_tracker.next_expected_index;
                log_entry_t* expected_entry = &g_pattern_tracker.pattern[expected_idx];
                
                if (log_entries_match(&current_entry, expected_entry))
                {
                    /* Message matches pattern - suppress it */
                    g_pattern_tracker.last_timestamp = current_time;
                    
                    /* Move to next position in pattern (pattern_length already validated above) */
                    g_pattern_tracker.next_expected_index = (expected_idx + 1) % g_pattern_tracker.pattern_length;
                    
                    /* If we completed a full pattern cycle, increment repeat count */
                    if (g_pattern_tracker.next_expected_index == 0)
                    {
                        /* Safety: Check for overflow (very unlikely but possible) */
                        if (g_pattern_tracker.repeat_count < UINT_MAX)
                        {
                            g_pattern_tracker.repeat_count++;
                        }
                    }
                    
                    is_duplicate = true;
                    DUP_DEBUG_LOG("SUPPRESSED: pattern continues (length=%d, cycles=%u, pos=%d->%d, msg='%.30s')",
                                 g_pattern_tracker.pattern_length, g_pattern_tracker.repeat_count,
                                 expected_idx, g_pattern_tracker.next_expected_index, logMsg);
                    
                    /* Add to history for potential future pattern changes */
                    add_to_history(mod_name, logMsg, level);
                    pthread_mutex_unlock(&g_duplicate_mutex);
                }
                else
                {
                    /* Pattern broken - flush summary and try to detect new pattern */
                    bool need_flush = (g_pattern_tracker.repeat_count > 0);
                    pthread_mutex_unlock(&g_duplicate_mutex);
                    
                    if (need_flush)
                    {
                        DUP_DEBUG_LOG("Pattern BROKEN, flushing summary");
                        flush_pattern_summary(cat, log4cPriority);
                    }
                    
                    /* Try to detect new pattern using current message and history (DON'T add to history yet) */
                    pthread_mutex_lock(&g_duplicate_mutex);
                    
                    int detected_len = try_detect_pattern(&current_entry);
                    
                    if (detected_len > 0)
                    {
                        /* New pattern detected - initialize pattern tracking */
                        g_pattern_tracker.pattern_length = detected_len;
                        g_pattern_tracker.next_expected_index = 0; /* Next message should match pattern[0] */
                        g_pattern_tracker.repeat_count = 1; /* First complete cycle seen */
                        g_pattern_tracker.first_timestamp = current_time;
                        g_pattern_tracker.last_timestamp = current_time;
                        is_duplicate = true; /* Suppress current message as it's part of detected pattern */
                        
                        /* Now add to history since we detected a pattern */
                        add_to_history(mod_name, logMsg, level);
                        
                        DUP_DEBUG_LOG("NEW pattern started: length=%d, msg='%.30s'", detected_len, logMsg);
                        pthread_mutex_unlock(&g_duplicate_mutex);
                    }
                    else
                    {
                        /* No pattern detected - this is a normal message, add to history now */
                        add_to_history(mod_name, logMsg, level);
                        g_pattern_tracker.pattern_length = 0;
                        g_pattern_tracker.next_expected_index = 0;
                        g_pattern_tracker.repeat_count = 0;
                        
                        DUP_DEBUG_LOG("NO pattern: normal message, msg='%.30s'", logMsg);
                        pthread_mutex_unlock(&g_duplicate_mutex);
                    }
                }
                } /* end of else block for valid pattern state */
            }
            else
            {
                /* No active pattern - try to detect one (DON'T add to history yet) */
                int detected_len = try_detect_pattern(&current_entry);
                
                if (detected_len > 0)
                {
                    /* Pattern detected! Initialize tracking */
                    g_pattern_tracker.pattern_length = detected_len;
                    g_pattern_tracker.next_expected_index = 0;
                    g_pattern_tracker.repeat_count = 1;
                    g_pattern_tracker.first_timestamp = current_time;
                    g_pattern_tracker.last_timestamp = current_time;
                    is_duplicate = true; /* Suppress this message */
                    
                    /* Add to history since pattern detected */
                    add_to_history(mod_name, logMsg, level);
                    
                    DUP_DEBUG_LOG("FIRST pattern detected: length=%d, msg='%.30s'", detected_len, logMsg);
                    pthread_mutex_unlock(&g_duplicate_mutex);
                }
                else
                {
                    /* No pattern yet - output message normally and add to history */
                    add_to_history(mod_name, logMsg, level);
                    
                    DUP_DEBUG_LOG("Added to history: msg='%.30s' (count=%d)", logMsg, g_pattern_tracker.history_count);
                    pthread_mutex_unlock(&g_duplicate_mutex);
                }
            }
        }
        else if (n > LOG4C_MSG_BUFFER_SIZE)
        {
            DUP_DEBUG_LOG("SKIP tracking (message too large): size=%d bytes, module=%s", n, module_name ? module_name : "NULL");
        }

        /* Only log if not a duplicate */
        if (!is_duplicate)
        {
            if (n > LOG4C_MSG_BUFFER_SIZE)
            {
                // Lets allocate the memory and split into multiple chunks of LOG4C_MSG_BUFFER_SIZE
                char *p = (char*) malloc(n + 1);
                if (p)
                {
                    va_list reAllocArg;
                    int toPrint = 0;
                    int i = 0;

                    va_copy(reAllocArg, args);
                    n = vsnprintf(p, n+1, format, reAllocArg);
                    va_end(reAllocArg);

                    for (i = 0; i < n; i += toPrint)
                    {
                        toPrint = ((n - i) < LOG4C_MSG_BUFFER_SIZE) ? (n - i) : LOG4C_MSG_BUFFER_SIZE;
                        log4c_category_log(cat, log4cPriority, "%.*s\n", toPrint, p+i);
                    }
                    free(p);
                }
            }
            else
            {
                log4c_category_log(cat, log4cPriority, "%s", logMsg);
            }
        }
    }
    pthread_mutex_unlock(&gLoggingMutex);

    return;
}


bool rdk_dbg_priv_log_reconfig(const char *pModuleName, rdk_LogLevel logLevel)
{
    bool ret = true;
    log4c_category_t* cat = NULL;
    log4c_priority_level_t prio = gRootPriority; // default
    prio = rdk_logLevel_to_log4c_priority(logLevel);

    pthread_mutex_lock(&gLoggingMutex);
    if (pModuleName)
    {
        cat = log4c_category_get(pModuleName);

        if (cat) {
            log4c_category_set_priority(cat, prio);
        }
    }
    else
    {
        ret = false;
    }

    pthread_mutex_unlock(&gLoggingMutex);

    return ret;
}

/****************************************************************
 * Plain Text format with no ending carriage return / line feed
 */
static const char* rdk_plaintext(const log4c_layout_t* layout, const log4c_logging_event_t* event)
{
    (void) snprintf(event->evt_buffer.buf_data, event->evt_buffer.buf_size, "[%-5s] %s",
                                                                        log4c_priority_to_string(event->evt_priority),
                                                                        event->evt_msg);

    return event->evt_buffer.buf_data;
}

#define COMCAST_DATAED_BUFF_SIZE    40
/****************************************************************
 * With TimeStamp format with no ending carriage return / line feed
 */
static const char* rdk_with_ts(const log4c_layout_t* layout, const log4c_logging_event_t* event)
{
    struct tm tm;
    char timeBuff[COMCAST_DATAED_BUFF_SIZE] = {0};

    gmtime_r(&event->evt_timestamp.tv_sec, &tm);
    printTime(&tm,timeBuff);

    (void) snprintf(event->evt_buffer.buf_data, event->evt_buffer.buf_size, "%s.%06ld [%-5s] %s", timeBuff,
                                                                                                event->evt_timestamp.tv_usec,
                                                                                                log4c_priority_to_string(event->evt_priority),
                                                                                                event->evt_msg);

    return event->evt_buffer.buf_data;
}

/****************************************************************
 * Detailed format with/without timestamp no ending carriage return / line feed
 */
static const char* rdk_detail_format_handler(const log4c_layout_t* layout, const log4c_logging_event_t* event, bool isTimed)
{
    struct tm tm;
    char timeBuff[COMCAST_DATAED_BUFF_SIZE] = {0};

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

    if (isTimed)
    {
        gmtime_r(&event->evt_timestamp.tv_sec, &tm);
        printTime(&tm,timeBuff);

        (void) snprintf(event->evt_buffer.buf_data, event->evt_buffer.buf_size, "%s.%06ld [%-5s] [%s] [%ld] %s",
                                                    timeBuff,
                                                    event->evt_timestamp.tv_usec,
                                                    log4c_priority_to_string(event->evt_priority),
                                                    p,
                                                    syscall(SYS_gettid),
                                                    event->evt_msg);
    }
    else
    {
        (void) snprintf(event->evt_buffer.buf_data, event->evt_buffer.buf_size, "[%-5s] [%s] [%ld] %s",
                                                    log4c_priority_to_string(event->evt_priority),
                                                    p,
                                                    syscall(SYS_gettid),
                                                    event->evt_msg);
    }

    return event->evt_buffer.buf_data;
}


/****************************************************************
 * Detailed format with timestamp
 */
static const char* rdk_detail_with_ts(const log4c_layout_t* layout, const log4c_logging_event_t* event)
{
    return rdk_detail_format_handler(layout, event, true);
}

/****************************************************************
 * Detailed format without timestamp
 */
static const char* rdk_detail_without_ts(const log4c_layout_t* layout, const log4c_logging_event_t* event)
{
    return rdk_detail_format_handler(layout, event, false);
}

/*****************************************************************/
static int to_console_open(log4c_appender_t* appender)
{
    FILE* fp = (FILE*)log4c_appender_get_udata(appender);

    if (fp)
        return 0;
    else
        fp = stdout;

    /** Set unbuffered mode */
    setbuf(fp, NULL);

    (void)log4c_appender_set_udata(appender, fp);
    return 0;
}

static int to_console_append(log4c_appender_t* appender, const log4c_logging_event_t* event)
{
    FILE* fp = (FILE*)log4c_appender_get_udata(appender);

    if (!fp)
        fp = stdout;

    fprintf(fp, "%s", event->evt_rendered_msg);
    (void)fflush(fp);

    return 0;
}

static int to_console_close(log4c_appender_t* appender)
{
    (void) appender;
    return 0;
}

static int to_syslog_open(log4c_appender_t * appender)
{
#ifdef HAVE_SYSLOG_H
    openlog(NULL, LOG_PID, LOG_USER);
#endif /* HAVE_SYSLOG_H */
    return 0;
}

static int to_syslog_append(log4c_appender_t* appender, const log4c_logging_event_t* event)
{
#ifdef HAVE_SYSLOG_H
    syslog(get_syslog_priority(event->evt_priority), "%s", event->evt_rendered_msg);
#endif /* HAVE_SYSLOG_H */
    return 0;
}

static int to_syslog_close(log4c_appender_t * appender)
{
    (void) appender;
    return 0;
}

static int to_journal_open(log4c_appender_t * appender)
{
    (void) appender;
    return 0;
}

static int to_journal_append(log4c_appender_t* appender, const log4c_logging_event_t* event)
{
#ifdef HAVE_SYSTEMD
    sd_journal_print(get_syslog_priority(event->evt_priority), "%s", event->evt_rendered_msg);
#endif /* HAVE_SYSTEMD */
    return 0;
}

static int to_journal_close(log4c_appender_t * appender)
{
    (void) appender;
    return 0;
}

/* End Of File */
