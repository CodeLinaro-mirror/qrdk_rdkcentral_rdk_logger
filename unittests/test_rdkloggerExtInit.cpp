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
        test_body; \
        rdk_logger_deinit(); \
        exit(::testing::Test::HasFailure() ? 1 : 0); \
    } else { \
        int status = 0; \
        waitpid(pid, &status, 0); \
        ASSERT_TRUE(WIFEXITED(status)); \
        if (WEXITSTATUS(status) != 0) { \
            FAIL() << "Child process failed with exit code " << WEXITSTATUS(status); \
        } \
    }

TEST_F(RdkLoggerExtInit, CreatesAppenderAndSetsLevel) {
    RUN_IN_FORK({
            rdk_LogOutput_File testPolicy;
            strncpy(testPolicy.fileName, "gtest_rdkunittest.log", sizeof(testPolicy.fileName)-1);
            testPolicy.fileName[sizeof(testPolicy.fileName) - 1] = '\0';
            strncpy(testPolicy.fileLocation, "/tmp", sizeof(testPolicy.fileLocation)-1);
            testPolicy.fileLocation[sizeof(testPolicy.fileLocation) - 1] = '\0';
            testPolicy.fileSizeMax = 1024;
            testPolicy.fileCountMax = 3;
            rdk_logger_ext_config_t cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.loglevel = RDK_LOG_TRACE;
            cfg.appender = RDKLOG_OUTPUT_FILE;
            cfg.layout = RDKLOG_FORMAT_WITH_DATETIME;
            cfg.pFilePolicy = &testPolicy;
            rdk_Error ret = rdk_logger_ext_init(&cfg);
            ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_logger_ext_init failed";


            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", testPolicy.fileLocation, testPolicy.fileName);


            log4c_appender_t* app = log4c_appender_get(fullpath);
            ASSERT_NE(app, nullptr) << "Appender not created by rdk_logger_ext_init: " << fullpath;

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
            memset(&cfg, 0, sizeof(cfg));
            cfg.loglevel = RDK_LOG_DEBUG;
            cfg.appender = RDKLOG_OUTPUT_CONSOLE;
            cfg.layout = RDKLOG_FORMAT_ONLY_TEXT;
            cfg.pFilePolicy = NULL;


            rdk_Error ret = rdk_logger_ext_init(&cfg);
            ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_logger_ext_init failed for Stdout";


            log4c_appender_t* app = log4c_appender_get("stdout");
            ASSERT_NE(app, nullptr) << "Stdout appender not found";

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
            rdk_LogOutput_File testPolicy;
            strncpy(testPolicy.fileName, "gtest_comcast_unittest.log", sizeof(testPolicy.fileName)-1);
            testPolicy.fileName[sizeof(testPolicy.fileName) - 1] = '\0';
            strncpy(testPolicy.fileLocation, "/tmp", sizeof(testPolicy.fileLocation)-1);
            testPolicy.fileLocation[sizeof(testPolicy.fileLocation) - 1] = '\0';
            testPolicy.fileSizeMax = 1024;
            testPolicy.fileCountMax = 2;
            rdk_logger_ext_config_t cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.loglevel = RDK_LOG_ERROR;
            cfg.appender = RDKLOG_OUTPUT_FILE;
            cfg.layout = RDKLOG_FORMAT_WITH_THREADID;
            cfg.pFilePolicy = &testPolicy;

            rdk_Error ret = rdk_logger_ext_init(&cfg);
            ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_logger_ext_init failed for Comcast layout";

            for (int i =0; i < 50; i++)
            {
            RDK_LOG(RDK_LOG_ERROR, "LOG.RDK.RTMESSAGE", "errorloh\n");
            RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.RTMESSAGE", "LOGGING\n");
            RDK_LOG(RDK_LOG_INFO, "LOG.RDK.TEST", "test_LOGGING\n");
            RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.TEST", "DEBUG\n");
            }
    });
}
