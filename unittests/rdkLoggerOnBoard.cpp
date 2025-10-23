#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <cstdio>
#include <cstring>
#include "rdk_logger.h"
#include "gtest_app.h"


// Define test-specific paths
#define TEST_ONBOARDING_LOG_FILE "test_onboarding.log"
#define TEST_DEVICE_ONBOARDED "test_device_onboarded"
#define TEST_DISABLE_ONBOARDING "test_disable_onboarding"

// You might need to redefine these in your test build
#define ONBOARDING_LOG_FILE TEST_ONBOARDING_LOG_FILE
#define DEVICE_ONBOARDED TEST_DEVICE_ONBOARDED
#define DISABLE_ONBOARDING TEST_DISABLE_ONBOARDING

TEST(rdkLoggerOnBoard, WritesLogToFileWhenConditionsAreMet) {
    // Ensure condition files do not exist
    remove(TEST_DEVICE_ONBOARDED);
    remove(TEST_DISABLE_ONBOARDING);
    remove(TEST_ONBOARDING_LOG_FILE);

    // Call the function
    rdk_logger_log_onboard("TestModule", "Test message %d", 123);

    // Read the log file
    std::ifstream logFile(TEST_ONBOARDING_LOG_FILE);
    ASSERT_TRUE(logFile.is_open());

    std::string content((std::istreambuf_iterator<char>(logFile)),
                         std::istreambuf_iterator<char>());

    logFile.close();

    // Check if expected content is present
    EXPECT_NE(content.find("TestModule"), std::string::npos);
    EXPECT_NE(content.find("Test message 123"), std::string::npos);

    // Cleanup
    remove(TEST_ONBOARDING_LOG_FILE);
}
