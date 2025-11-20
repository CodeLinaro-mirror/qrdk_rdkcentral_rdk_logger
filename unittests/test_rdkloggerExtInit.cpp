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
        rdk_logger_ext_config_t cfg;
        memset(&cfg, 0, sizeof(cfg));
        strncpy(cfg.fileName, "gtest_rdk_unittest.log", sizeof(cfg.fileName)-1);
        cfg.fileName[sizeof(cfg.fileName)-1] = '\0';
        strncpy(cfg.logdir, "/tmp", sizeof(cfg.logdir)-1);
        cfg.logdir[sizeof(cfg.logdir)-1] = '\0';
        cfg.maxCount = 3;
        cfg.maxSize = 1024;
        cfg.appender_type = FileOutput;
        cfg.loglevel = RDK_LOG_TRACE;
        cfg.layout = LAYOUT_DATED;


        rdk_Error ret = rdk_logger_ext_init(&cfg);
        ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_logger_ext_init failed";


        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", cfg.logdir, cfg.fileName);


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
        memset(&cfg, 0, sizeof(cfg));
        strncpy(cfg.fileName, "unused.txt", sizeof(cfg.fileName)-1);
        cfg.fileName[sizeof(cfg.fileName)-1] = '\0';
        strncpy(cfg.logdir, "/tmp", sizeof(cfg.logdir)-1);
        cfg.logdir[sizeof(cfg.logdir)-1] = '\0';
        cfg.maxCount = 1;
        cfg.maxSize = 512;
        cfg.appender_type = Stdout;
        cfg.loglevel = RDK_LOG_DEBUG;
        cfg.layout = LAYOUT_BASIC;


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
        rdk_logger_ext_config_t cfg;
        memset(&cfg, 0, sizeof(cfg));
        strncpy(cfg.fileName, "gtest_comcast_unittest.log", sizeof(cfg.fileName)-1);
        cfg.fileName[sizeof(cfg.fileName)-1] = '\0';
        strncpy(cfg.logdir, "/tmp", sizeof(cfg.logdir)-1);
        cfg.logdir[sizeof(cfg.logdir)-1] = '\0';
        cfg.maxCount = 2;
        cfg.maxSize = 1024;
        cfg.appender_type = FileOutput;
        cfg.loglevel = RDK_LOG_ERROR;
        cfg.layout = LAYOUT_COMCAST_DATED;


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
