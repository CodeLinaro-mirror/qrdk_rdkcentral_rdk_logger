#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <gtest/gtest.h>
#include "rdk_logger.h"
#include "rdk_error.h"
#include "log4c.h"

TEST(RdkLoggerExtInit, CreatesAppenderAndSetsLevel) {
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
    //rdk_log_exit();
    rdk_logger_deinit();
}

TEST(RdkLoggerExtInit, StdoutAppenderAndLayout) {
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
    //rdk_log_exit();
    rdk_logger_deinit();
}

TEST(RdkLoggerExtInit, ComcastDatedViaExtInit) {
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
    cfg.layout = LAYOUT_COMCAST_DATED; /* request comcast_dated layout via public API */

    rdk_Error ret = rdk_logger_ext_init(&cfg);
    ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_logger_ext_init failed for Comcast layout";

    /*char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", cfg.logdir, cfg.fileName);
    log4c_category_t* rootCat = log4c_category_get("LOG.RDK");

    //log4c_appender_t* app = log4c_appender_get(fullpath);
    //ASSERT_NE(app, nullptr) << "Appender not created by rdk_logger_ext_init: " << fullpath;
    EXPECT_EQ(log4c_category_get_priority(rootCat), LOG4C_PRIORITY_ERROR) << "LOG.RDK priority not ERROR";
    EXPECT_EQ(log4c_appender_get_layout(app), log4c_layout_get("comcast_dated")) << "Appender layout is not comcast_dated as requested";*/
    for (int i =0; i < 50; i++)
    {
         RDK_LOG(RDK_LOG_ERROR, "LOG.RDK.RTMESSAGE", "errorloh\n");
         RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.RTMESSAGE", "LOGGING\n");
         RDK_LOG(RDK_LOG_INFO, "LOG.RDK.TEST", "test_LOGGING\n");
         RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.TEST", "DEBUG\n");
    }
    //rdk_log_exit();
    rdk_logger_deinit();
}
