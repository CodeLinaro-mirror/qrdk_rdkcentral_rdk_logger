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
Test Case : Testing RDK Logger Configuration Management
*******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "rdk_logger.h"
#include "gtest_app.h"

class RDKLoggerConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test files
        system("mkdir -p /tmp/rdk_logger_test");
        
        // Create test configuration files
        createTestConfigFile("/tmp/rdk_logger_test/valid.ini", 
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.ERROR=ERROR\n");
            
        createTestConfigFile("/tmp/rdk_logger_test/empty.ini", "");
        
        createTestConfigFile("/tmp/rdk_logger_test/malformed.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "INVALID_LINE_WITHOUT_EQUALS\n"
            "LOG.RDK.TEST=DEBUG\n");
            
        createTestConfigFile("/tmp/rdk_logger_test/comments.ini",
            "# This is a comment\n"
            "LOG.RDK.DEFAULT=WARNING\n"
            "# Another comment\n"
            "LOG.RDK.TEST=DEBUG\n");
            
        createTestConfigFile("/tmp/rdk_logger_test/whitespace.ini",
            "LOG.RDK.DEFAULT = INFO \n"
            " LOG.RDK.TEST = DEBUG \n"
            "LOG.RDK.ERROR=ERROR\n");
    }
    
    void TearDown() override {
        // Cleanup test files
        system("rm -rf /tmp/rdk_logger_test");
        
        // Reset logger state
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

// Test valid configuration file parsing
TEST_F(RDKLoggerConfigTest, ValidConfigFileParsing) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/valid.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize with valid config file";
    
    // Test that configuration was parsed correctly
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.TEST", RDK_LOG_DEBUG);
    EXPECT_EQ(enabled, TRUE) << "DEBUG level should be enabled for LOG.RDK.TEST";
    
    enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.ERROR", RDK_LOG_ERROR);
    EXPECT_EQ(enabled, TRUE) << "ERROR level should be enabled for LOG.RDK.ERROR";
}

// Test empty configuration file
TEST_F(RDKLoggerConfigTest, EmptyConfigFile) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/empty.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle empty config file gracefully";
}

// Test malformed configuration file
TEST_F(RDKLoggerConfigTest, MalformedConfigFile) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/malformed.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle malformed config file gracefully";
    
    // Should still parse valid lines
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.TEST", RDK_LOG_DEBUG);
    EXPECT_EQ(enabled, TRUE) << "Valid lines should still be parsed";
}

// Test configuration file with comments
TEST_F(RDKLoggerConfigTest, ConfigFileWithComments) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/comments.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle config file with comments";
    
    // Comments should be ignored
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.TEST", RDK_LOG_DEBUG);
    EXPECT_EQ(enabled, TRUE) << "Comments should be ignored";
}

// Test configuration file with whitespace
TEST_F(RDKLoggerConfigTest, ConfigFileWithWhitespace) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/whitespace.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle config file with whitespace";
    
    // Whitespace should be trimmed
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.TEST", RDK_LOG_DEBUG);
    EXPECT_EQ(enabled, TRUE) << "Whitespace should be trimmed";
}

// Test non-existent configuration file
TEST_F(RDKLoggerConfigTest, NonExistentConfigFile) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/nonexistent.ini");
    ASSERT_NE(ret, RDK_SUCCESS) << "Should fail with non-existent config file";
}

// Test NULL configuration file path
TEST_F(RDKLoggerConfigTest, NullConfigFile) {
    rdk_Error ret = rdk_logger_init(NULL);
    // Should use default configuration file
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle NULL config file path";
}

// Test very large configuration file
TEST_F(RDKLoggerConfigTest, LargeConfigFile) {
    // Create a large configuration file
    FILE* file = fopen("/tmp/rdk_logger_test/large.ini", "w");
    ASSERT_TRUE(file != NULL) << "Failed to create large config file";
    
    // Write many configuration entries
    for (int i = 0; i < 1000; i++) {
        fprintf(file, "LOG.RDK.MODULE%d=DEBUG\n", i);
    }
    fclose(file);
    
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/large.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle large config file";
    
    // Test a few entries
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.MODULE0", RDK_LOG_DEBUG);
    EXPECT_EQ(enabled, TRUE) << "Should parse large config file correctly";
    
    enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.MODULE999", RDK_LOG_DEBUG);
    EXPECT_EQ(enabled, TRUE) << "Should parse large config file correctly";
}

// Test configuration file with duplicate entries
TEST_F(RDKLoggerConfigTest, DuplicateConfigEntries) {
    createTestConfigFile("/tmp/rdk_logger_test/duplicate.ini",
        "LOG.RDK.TEST=DEBUG\n"
        "LOG.RDK.TEST=ERROR\n"  // Duplicate entry
        "LOG.RDK.TEST=INFO\n"); // Another duplicate
    
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/duplicate.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle duplicate config entries";
    
    // Last entry should take precedence
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.TEST", RDK_LOG_INFO);
    EXPECT_EQ(enabled, TRUE) << "Last duplicate entry should take precedence";
}

// Test configuration file with invalid log levels
TEST_F(RDKLoggerConfigTest, InvalidLogLevels) {
    createTestConfigFile("/tmp/rdk_logger_test/invalid_levels.ini",
        "LOG.RDK.DEFAULT=INVALID_LEVEL\n"
        "LOG.RDK.TEST=DEBUG\n"
        "LOG.RDK.ERROR=UNKNOWN_LEVEL\n");
    
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/invalid_levels.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle invalid log levels gracefully";
    
    // Valid entries should still work
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.TEST", RDK_LOG_DEBUG);
    EXPECT_EQ(enabled, TRUE) << "Valid log levels should still work";
}

// Test configuration file with very long module names
TEST_F(RDKLoggerConfigTest, LongModuleNames) {
    char long_module[256];
    snprintf(long_module, sizeof(long_module), "LOG.RDK.%s", 
             "VERY_LONG_MODULE_NAME_THAT_EXCEEDS_NORMAL_LENGTH_AND_SHOULD_BE_HANDLED_GRACEFULLY_BY_THE_CONFIGURATION_PARSER");
    
    FILE* file = fopen("/tmp/rdk_logger_test/long_names.ini", "w");
    ASSERT_TRUE(file != NULL) << "Failed to create config file with long names";
    fprintf(file, "LOG.RDK.DEFAULT=INFO\n");
    fprintf(file, "%s=DEBUG\n", long_module);
    fclose(file);
    
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/long_names.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle long module names";
    
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled(long_module, RDK_LOG_DEBUG);
    EXPECT_EQ(enabled, TRUE) << "Long module names should be handled correctly";
}

// Test configuration file with special characters
TEST_F(RDKLoggerConfigTest, SpecialCharactersInConfig) {
    createTestConfigFile("/tmp/rdk_logger_test/special_chars.ini",
        "LOG.RDK.DEFAULT=INFO\n"
        "LOG.RDK.TEST_WITH_UNDERSCORES=DEBUG\n"
        "LOG.RDK.TEST-WITH-DASHES=ERROR\n"
        "LOG.RDK.TEST.WITH.DOTS=WARNING\n");
    
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/special_chars.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle special characters in module names";
    
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.TEST_WITH_UNDERSCORES", RDK_LOG_DEBUG);
    EXPECT_EQ(enabled, TRUE) << "Underscores should be handled correctly";
}

// Test configuration file with mixed case log levels
TEST_F(RDKLoggerConfigTest, MixedCaseLogLevels) {
    createTestConfigFile("/tmp/rdk_logger_test/mixed_case.ini",
        "LOG.RDK.DEFAULT=info\n"
        "LOG.RDK.TEST=debug\n"
        "LOG.RDK.ERROR=ERROR\n"
        "LOG.RDK.WARNING=Warning\n");
    
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_test/mixed_case.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle mixed case log levels";
    
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.TEST", RDK_LOG_DEBUG);
    EXPECT_EQ(enabled, TRUE) << "Lowercase log levels should be handled correctly";
    
    enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.WARNING", RDK_LOG_WARN);
    EXPECT_EQ(enabled, TRUE) << "Mixed case log levels should be handled correctly";
}
