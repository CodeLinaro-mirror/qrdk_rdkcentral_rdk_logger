
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include "rdk_logger.h"
#include "gtest_app.h"
typedef enum {
    CAT_FATAL,
    CAT_ERROR,
    CAT_WARN,
    CAT_NOTICE,
    CAT_INFO,
    CAT_DEBUG,
    CAT_TRACE,
    CAT_NONE
} LogCategory;

typedef enum {
    LVL_FATAL,
    LVL_ERROR,
    LVL_WARN,
    LVL_NOTICE,
    LVL_INFO,
    LVL_DEBUG,
    LVL_TRACE,
    LVL_NONE,
    LVL_TEST	    
} LogLevel;

typedef struct {
    LogCategory category;
    LogLevel level;
} rdklogctrl_args_t;

void* run_rdklogctrl(void* arg) {
    rdklogctrl_args_t* args = (rdklogctrl_args_t*)arg;
    char category_str[32] = "";
    char level_str[16] = "";

    // Switch-case for category
    switch (args->category) {
        case CAT_FATAL:
            strcpy(category_str, "LOG.RDK.FATAL");
            break;
        case CAT_ERROR:
            strcpy(category_str, "LOG.RDK.ERROR");
            break;
        case CAT_WARN:
            strcpy(category_str, "LOG.RDK.WARN");
            break;
        case CAT_NOTICE:
            strcpy(category_str, "LOG.RDK.NOTICE");
	    break;
        case CAT_INFO:
            strcpy(category_str, "LOG.RDK.INFO");
	    break;
        case CAT_DEBUG:
            strcpy(category_str, "LOG.RDK.DEBUG");
	    break;
        case CAT_TRACE:
            strcpy(category_str, "LOG.RDK.TRACE");
	    break;
        case CAT_NONE:
            strcpy(category_str, "LOG.RDK.NONE");
	    break;
        default:
            break;
    }

    // Switch-case for level
    switch (args->level) {
        case LVL_FATAL:
            strcpy(level_str, "FATAL");
            break;
        case LVL_ERROR:
            strcpy(level_str, "ERROR");
            break;
        case LVL_WARN:
            strcpy(level_str, "WARN");
            break;
        case LVL_NOTICE:
            strcpy(level_str, "NOTICE");
            break;
        case LVL_INFO:
            strcpy(level_str, "INFO");
            break;
        case LVL_DEBUG:
            strcpy(level_str, "DEBUG");
            break;
        case LVL_TRACE:
            strcpy(level_str, "TRACE");
            break;
        case LVL_NONE:
            strcpy(level_str, "NONE");
            break;
        case LVL_TEST:
            strcpy(level_str, "~NONE");
            break;
        default:
            break;
    }

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "./rdklogctrl rdk_logger_gtest %s %s", category_str, level_str);
    fprintf(stderr, "Executing command: %s\n", cmd);
    int ret = system(cmd);
    (void)ret;
    return NULL;
}

TEST(RdkDynamicLoggerTest, MessageProcessingViaSystem_fatal) {
    rdk_Error ret = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;    
    EXPECT_EQ(rdk_logger_init(conf_file), 0);

    // Prepare arguments for thread
    rdklogctrl_args_t args;
    args.category = CAT_FATAL;
    args.level = LVL_FATAL;

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, &args));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Log a message using the high-level API, which will exercise dynamic logger code
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.TESTMOD", "Test message for dynamic logger\n");
    // Clean up
    pthread_join(client_thread, nullptr);
}

TEST(RdkDynamicLoggerTest, MessageProcessingViaSystem_error) {
    rdk_Error ret = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    EXPECT_EQ(rdk_logger_init(conf_file), 0);

    // Prepare arguments for thread
    rdklogctrl_args_t args;
    args.category = CAT_ERROR;
    args.level = LVL_ERROR;

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, &args));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Log a message using the high-level API, which will exercise dynamic logger code
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.TESTMOD", "Test message for dynamic logger\n");
    // Clean up
    pthread_join(client_thread, nullptr);
}
TEST(RdkDynamicLoggerTest, MessageProcessingViaSystem_warn) {
    rdk_Error ret = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    EXPECT_EQ(rdk_logger_init(conf_file), 0);

    // Prepare arguments for thread
    rdklogctrl_args_t args;
    args.category = CAT_WARN;
    args.level = LVL_WARN;

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, &args));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Log a message using the high-level API, which will exercise dynamic logger code
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.TESTMOD", "Test message for dynamic logger\n");
    // Clean up
    pthread_join(client_thread, nullptr);
}
TEST(RdkDynamicLoggerTest, MessageProcessingViaSystem_notice) {
    rdk_Error ret = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    EXPECT_EQ(rdk_logger_init(conf_file), 0);

    // Prepare arguments for thread
    rdklogctrl_args_t args;
    args.category = CAT_NOTICE;
    args.level = LVL_NOTICE;

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, &args));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Log a message using the high-level API, which will exercise dynamic logger code
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.TESTMOD", "Test message for dynamic logger\n");
    // Clean up
    pthread_join(client_thread, nullptr);
}
TEST(RdkDynamicLoggerTest, MessageProcessingViaSystem_info) {
    rdk_Error ret = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    EXPECT_EQ(rdk_logger_init(conf_file), 0);

    // Prepare arguments for thread
    rdklogctrl_args_t args;
    args.category = CAT_INFO;
    args.level = LVL_INFO;

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, &args));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Log a message using the high-level API, which will exercise dynamic logger code
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.TESTMOD", "Test message for dynamic logger\n");
    // Clean up
    pthread_join(client_thread, nullptr);
}
TEST(RdkDynamicLoggerTest, MessageProcessingViaSystem_debug) {
    rdk_Error ret = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    EXPECT_EQ(rdk_logger_init(conf_file), 0);

    // Prepare arguments for thread
    rdklogctrl_args_t args;
    args.category = CAT_DEBUG;
    args.level = LVL_DEBUG;

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, &args));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Log a message using the high-level API, which will exercise dynamic logger code
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.TESTMOD", "Test message for dynamic logger\n");
    // Clean up
    pthread_join(client_thread, nullptr);
}
TEST(RdkDynamicLoggerTest, MessageProcessingViaSystem_trace) {
    rdk_Error ret = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    EXPECT_EQ(rdk_logger_init(conf_file), 0);

    // Prepare arguments for thread
    rdklogctrl_args_t args;
    args.category = CAT_TRACE;
    args.level = LVL_TRACE;

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, &args));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Log a message using the high-level API, which will exercise dynamic logger code
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.TESTMOD", "Test message for dynamic logger\n");
    // Clean up
    pthread_join(client_thread, nullptr);
}
TEST(RdkDynamicLoggerTest, MessageProcessingViaSystem_none) {
    rdk_Error ret = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    EXPECT_EQ(rdk_logger_init(conf_file), 0);

    // Prepare arguments for thread
    rdklogctrl_args_t args;
    args.category = CAT_NONE;
    args.level = LVL_NONE;

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, &args));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Log a message using the high-level API, which will exercise dynamic logger code
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.TESTMOD", "Test message for dynamic logger\n");
    // Clean up
    pthread_join(client_thread, nullptr);
}

TEST(RdkDynamicLoggerTest, MessageProcessingViaSystem_negnone) {
    rdk_Error ret = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    EXPECT_EQ(rdk_logger_init(conf_file), 0);

    // Prepare arguments for thread
    rdklogctrl_args_t args;
    args.category = CAT_NONE;
    args.level = LVL_TEST;

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, &args));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Log a message using the high-level API, which will exercise dynamic logger code
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.TESTMOD", "Test message for dynamic logger\n");
    // Clean up
    pthread_join(client_thread, nullptr);

}

