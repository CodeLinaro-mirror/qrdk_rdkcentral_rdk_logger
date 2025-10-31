#include <gtest/gtest.h>
#include <cstdio>
#include <log4c.h>

extern "C" {
#include "rdk_debug_priv.h"

}

TEST(RdkDebugPriv, SetDefaultAppenderType) {
    rdk_dbg_priv_init();

    const char* appName = "unit_test_appender_env";
    log4c_appender_t* app = log4c_appender_new(appName);
    ASSERT_NE(app, nullptr) << "Failed to create appender: " << appName;

    set_default_appender_type(app, Stdout);
    const log4c_appender_type_t* type = log4c_appender_get_type(app);
    ASSERT_EQ(type, log4c_appender_type_get("stream_env")) << "Appender type is not stream_env for app: " << appName;

    set_default_appender_type(app, FileOutput);
    ASSERT_EQ(log4c_appender_get_type(app), log4c_appender_type_get("rollingfile")) << "Appender type is not rollingfile for app: " << appName;
}

TEST(RdkDebugPriv, SetDefaultLayout) {
    rdk_dbg_priv_init();

    const char* appName = "unit_test_appender_layout";
    log4c_appender_t* app = log4c_appender_new(appName);
    ASSERT_NE(app, nullptr) << "Failed to create appender for layout test: " << appName;

    set_default_layout(app, LAYOUT_DATED);
    ASSERT_EQ(log4c_appender_get_layout(app), log4c_layout_get("dated")) << "Layout not set to dated for app: " << appName;

    set_default_layout(app, LAYOUT_BASIC);
    ASSERT_EQ(log4c_appender_get_layout(app), log4c_layout_get("basic")) << "Layout not set to basic for app: " << appName;
}

TEST(RdkDebugPriv, SetDefaultLogLevelAndExtInit) {
    const char* catName = "UNIT.TEST.CAT";
    log4c_category_t* cat = log4c_category_get(catName);
    if (!cat) {
        cat = log4c_category_new(catName);
    }
    ASSERT_NE(cat, nullptr) << "Failed to create/obtain category: " << catName;

    set_default_log_level(catName, RDK_LOG_DEBUG);
    EXPECT_EQ(log4c_category_get_priority(cat), LOG4C_PRIORITY_DEBUG) << "Category " << catName << " priority is not DEBUG as expected";

    set_default_log_level(catName, RDK_LOG_ERROR);
    EXPECT_EQ(log4c_category_get_priority(cat), LOG4C_PRIORITY_ERROR) << "Category " << catName << " priority is not ERROR as expected";

    const char* tmpdir = "/tmp";
    const char* logfile = "rdk_unittest.log";

    rdk_logger_ext_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    snprintf(cfg.fileName, sizeof(cfg.fileName), "%s", logfile);
    snprintf(cfg.logdir, sizeof(cfg.logdir), "%s", tmpdir);
    cfg.maxCount = 3;
    cfg.maxSize = 1024;
    cfg.appender_type = FileOutput;
    cfg.loglevel = RDK_LOG_DEBUG;
    cfg.layout = LAYOUT_COMCAST_DATED;

    rdk_Error rc = rdk_logger_ext_init(&cfg);
    ASSERT_EQ(rc, RDK_SUCCESS) << "rdk_logger_ext_init failed with return: " << rc;

    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", tmpdir, logfile);
    log4c_appender_t* app = log4c_appender_get(fullpath);
    ASSERT_NE(app, nullptr) << "Appender not created by rdk_logger_ext_init: " << fullpath;

    EXPECT_EQ(log4c_appender_get_type(app), log4c_appender_type_get("rollingfile")) << "Appender type for " << fullpath << " is not rollingfile";
    EXPECT_EQ(log4c_appender_get_layout(app), log4c_layout_get("comcast_dated")) << "Appender layout for " << fullpath << " is not comcast_dated";

    log4c_category_t* rootCat = log4c_category_get("LOG.RDK");
    ASSERT_NE(rootCat, nullptr) << "Failed to obtain LOG.RDK category";
    EXPECT_EQ(log4c_category_get_priority(rootCat), LOG4C_PRIORITY_DEBUG) << "LOG.RDK category priority is not DEBUG as expected";
}
