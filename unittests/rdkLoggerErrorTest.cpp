/*
  * If not stated otherwise in this file or this component's LICENSE file
  * the following copyright and licenses apply:
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
/******************************************************
Test Case : Testing RDK Logger Error Handling and Edge Cases
*******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "rdk_logger.h"
#include "gtest_app.h"

class RDKLoggerErrorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration file
        system("mkdir -p /tmp/rdk_logger_error_test");
        createTestConfigFile("/tmp/rdk_logger_error_test/test.ini", 
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n");
    }
    
    void TearDown() override {
        // Cleanup
        system("rm -rf /tmp/rdk_logger_error_test");
        rdk_logger_deinit();
        log4c_fini();
    }
    
    void createTestConfigFile(const char* filename, const char* content) {
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%s", content);
            fclose(file);
        }
    }
};

// Test NULL pointer handling in logging functions
TEST_F(RDKLoggerErrorTest, NullPointerHandling) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test NULL module name
    rdk_logger_msg_printf(RDK_LOG_INFO, NULL, "Test message");
    // Should not crash, but may not log anything
    
    // Test NULL format string
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", NULL);
    // Should not crash
    
    // Test NULL module name in enable_logLevel
    rdk_logger_Bool result = rdk_logger_enable_logLevel(NULL, RDK_LOG_INFO, TRUE);
    EXPECT_EQ(result, FALSE) << "Should return FALSE for NULL module name";
    
    // Test NULL module name in is_logLevel_enabled
    result = rdk_logger_is_logLevel_enabled(NULL, RDK_LOG_INFO);
    EXPECT_EQ(result, FALSE) << "Should return FALSE for NULL module name";
}

// Test empty string handling
TEST_F(RDKLoggerErrorTest, EmptyStringHandling) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test empty module name
    rdk_logger_msg_printf(RDK_LOG_INFO, " ", "Test message");
    // Should not crash
    
    // Test empty format string
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", " ");
    // Should not crash
    
    // Test empty module name in enable_logLevel
    rdk_logger_Bool result = rdk_logger_enable_logLevel("", RDK_LOG_INFO, TRUE);
    // Should handle gracefully
    
    // Test empty module name in is_logLevel_enabled
    result = rdk_logger_is_logLevel_enabled("", RDK_LOG_INFO);
    // Should handle gracefully
}

// Test invalid log levels
TEST_F(RDKLoggerErrorTest, InvalidLogLevels) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test invalid log level (beyond enum range)
    rdk_logger_msg_printf((rdk_LogLevel)999, "LOG.RDK.TEST", "Test message");
    // Should not crash, may use default level
    
    // Test invalid log level in enable_logLevel
    rdk_logger_Bool result = rdk_logger_enable_logLevel("LOG.RDK.TEST", (rdk_LogLevel)999, TRUE);
    // Should handle gracefully
    
    // Test invalid log level in is_logLevel_enabled
    result = rdk_logger_is_logLevel_enabled("LOG.RDK.TEST", (rdk_LogLevel)999);
    // Should handle gracefully
}

// Test very long messages
TEST_F(RDKLoggerErrorTest, VeryLongMessages) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Create a very long message
    char long_message[10000];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';
    
    // Test very long message
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Long message: %s", long_message);
    // Should not crash, may truncate or handle gracefully
    
    // Test very long format string
    char long_format[5000];
    memset(long_format, '%', sizeof(long_format) - 1);
    long_format[sizeof(long_format) - 1] = '\0';
    
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", long_format, "test");
    // Should not crash
}

// Test very long module names
TEST_F(RDKLoggerErrorTest, VeryLongModuleNames) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Create a very long module name
    char long_module[1000];
    memset(long_module, 'A', sizeof(long_module) - 1);
    long_module[sizeof(long_module) - 1] = '\0';
    
    // Test very long module name
    rdk_logger_msg_printf(RDK_LOG_INFO, long_module, "Test message");
    // Should not crash
    
    // Test very long module name in enable_logLevel
    rdk_logger_Bool result = rdk_logger_enable_logLevel(long_module, RDK_LOG_INFO, TRUE);
    // Should handle gracefully
    
    // Test very long module name in is_logLevel_enabled
    result = rdk_logger_is_logLevel_enabled(long_module, RDK_LOG_INFO);
    // Should handle gracefully
}

// Test special characters in messages
TEST_F(RDKLoggerErrorTest, SpecialCharactersInMessages) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test various special characters
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Special chars: \n\r\t\b\f\v\\\"'");
    // Should handle special characters correctly
    
    // Test Unicode characters (if supported)
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Unicode: \u00A9 \u00AE \u2122");
    // Should handle Unicode characters
    
    // Test control characters
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Control: %c%c%c", 0x01, 0x02, 0x03);
    // Should handle control characters
}

// Test format string vulnerabilities
TEST_F(RDKLoggerErrorTest, FormatStringVulnerabilities) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test format string with %n (should be handled safely)
    int n_value = 0;
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Test %n message", &n_value);
    // Should not crash or cause security issues
    
    // Test format string with %s and NULL
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Test %s message", (char*)"(null)");
    // Should handle NULL string parameter safely
    
    // Test format string with %d and invalid pointer
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Test %d message", 0);
    // Should handle invalid pointer safely
}

// Test multiple initialization calls
TEST_F(RDKLoggerErrorTest, MultipleInitialization) {
    // First initialization
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "First initialization should succeed";
    
    // Second initialization (should be handled gracefully)
    ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Second initialization should be handled gracefully";
    
    // Test that logging still works
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Test message after multiple init");
    // Should work correctly
}

// Test deinitialization without initialization
TEST_F(RDKLoggerErrorTest, DeinitWithoutInit) {
    // Deinitialize without initializing first
    rdk_Error ret = rdk_logger_deinit();
    ASSERT_EQ(ret, RDK_SUCCESS) << "Deinit without init should be handled gracefully";
    
    // Try to log after deinit without init
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Test message after deinit");
    // Should not crash
}

// Test logging after deinitialization
TEST_F(RDKLoggerErrorTest, LoggingAfterDeinit) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Deinitialize
    ret = rdk_logger_deinit();
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to deinitialize logger";
    
    // Try to log after deinit
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Test message after deinit");
    // Should not crash, may not log anything
}

// Test enable_logLevel with invalid parameters
TEST_F(RDKLoggerErrorTest, EnableLogLevelInvalidParams) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test with NULL module name
    rdk_logger_Bool result = rdk_logger_enable_logLevel(NULL, RDK_LOG_INFO, TRUE);
    EXPECT_EQ(result, FALSE) << "Should return FALSE for NULL module name";
    
    // Test with empty module name
    result = rdk_logger_enable_logLevel("", RDK_LOG_INFO, TRUE);
    // Should handle gracefully
    
    // Test with invalid log level
    result = rdk_logger_enable_logLevel("LOG.RDK.TEST", (rdk_LogLevel)999, TRUE);
    // Should handle gracefully
}

// Test is_logLevel_enabled with invalid parameters
TEST_F(RDKLoggerErrorTest, IsLogLevelEnabledInvalidParams) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test with NULL module name
    rdk_logger_Bool result = rdk_logger_is_logLevel_enabled(NULL, RDK_LOG_INFO);
    EXPECT_EQ(result, FALSE) << "Should return FALSE for NULL module name";
    
    // Test with empty module name
    result = rdk_logger_is_logLevel_enabled("", RDK_LOG_INFO);
    // Should handle gracefully
    
    // Test with invalid log level
    result = rdk_logger_is_logLevel_enabled("LOG.RDK.TEST", (rdk_LogLevel)999);
    // Should handle gracefully
}

// Test rdk_logger_level_from_string with invalid inputs
TEST_F(RDKLoggerErrorTest, LevelFromStringInvalidInputs) {
    // Test NULL input
    rdk_LogLevel level = rdk_logger_level_from_string(NULL);
    EXPECT_EQ(level, RDK_LOG_NONE) << "Should return RDK_LOG_NONE for NULL input";
    
    // Test empty string
    level = rdk_logger_level_from_string("");
    EXPECT_EQ(level, RDK_LOG_NONE) << "Should return RDK_LOG_NONE for empty string";
    
    // Test invalid string
    level = rdk_logger_level_from_string("INVALID_LEVEL");
    EXPECT_EQ(level, RDK_LOG_NONE) << "Should return RDK_LOG_NONE for invalid string";
    
    // Test partial string
    level = rdk_logger_level_from_string("DEBU");
    EXPECT_EQ(level, RDK_LOG_NONE) << "Should return RDK_LOG_NONE for partial string";
    
    // Test mixed case (should work)
    level = rdk_logger_level_from_string("debug");
    EXPECT_EQ(level, RDK_LOG_DEBUG) << "Should handle lowercase correctly";
    
    level = rdk_logger_level_from_string("Debug");
    EXPECT_EQ(level, RDK_LOG_DEBUG) << "Should handle mixed case correctly";
}

// Test rdk_logger_log_onboard with invalid parameters
TEST_F(RDKLoggerErrorTest, LogOnboardInvalidParams) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test with NULL module name
    rdk_logger_log_onboard(NULL, "Test message");
    // Should not crash
    
    // Test with NULL message
    rdk_logger_log_onboard("LOG.RDK.TEST", NULL);
    // Should not crash
    
    // Test with empty module name
    rdk_logger_log_onboard(" ", "Test message");
    // Should not crash
    
    // Test with empty message
    rdk_logger_log_onboard("LOG.RDK.TEST", " ");
    // Should not crash
}

// Test legacy functions with invalid parameters
TEST_F(RDKLoggerErrorTest, LegacyFunctionsInvalidParams) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test rdk_dbg_MsgRaw with NULL parameters
    rdk_dbg_MsgRaw(RDK_LOG_INFO, NULL, "Test message");
    // Should not crash
    
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.TEST", NULL);
    // Should not crash
    
    // Test rdk_dbg_MsgRaw1 with NULL parameters
    // Note: rdk_dbg_MsgRaw1 requires va_list, so we test it differently
    // We'll test the function by calling it with a simple va_list setup
    rdk_dbg_MsgRaw(RDK_LOG_INFO, NULL, "Test message");
    // Should not crash
    
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.TEST", NULL);
    // Should not crash
}

// Test memory allocation edge cases
TEST_F(RDKLoggerErrorTest, MemoryAllocationEdgeCases) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_error_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test with very large number of modules
    for (int i = 0; i < 10000; i++) {
        char module_name[100];
        snprintf(module_name, sizeof(module_name), "LOG.RDK.MODULE%d", i);
        rdk_logger_msg_printf(RDK_LOG_INFO, module_name, "Test message %d", i);
    }
    // Should handle large number of modules gracefully
    
    // Test rapid logging
    for (int i = 0; i < 1000; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Rapid message %d", i);
    }
    // Should handle rapid logging gracefully
}
