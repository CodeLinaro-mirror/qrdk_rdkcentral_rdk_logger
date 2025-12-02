#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <gtest/gtest.h>
#include "rdk_logger.h"
#include "rdk_error.h"
#include "log4c.h"


class RdkLoggerExtInit : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
};


#define RUN_IN_FORK(test_body) \
    pid_t pid = fork(); \
    ASSERT_NE(pid, -1) << "fork failed"; \
    if (pid == 0) { \
        log4c_init(); \
        test_body; \
        rdk_logger_deinit(); \
        exit(0); \
    } else { \
        int status = 0; \
        waitpid(pid, &status, 0); \
        ASSERT_TRUE(WIFEXITED(status)); \
        ASSERT_EQ(WEXITSTATUS(status), 0); \
    }


TEST_F(RdkLoggerExtInit, CreatesAppenderAndSetsLevel) {
    RUN_IN_FORK({
            rdk_LogFilePolicy testPolicy;
            strncpy(testPolicy.fileName, "gtest_rdkunittest.log", sizeof(testPolicy.fileName)-1);
            strncpy(testPolicy.logdir, "/tmp", sizeof(testPolicy.logdir)-1);
            testPolicy.maxBytesPerFile = 1024;
            testPolicy.maxRotationCount = 3;
            rdk_logger_ext_config_t cfg;
            cfg.loglevel = RDK_LOG_TRACE;
            cfg.appender = RDK_LOG_OUTPUT_FILE;
            cfg.layout = RDK_LOG_LAYOUT_TIMESTAMPED;
            cfg.pFilePolicy = &testPolicy;
            rdk_Error ret = rdk_logger_ext_init(&cfg);
            ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_logger_ext_init failed";


            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", testPolicy.logdir, testPolicy.fileName);


            log4c_appender_t* app = log4c_appender_get(fullpath);
            ASSERT_NE(app, nullptr) << "Appender not created by rdk_logger_ext_init: " << fullpath;


            EXPECT_EQ(log4c_appender_get_type(app), log4c_appender_type_get("rollingfile")) << "Appender type mismatch";
            EXPECT_EQ(log4c_appender_get_layout(app), log4c_layout_get("dated")) << "Appender layout mismatch";


            log4c_category_t* rootCat = log4c_category_get("LOG.RDK");
            ASSERT_NE(rootCat, nullptr) << "Failed to obtain LOG.RDK category";
            EXPECT_EQ(log4c_category_get_priority(rootCat), LOG4C_PRIORITY_TRACE) << "LOG.RDK priority not TRACE";
            for (int i =0; i < 50; i++)
            {
                RDK_LOG(RDK_LOG_TRACE, "LOG.RDK.RTMESSAGE", "errorloh\n");
                RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.RTMESSAGE", "LOGGING\n");
                RDK_LOG(RDK_LOG_WARN, "LOG.RDK.TEST", "test_LOGGING\n");
                RDK_LOG(RDK_LOG_INFO, "LOG.RDK.TEST", "DEBUG\n");
            }
    });
}


TEST_F(RdkLoggerExtInit, StdoutAppenderAndLayout) {
    RUN_IN_FORK({
            rdk_logger_ext_config_t cfg;
            cfg.loglevel = RDK_LOG_DEBUG;
            cfg.appender = RDK_LOG_OUTPUT_STDOUT;
            cfg.layout = RDK_LOG_LAYOUT_PLAINTEXT;
            cfg.pFilePolicy = NULL;


            rdk_Error ret = rdk_logger_ext_init(&cfg);
            ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_logger_ext_init failed for Stdout";


            log4c_appender_t* app = log4c_appender_get("stdout");
            ASSERT_NE(app, nullptr) << "Stdout appender not found";


            log4c_category_t* rootCat = log4c_category_get("LOG.RDK");
            EXPECT_EQ(log4c_appender_get_type(app), log4c_appender_type_get("stream_env")) << "Stdout appender type mismatch";
            EXPECT_EQ(log4c_category_get_priority(rootCat), LOG4C_PRIORITY_DEBUG) << "LOG.RDK priority not DEBUG";


            EXPECT_EQ(log4c_appender_get_layout(app), log4c_layout_get("basic")) << "Stdout appender layout mismatch";
            for (int i =0; i < 50; i++)
            {
                RDK_LOG(RDK_LOG_ERROR, "LOG.RDK.RTMESSAGE", "errorloh\n");
                RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.RTMESSAGE", "LOGGING\n");
                RDK_LOG(RDK_LOG_INFO, "LOG.RDK.TEST", "test_LOGGING\n");
                RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.TEST", "DEBUG\n");
            }
    });
}


TEST_F(RdkLoggerExtInit, ComcastDatedViaExtInit) {
    RUN_IN_FORK({
            rdk_LogFilePolicy testPolicy;
            strncpy(testPolicy.fileName, "gtest_comcast_unittest.log", sizeof(testPolicy.fileName)-1);
            strncpy(testPolicy.logdir, "/tmp", sizeof(testPolicy.logdir)-1);
            testPolicy.maxBytesPerFile = 1024;
            testPolicy.maxRotationCount = 2;
            rdk_logger_ext_config_t cfg;
            cfg.loglevel = RDK_LOG_ERROR;
            cfg.appender = RDK_LOG_OUTPUT_FILE;
            cfg.layout = RDK_LOG_LAYOUT_COMCAST;
            cfg.pFilePolicy = &testPolicy;

            rdk_Error ret = rdk_logger_ext_init(&cfg);
            ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_logger_ext_init failed for Comcast layout";


            log4c_category_t* rootCat = log4c_category_get("LOG.RDK");
            EXPECT_EQ(log4c_category_get_priority(rootCat), LOG4C_PRIORITY_ERROR) << "LOG.RDK priority not ERROR";
            for (int i =0; i < 50; i++)
            {
            RDK_LOG(RDK_LOG_ERROR, "LOG.RDK.RTMESSAGE", "errorloh\n");
            RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.RTMESSAGE", "LOGGING\n");
            RDK_LOG(RDK_LOG_INFO, "LOG.RDK.TEST", "test_LOGGING\n");
            RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.TEST", "DEBUG\n");
            }
    });
}
