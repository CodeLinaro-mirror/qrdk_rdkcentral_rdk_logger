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
Test Case : Testing RDK Logger Utility Functions and Edge Cases
*******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include "rdk_logger.h"
#include "gtest_app.h"

class RDKLoggerUtilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        system("mkdir -p /tmp/rdk_logger_utility_test");
        
        // Create test configuration file
        createTestConfigFile("/tmp/rdk_logger_utility_test/test.ini", 
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.UTILITY=DEBUG\n"
            "LOG.RDK.ONBOARD=WARNING\n");
    }
    
    void TearDown() override {
        // Cleanup
        system("rm -rf /tmp/rdk_logger_utility_test");
        rdk_logger_deinit();
    }
    
    void createTestConfigFile(const char* filename, const char* content) {
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%s", content);
            fclose(file);
        }
    }
};

// Test rdk_logger_level_from_string with all valid levels
TEST_F(RDKLoggerUtilityTest, LevelFromStringValidLevels) {
    // Test all valid log levels
    EXPECT_EQ(rdk_logger_level_from_string("FATAL"), RDK_LOG_FATAL);
    EXPECT_EQ(rdk_logger_level_from_string("ERROR"), RDK_LOG_ERROR);
    EXPECT_EQ(rdk_logger_level_from_string("WARNING"), RDK_LOG_WARN);
    EXPECT_EQ(rdk_logger_level_from_string("WARN"), RDK_LOG_WARN);
    EXPECT_EQ(rdk_logger_level_from_string("NOTICE"), RDK_LOG_NOTICE);
    EXPECT_EQ(rdk_logger_level_from_string("INFO"), RDK_LOG_INFO);
    EXPECT_EQ(rdk_logger_level_from_string("DEBUG"), RDK_LOG_DEBUG);
    EXPECT_EQ(rdk_logger_level_from_string("TRACE"), RDK_LOG_TRACE);
    EXPECT_EQ(rdk_logger_level_from_string("NONE"), RDK_LOG_NONE);
}

// Test rdk_logger_level_from_string with case variations
TEST_F(RDKLoggerUtilityTest, LevelFromStringCaseVariations) {
    // Test lowercase
    EXPECT_EQ(rdk_logger_level_from_string("fatal"), RDK_LOG_FATAL);
    EXPECT_EQ(rdk_logger_level_from_string("error"), RDK_LOG_ERROR);
    EXPECT_EQ(rdk_logger_level_from_string("warning"), RDK_LOG_WARN);
    EXPECT_EQ(rdk_logger_level_from_string("warn"), RDK_LOG_WARN);
    EXPECT_EQ(rdk_logger_level_from_string("notice"), RDK_LOG_NOTICE);
    EXPECT_EQ(rdk_logger_level_from_string("info"), RDK_LOG_INFO);
    EXPECT_EQ(rdk_logger_level_from_string("debug"), RDK_LOG_DEBUG);
    EXPECT_EQ(rdk_logger_level_from_string("trace"), RDK_LOG_TRACE);
    EXPECT_EQ(rdk_logger_level_from_string("none"), RDK_LOG_NONE);
    
    // Test mixed case
    EXPECT_EQ(rdk_logger_level_from_string("Fatal"), RDK_LOG_FATAL);
    EXPECT_EQ(rdk_logger_level_from_string("Error"), RDK_LOG_ERROR);
    EXPECT_EQ(rdk_logger_level_from_string("Warning"), RDK_LOG_WARN);
    EXPECT_EQ(rdk_logger_level_from_string("Warn"), RDK_LOG_WARN);
    EXPECT_EQ(rdk_logger_level_from_string("Notice"), RDK_LOG_NOTICE);
    EXPECT_EQ(rdk_logger_level_from_string("Info"), RDK_LOG_INFO);
    EXPECT_EQ(rdk_logger_level_from_string("Debug"), RDK_LOG_DEBUG);
    EXPECT_EQ(rdk_logger_level_from_string("Trace"), RDK_LOG_TRACE);
    EXPECT_EQ(rdk_logger_level_from_string("None"), RDK_LOG_NONE);
}

// Test rdk_logger_level_from_string with invalid inputs
TEST_F(RDKLoggerUtilityTest, LevelFromStringInvalidInputs) {
    // Test NULL input
    EXPECT_EQ(rdk_logger_level_from_string(NULL), RDK_LOG_NONE);
    
    // Test empty string
    EXPECT_EQ(rdk_logger_level_from_string(""), RDK_LOG_NONE);
    
    // Test invalid strings
    EXPECT_EQ(rdk_logger_level_from_string("INVALID"), RDK_LOG_NONE);
    EXPECT_EQ(rdk_logger_level_from_string("UNKNOWN"), RDK_LOG_NONE);
    EXPECT_EQ(rdk_logger_level_from_string("123"), RDK_LOG_NONE);
    EXPECT_EQ(rdk_logger_level_from_string("DEBUGGING"), RDK_LOG_NONE);
    EXPECT_EQ(rdk_logger_level_from_string("ERRORS"), RDK_LOG_NONE);
    
    // Test partial strings
    EXPECT_EQ(rdk_logger_level_from_string("DEBU"), RDK_LOG_NONE);
    EXPECT_EQ(rdk_logger_level_from_string("ERRO"), RDK_LOG_NONE);
    EXPECT_EQ(rdk_logger_level_from_string("WARNI"), RDK_LOG_NONE);
    
    // Test strings with extra characters
    EXPECT_EQ(rdk_logger_level_from_string("DEBUG "), RDK_LOG_NONE);
    EXPECT_EQ(rdk_logger_level_from_string(" DEBUG"), RDK_LOG_NONE);
    EXPECT_EQ(rdk_logger_level_from_string("DEBUG_EXTRA"), RDK_LOG_NONE);
}

// Test rdk_logger_log_onboard functionality
TEST_F(RDKLoggerUtilityTest, LogOnboardFunctionality) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test onboard logging
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Test onboard message");
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Onboard message with format: %d", 123);
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Onboard message with multiple args: %s %d %f", "test", 456, 3.14);
    
    // Should work correctly
}

// Test rdk_logger_log_onboard with different log levels
TEST_F(RDKLoggerUtilityTest, LogOnboardDifferentLevels) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test onboard logging with different message types
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Fatal onboard message");
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Error onboard message");
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Warning onboard message");
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Notice onboard message");
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Info onboard message");
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Debug onboard message");
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Trace onboard message");
    
    // Should work correctly
}

// Test rdk_logger_log_onboard with special characters
TEST_F(RDKLoggerUtilityTest, LogOnboardSpecialCharacters) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test onboard logging with special characters
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Special chars: \n\r\t\b\f\v\\\"'");
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Unicode: \u00A9 \u00AE \u2122");
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Control: %c%c%c", 0x01, 0x02, 0x03);
    
    // Should handle special characters correctly
}

// Test rdk_logger_log_onboard with very long messages
TEST_F(RDKLoggerUtilityTest, LogOnboardLongMessages) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Create a very long message
    char long_message[5000];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';
    
    // Test onboard logging with long message
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Long message: %s", long_message);
    
    // Should handle long messages correctly
}

// Test rdk_logger_log_onboard with format string vulnerabilities
TEST_F(RDKLoggerUtilityTest, LogOnboardFormatStringVulnerabilities) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test format string with %n (should be handled safely)
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Test %n message");
    
    // Test format string with %s and NULL
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Test %s message", (char*)NULL);
    
    // Test format string with %d and invalid pointer
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Test %d message", (int*)NULL);
    
    // Should handle format string vulnerabilities safely
}

// Test legacy functions rdk_dbg_MsgRaw and rdk_dbg_MsgRaw1
TEST_F(RDKLoggerUtilityTest, LegacyFunctions) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test rdk_dbg_MsgRaw
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Legacy MsgRaw test");
    rdk_dbg_MsgRaw(RDK_LOG_DEBUG, "LOG.RDK.UTILITY", "Legacy MsgRaw with format: %d", 123);
    rdk_dbg_MsgRaw(RDK_LOG_ERROR, "LOG.RDK.UTILITY", "Legacy MsgRaw with multiple args: %s %d", "test", 456);
    
    // Test rdk_dbg_MsgRaw1 with va_list
    va_list args;
    va_start(args, "Legacy MsgRaw1 with va_list: %d %s");
    rdk_dbg_MsgRaw1(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Legacy MsgRaw1 with va_list: %d %s", args);
    va_end(args);
    
    // Should work correctly
}

// Test legacy functions with different log levels
TEST_F(RDKLoggerUtilityTest, LegacyFunctionsDifferentLevels) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test all log levels with legacy functions
    rdk_dbg_MsgRaw(RDK_LOG_FATAL, "LOG.RDK.UTILITY", "Legacy fatal message");
    rdk_dbg_MsgRaw(RDK_LOG_ERROR, "LOG.RDK.UTILITY", "Legacy error message");
    rdk_dbg_MsgRaw(RDK_LOG_WARN, "LOG.RDK.UTILITY", "Legacy warning message");
    rdk_dbg_MsgRaw(RDK_LOG_NOTICE, "LOG.RDK.UTILITY", "Legacy notice message");
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Legacy info message");
    rdk_dbg_MsgRaw(RDK_LOG_DEBUG, "LOG.RDK.UTILITY", "Legacy debug message");
    rdk_dbg_MsgRaw(RDK_LOG_TRACE, "LOG.RDK.UTILITY", "Legacy trace message");
    
    // Should work correctly
}

// Test legacy functions with special characters
TEST_F(RDKLoggerUtilityTest, LegacyFunctionsSpecialCharacters) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test legacy functions with special characters
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Legacy special chars: \n\r\t\b\f\v\\\"'");
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Legacy unicode: \u00A9 \u00AE \u2122");
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Legacy control: %c%c%c", 0x01, 0x02, 0x03);
    
    // Should handle special characters correctly
}

// Test legacy functions with very long messages
TEST_F(RDKLoggerUtilityTest, LegacyFunctionsLongMessages) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Create a very long message
    char long_message[5000];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';
    
    // Test legacy functions with long message
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Legacy long message: %s", long_message);
    
    // Should handle long messages correctly
}

// Test legacy functions with format string vulnerabilities
TEST_F(RDKLoggerUtilityTest, LegacyFunctionsFormatStringVulnerabilities) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test format string with %n (should be handled safely)
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Legacy test %n message");
    
    // Test format string with %s and NULL
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Legacy test %s message", (char*)NULL);
    
    // Test format string with %d and invalid pointer
    rdk_dbg_MsgRaw(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Legacy test %d message", (int*)NULL);
    
    // Should handle format string vulnerabilities safely
}

// Test rdk_logger_msg_vsprintf functionality
TEST_F(RDKLoggerUtilityTest, MsgVsprintfFunctionality) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test rdk_logger_msg_vsprintf
    va_list args;
    va_start(args, "Vsprintf test with format: %d %s %f");
    rdk_logger_msg_vsprintf(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Vsprintf test with format: %d %s %f", args);
    va_end(args);
    
    // Test with different log levels
    va_start(args, "Vsprintf error message");
    rdk_logger_msg_vsprintf(RDK_LOG_ERROR, "LOG.RDK.UTILITY", "Vsprintf error message", args);
    va_end(args);
    
    va_start(args, "Vsprintf debug message");
    rdk_logger_msg_vsprintf(RDK_LOG_DEBUG, "LOG.RDK.UTILITY", "Vsprintf debug message", args);
    va_end(args);
    
    // Should work correctly
}

// Test rdk_logger_msg_vsprintf with special characters
TEST_F(RDKLoggerUtilityTest, MsgVsprintfSpecialCharacters) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test vsprintf with special characters
    va_list args;
    va_start(args, "Vsprintf special chars: \n\r\t\b\f\v\\\"'");
    rdk_logger_msg_vsprintf(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Vsprintf special chars: \n\r\t\b\f\v\\\"'", args);
    va_end(args);
    
    // Should handle special characters correctly
}

// Test rdk_logger_msg_vsprintf with very long messages
TEST_F(RDKLoggerUtilityTest, MsgVsprintfLongMessages) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Create a very long message
    char long_message[5000];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';
    
    // Test vsprintf with long message
    va_list args;
    va_start(args, long_message);
    rdk_logger_msg_vsprintf(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Vsprintf long message: %s", args);
    va_end(args);
    
    // Should handle long messages correctly
}

// Test rdk_logger_msg_vsprintf with format string vulnerabilities
TEST_F(RDKLoggerUtilityTest, MsgVsprintfFormatStringVulnerabilities) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test format string with %n (should be handled safely)
    va_list args;
    va_start(args, "Vsprintf test %n message");
    rdk_logger_msg_vsprintf(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Vsprintf test %n message", args);
    va_end(args);
    
    // Test format string with %s and NULL
    va_start(args, (char*)NULL);
    rdk_logger_msg_vsprintf(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Vsprintf test %s message", args);
    va_end(args);
    
    // Should handle format string vulnerabilities safely
}

// Test rdk_logger_msg_vsprintf with invalid parameters
TEST_F(RDKLoggerUtilityTest, MsgVsprintfInvalidParameters) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test with NULL module name
    va_list args;
    va_start(args, "Test message");
    rdk_logger_msg_vsprintf(RDK_LOG_INFO, NULL, "Test message", args);
    va_end(args);
    
    // Test with NULL format string
    va_start(args, NULL);
    rdk_logger_msg_vsprintf(RDK_LOG_INFO, "LOG.RDK.UTILITY", NULL, args);
    va_end(args);
    
    // Test with empty module name
    va_start(args, "Test message");
    rdk_logger_msg_vsprintf(RDK_LOG_INFO, "", "Test message", args);
    va_end(args);
    
    // Test with empty format string
    va_start(args, "");
    rdk_logger_msg_vsprintf(RDK_LOG_INFO, "LOG.RDK.UTILITY", "", args);
    va_end(args);
    
    // Should handle invalid parameters gracefully
}

// Test rdk_logger_msg_vsprintf with invalid log levels
TEST_F(RDKLoggerUtilityTest, MsgVsprintfInvalidLogLevels) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test with invalid log level
    va_list args;
    va_start(args, "Test message");
    rdk_logger_msg_vsprintf((rdk_LogLevel)999, "LOG.RDK.UTILITY", "Test message", args);
    va_end(args);
    
    // Should handle invalid log levels gracefully
}

// Test rdk_logger_msg_vsprintf with very long module names
TEST_F(RDKLoggerUtilityTest, MsgVsprintfLongModuleNames) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Create a very long module name
    char long_module[1000];
    memset(long_module, 'A', sizeof(long_module) - 1);
    long_module[sizeof(long_module) - 1] = '\0';
    
    // Test vsprintf with long module name
    va_list args;
    va_start(args, "Test message");
    rdk_logger_msg_vsprintf(RDK_LOG_INFO, long_module, "Test message", args);
    va_end(args);
    
    // Should handle long module names correctly
}

// Test rdk_logger_msg_vsprintf with rapid calls
TEST_F(RDKLoggerUtilityTest, MsgVsprintfRapidCalls) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_utility_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test rapid vsprintf calls
    for (int i = 0; i < 100; i++) {
        va_list args;
        va_start(args, i);
        rdk_logger_msg_vsprintf(RDK_LOG_INFO, "LOG.RDK.UTILITY", "Rapid vsprintf call %d", args);
        va_end(args);
    }
    
    // Should handle rapid calls gracefully
}
