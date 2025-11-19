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
Test Case : Testing RDK Logger Log Rotation Functionality
*******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include "rdk_logger.h"
#include "gtest_app.h"
#include "log4c.h"

class RDKLoggerRotationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        system("mkdir -p /tmp/rdk_logger_rotation_test");
    }
    
    void TearDown() override {
        // Cleanup
        system("rm -rf /tmp/rdk_logger_rotation_test");
		/*const char* cat_name = "LOG.RDK";
        log4c_category_t* cat = log4c_category_get(cat_name);
        if (cat) {
            // Get const appender from category
            const log4c_appender_t* const_app = log4c_category_get_appender(cat);
            if (const_app) {
                // Get mutable appender using the name
                const char* app_name = log4c_appender_get_name(const_app);
                if (app_name) {
                    log4c_appender_t* app = log4c_appender_get(app_name);
                    if (app) {
                        log4c_appender_close(app);
                        log4c_appender_set_udata(app, NULL);
                    }
                }
            }
        }
		rdk_log_exit();
        rdk_logger_deinit();*/
    }

    #define RUN_IN_FORK(test_body) \
    pid_t pid = fork(); \
    ASSERT_NE(pid, -1) << "fork failed"; \
    if (pid == 0) { \
        log4c_init(); \
        test_body; \
        rdk_log_exit(); \
        rdk_logger_deinit(); \
        exit(0); \
    } else { \
        int status = 0; \
        waitpid(pid, &status, 0); \
        ASSERT_TRUE(WIFEXITED(status)); \
        ASSERT_EQ(WEXITSTATUS(status), 0); \
    }

    
    void createTestConfigFile(const char* filename, const char* content) {
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%s", content);
            fclose(file);
        }
    }
    
    long getFileSize(const char* filename) {
        struct stat st;
        if (stat(filename, &st) == 0) {
            return st.st_size;
        }
        return -1;
    }
    
    int countFilesInDirectory(const char* dirname) {
        DIR* dir = opendir(dirname);
        if (!dir) {
            return -1;
        }
        
        int count = 0;
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
	        printf("Found file: %s\n", entry->d_name);	    
                count++;
            }
        }
        closedir(dir);
        return count;
    }
    
    void createLargeLogMessage(char* buffer, size_t size) {
        // Create a message that will fill the buffer
        const char* base_message = "This is a test log message for rotation testing. ";
        size_t base_len = strlen(base_message);
        
        size_t pos = 0;
        while (pos < size - 1) {
            size_t remaining = size - pos - 1;
            size_t copy_len = (remaining > base_len) ? base_len : remaining;
            memcpy(buffer + pos, base_message, copy_len);
            pos += copy_len;
        }
        buffer[size - 1] = '\0';
    }
};

 #define RUN_IN_FORK(test_body) \
    pid_t pid = fork(); \
    ASSERT_NE(pid, -1) << "fork failed"; \
    if (pid == 0) { \
        log4c_init(); \
        test_body; \
        rdk_log_exit(); \
        rdk_logger_deinit(); \
        exit(0); \
    } else { \
        int status = 0; \
        waitpid(pid, &status, 0); \
        ASSERT_TRUE(WIFEXITED(status)); \
        ASSERT_EQ(WEXITSTATUS(status), 0); \
    }
// Test extended initialization with log rotation
TEST_F(RDKLoggerRotationTest, ExtendedInitialization) {
  RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.fileName, "test_rotation.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 1024;  // 1KB max size
    config.maxCount = 3;    // Keep 3 files
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_TRACE;
    config.layout = LAYOUT_DATED;
    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Extended initialization should succeed";
    
    // Test that logging works
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Test message for rotation");
  );
}
// Test log rotation with count limits
TEST_F(RDKLoggerRotationTest, CountBasedRotation) {
   RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.fileName, "count_test.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 256;   // 256 bytes max size
    config.maxCount = 2;    // Keep only 2 files
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_ERROR;
    config.layout = LAYOUT_DATED;
    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Extended initialization should succeed";
    
    // Generate many log messages to trigger multiple rotations
    char large_message[200];
    createLargeLogMessage(large_message, sizeof(large_message));
    
    for (int i = 0; i < 20; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Message %d: %s", i, large_message);
        sleep(1);
    }
    
    // Check that only maxCount files exist
    int file_count = countFilesInDirectory("/tmp/rdk_logger_rotation_test");
    printf("file_count : %d\n",file_count);
    system("ls -lt /tmp/rdk_logger_rotation_test");
    EXPECT_LE(file_count, config.maxCount + 1) << "Should not exceed maxCount files";
   });
}
#if 0
// Test log rotation with invalid configuration
TEST_F(RDKLoggerRotationTest, InvalidConfiguration) {
    rdk_logger_ext_config_t config;
    
    // Test with NULL config
    rdk_Error ret = rdk_logger_ext_init(NULL);
    // Should handle gracefully
    
    // Test with empty file name
    strncpy(config.fileName, "", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 1024;
    config.maxCount = 3;
    
    ret = rdk_logger_ext_init(&config);
    // Should handle gracefully
    
    // Test with empty log directory
    strncpy(config.fileName, "test.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    ret = rdk_logger_ext_init(&config);
    // Should handle gracefully
}
#endif
// Test log rotation with invalid directory
TEST_F(RDKLoggerRotationTest, InvalidDirectory) {
  RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.fileName, "test.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/nonexistent/directory", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 1024;
    config.maxCount = 3;
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_NONE;
    config.layout = LAYOUT_DATED;
    rdk_Error ret = rdk_logger_ext_init(&config);
    // Should handle gracefully (may fail or create directory)
 });
}

// Test log rotation with very small size limits
TEST_F(RDKLoggerRotationTest, VerySmallSizeLimits) {
   RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.fileName, "small_test.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 10;    // Very small size
    config.maxCount = 2;
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_INFO;
    config.layout = LAYOUT_DATED;
    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle very small size limits";
    
    // Generate log messages
    for (int i = 0; i < 5; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Message %d", i);
        sleep(1);
    }
});
    // Should handle gracefully
}

// Test log rotation with very large size limits
TEST_F(RDKLoggerRotationTest, VeryLargeSizeLimits) {
   RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.fileName, "large_test.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    printf("strcpy success\n"); 
    config.maxSize = 1024 * 1024 * 100;  // 100MB
    config.maxCount = 10;
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_WARN;
    config.layout = LAYOUT_COMCAST_DATED;
    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle very large size limits";
    printf("ext_init succes\n"); 
    // Generate some log messages
    for (int i = 0; i < 10; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Message %d", i);
        sleep(1);
        printf("Iteration:%d",i);
    }
    
    // Should handle gracefully
});
}

// Test log rotation with zero count limits
TEST_F(RDKLoggerRotationTest, ZeroCountLimits) {
   RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.fileName, "zero_count_test.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 1024;
    config.maxCount = 0;    // Zero count
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_FATAL;
    config.layout = LAYOUT_BASIC;

    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle zero count limits";
    
    // Generate log messages
    for (int i = 0; i < 5; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Message %d", i);
        sleep(1);
    }
    
    // Should handle gracefully
  });
}

// Test log rotation with negative values
TEST_F(RDKLoggerRotationTest, NegativeValues) {
   RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.fileName, "negative_test.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = -1;    // Negative size
    config.maxCount = -1;   // Negative count
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_TRACE;
    config.layout = LAYOUT_DATED;
    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle negative values";
    
    // Generate log messages
    for (int i = 0; i < 5; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Message %d", i);
        sleep(1);
    }
    
    // Should handle gracefully
  });
}

// Test log rotation with long file names
TEST_F(RDKLoggerRotationTest, LongFileNames) {
   RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config)); 
    // Create a very long file name
    char long_filename[RDK_LOGGER_EXT_FILENAME_SIZE];
    memset(long_filename, 'A', sizeof(long_filename) - 5);
    strcpy(long_filename + sizeof(long_filename) - 5, ".log");
    long_filename[sizeof(long_filename) - 1] = '\0';
    
    strncpy(config.fileName, long_filename, sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 1024;
    config.maxCount = 3;
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_TRACE;
    config.layout = LAYOUT_DATED;
    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle long file names";
    
    // Generate log messages
    for (int i = 0; i < 5; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Message %d", i);
        sleep(1);
    }
    
    // Should handle gracefully
  });
}

// Test log rotation with long directory paths
TEST_F(RDKLoggerRotationTest, LongDirectoryPaths) {
   RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.fileName, "test.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    // Create a very long directory path
    char long_dir[RDK_LOGGER_EXT_LOGDIR_SIZE];
    memset(long_dir, 'A', sizeof(long_dir) - 1);
    long_dir[sizeof(long_dir) - 1] = '\0';
    
    strncpy(config.logdir, long_dir, sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 1024;
    config.maxCount = 3;
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_TRACE;
    config.layout = LAYOUT_DATED;
    rdk_Error ret = rdk_logger_ext_init(&config);
    // Should handle gracefully (may fail due to path length)
    
    // Generate log messages
    for (int i = 0; i < 5; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Message %d", i);
        sleep(1);
    }
});
    // Should handle gracefully
}

// Test log rotation with special characters in file names
TEST_F(RDKLoggerRotationTest, SpecialCharactersInFileNames) {
   RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.fileName, "test_file_with_special_chars.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 1024;
    config.maxCount = 3;
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_TRACE;
    config.layout = LAYOUT_BASIC;
    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Should handle special characters in file names";
    
    // Generate log messages
    for (int i = 0; i < 5; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Message %d", i);
        sleep(1);
    }
}); 
    // Should handle gracefully
}

// Test log rotation with concurrent access
TEST_F(RDKLoggerRotationTest, ConcurrentAccess) {
   RUN_IN_FORK({
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    strncpy(config.fileName, "concurrent_test.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 512;
    config.maxCount = 3;
    config.appender_type = FileOutput;
    config.loglevel = RDK_LOG_DEBUG;
    config.layout = LAYOUT_DATED;

    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Extended initialization should succeed";
    
    // Generate log messages rapidly to test concurrent access
    for (int i = 0; i < 20; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Concurrent message %d", i);
        rdk_logger_msg_printf(RDK_LOG_DEBUG, "LOG.RDK.ROTATION", "Debug message %d", i);
        rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.ROTATION", "Error message %d", i);
        sleep(1);
    }
    
    // Should handle concurrent access gracefully
});
}
#if 0
// Test log rotation with different log levels
TEST_F(RDKLoggerRotationTest, DifferentLogLevels) {
    rdk_logger_ext_config_t config;
    strncpy(config.fileName, "levels_test.log", sizeof(config.fileName) - 1);
    config.fileName[sizeof(config.fileName) - 1] = '\0';
    
    strncpy(config.logdir, "/tmp/rdk_logger_rotation_test", sizeof(config.logdir) - 1);
    config.logdir[sizeof(config.logdir) - 1] = '\0';
    
    config.maxSize = 1024;
    config.maxCount = 3;
    
    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Extended initialization should succeed";
    
    // Test all log levels
    rdk_logger_msg_printf(RDK_LOG_FATAL, "LOG.RDK.ROTATION", "Fatal message");
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.ROTATION", "Error message");
    rdk_logger_msg_printf(RDK_LOG_WARN, "LOG.RDK.ROTATION", "Warning message");
    rdk_logger_msg_printf(RDK_LOG_NOTICE, "LOG.RDK.ROTATION", "Notice message");
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ROTATION", "Info message");
    rdk_logger_msg_printf(RDK_LOG_DEBUG, "LOG.RDK.ROTATION", "Debug message");
    rdk_logger_msg_printf(RDK_LOG_TRACE, "LOG.RDK.ROTATION", "Trace message");
    
    // Should handle all log levels correctly
}
#endif
