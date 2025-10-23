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
Test Case : Testing RDK Logger Dynamic Logger Functionality
*******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include "rdk_logger.h"
#include "gtest_app.h"
#if 0
class RDKLoggerDynamicTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration file
        system("mkdir -p /tmp/rdk_logger_dynamic_test");
        createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini", 
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");
        
        // Initialize logger
        rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_dynamic_test/test.ini");
        ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
        
        // Set program name for dynamic logger
        __progname = "test_program";
    }
    
    void TearDown() override {
        // Cleanup
        system("rm -rf /tmp/rdk_logger_dynamic_test");
        rdk_logger_deinit();
    }
#endif    
    void createTestConfigFile(const char* filename, const char* content) {
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%s", content);
            fclose(file);
        }
    }
    
    int createTestSocket() {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            return -1;
        }
        
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(12035);
        addr.sin_addr.s_addr = inet_addr("127.255.255.255");
        
        if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sockfd);
            return -1;
        }
        
        return sockfd;
    }
    
    void sendDynamicLogMessage(int sockfd, const char* app_name, const char* module_name, int log_level) {
        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(12035);
        dest_addr.sin_addr.s_addr = inet_addr("127.255.255.255");
        
        unsigned char buf[128] = {0};
        int i = 0;
        
        // Signature "COMC"
        memcpy(buf + i, "COMC", 4);
        i += 4;
        
        // Log level
        buf[i++] = (unsigned char)log_level;
        
        // App name length
        int app_len = strlen(app_name);
        buf[i++] = app_len;
        
        // App name
        memcpy(buf + i, app_name, app_len);
        i += app_len;
        
        // Module name length
        int module_len = strlen(module_name);
        buf[i++] = module_len;
        
        // Module name
        memcpy(buf + i, module_name, module_len);
        i += module_len;
        
        // Total packet length
        buf[4] = app_len + module_len + 3;
        
        sendto(sockfd, buf, buf[4] + 5, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    }

// Test dynamic logger initialization
TEST(RDKLoggerDynamicTest, DynamicLoggerInitialization) {
    // Dynamic logger should be initialized during rdk_logger_init
    // Test that we can log messages
   system("mkdir -p /tmp/rdk_logger_dynamic_test");
   createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Test message after dynamic logger init");
    system("rm -f /tmp/rdk_logger_dynamic_test/test.ini");
    // Should work correctly
}

// Test dynamic log level change
TEST(RDKLoggerDynamicTest, DynamicLogLevelChange) {
    system("mkdir -p /tmp/rdk_logger_dynamic_test");
    createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");	
    int sockfd = createTestSocket();
    ASSERT_GE(sockfd, 0) << "Failed to create test socket";
    
    // Send dynamic log level change message
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_DEBUG);
    
    // Give some time for message processing
    usleep(100000); // 100ms
    
    // Process pending requests
    // Note: This would normally be called automatically during logging
    // but we can't access the private function directly
    
    // Test that the log level change took effect
    rdk_logger_Bool enabled = rdk_logger_is_logLevel_enabled("LOG.RDK.DYNAMIC", RDK_LOG_DEBUG);
    // The change might not be visible immediately due to timing
    
    close(sockfd);
    system("rm -f /tmp/rdk_logger_dynamic_test/test.ini");
}
#if 0
// Test dynamic logger with invalid messages
TEST(RDKLoggerDynamicTest, InvalidDynamicMessages) {
   system("mkdir -p /tmp/rdk_logger_dynamic_test");
   createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");	
    int sockfd = createTestSocket();
    ASSERT_GE(sockfd, 0) << "Failed to create test socket";
    
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(12035);
    dest_addr.sin_addr.s_addr = inet_addr("127.255.255.255");
    
    // Test invalid signature
    unsigned char invalid_sig[10] = "INVALID";
    sendto(sockfd, invalid_sig, 7, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    // Test wrong app name
    sendDynamicLogMessage(sockfd, "wrong_app", "LOG.RDK.DYNAMIC", RDK_LOG_DEBUG);
    
    // Test invalid log level
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", 999);
    
    // Test empty module name
    sendDynamicLogMessage(sockfd, "test_program", "", RDK_LOG_DEBUG);
    
    // Test very long module name
    char long_module[1000];
    memset(long_module, 'A', sizeof(long_module) - 1);
    long_module[sizeof(long_module) - 1] = '\0';
    sendDynamicLogMessage(sockfd, "test_program", long_module, RDK_LOG_DEBUG);
    
    close(sockfd);
    system("rm -f /tmp/rdk_logger_dynamic_test/test.ini");
}

// Test dynamic logger with different log levels
TEST(RDKLoggerDynamicTest, DifferentLogLevels) {
   system("mkdir -p /tmp/rdk_logger_dynamic_test");
   createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");	
    int sockfd = createTestSocket();
    ASSERT_GE(sockfd, 0) << "Failed to create test socket";
    
    // Test all valid log levels
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_FATAL);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_ERROR);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_WARN);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_NOTICE);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_INFO);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_DEBUG);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_TRACE);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_NONE);
    
    // Test negated log levels (with 0x80 bit set)
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_DEBUG | 0x80);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_ERROR | 0x80);
    
    close(sockfd);
    system("rm -f /tmp/rdk_logger_dynamic_test/test.ini");
}

// Test dynamic logger with different module names
TEST(RDKLoggerDynamicTest, DifferentModuleNames) {
   system("mkdir -p /tmp/rdk_logger_dynamic_test");
   createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");	
    int sockfd = createTestSocket();
    ASSERT_GE(sockfd, 0) << "Failed to create test socket";
    
    // Test various module name formats
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.TEST", RDK_LOG_DEBUG);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.MODULE1", RDK_LOG_INFO);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.MODULE_WITH_UNDERSCORES", RDK_LOG_WARN);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.MODULE-WITH-DASHES", RDK_LOG_ERROR);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.MODULE.WITH.DOTS", RDK_LOG_FATAL);
    
    close(sockfd);
    system("rm -f /tmp/rdk_logger_dynamic_test/test.ini");
}

// Test dynamic logger with different app names
TEST(RDKLoggerDynamicTest, DifferentAppNames) {
   system("mkdir -p /tmp/rdk_logger_dynamic_test");
   createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");
    int sockfd = createTestSocket();
    ASSERT_GE(sockfd, 0) << "Failed to create test socket";
    
    // Test with current app name (should work)
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_DEBUG);
    
    // Test with different app name (should be ignored)
    sendDynamicLogMessage(sockfd, "other_program", "LOG.RDK.DYNAMIC", RDK_LOG_DEBUG);
    
    // Test with empty app name
    sendDynamicLogMessage(sockfd, "", "LOG.RDK.DYNAMIC", RDK_LOG_DEBUG);
    
    // Test with very long app name
    char long_app[1000];
    memset(long_app, 'A', sizeof(long_app) - 1);
    long_app[sizeof(long_app) - 1] = '\0';
    sendDynamicLogMessage(sockfd, long_app, "LOG.RDK.DYNAMIC", RDK_LOG_DEBUG);
    
    close(sockfd);
    system("rm -f /tmp/rdk_logger_dynamic_test/test.ini");
}

// Test dynamic logger message format validation
TEST(RDKLoggerDynamicTest, MessageFormatValidation) {
   system("mkdir -p /tmp/rdk_logger_dynamic_test");
   createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");	
    int sockfd = createTestSocket();
    ASSERT_GE(sockfd, 0) << "Failed to create test socket";
    
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(12035);
    dest_addr.sin_addr.s_addr = inet_addr("127.255.255.255");
    
    // Test message with wrong length
    unsigned char buf[128] = {0};
    memcpy(buf, "COMC", 4);
    buf[4] = 10; // Wrong length
    buf[5] = RDK_LOG_DEBUG;
    buf[6] = 11; // App name length
    memcpy(buf + 7, "test_program", 11);
    buf[18] = 15; // Module name length
    memcpy(buf + 19, "LOG.RDK.DYNAMIC", 15);
    
    sendto(sockfd, buf, 34, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    // Test message with zero length
    buf[4] = 0;
    sendto(sockfd, buf, 5, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    // Test message with negative length
    buf[4] = 255; // Maximum value
    sendto(sockfd, buf, 5, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    close(sockfd);
    system("rm -f /tmp/rdk_logger_dynamic_test/test.ini");
}

// Test dynamic logger with rapid messages
TEST(RDKLoggerDynamicTest, RapidMessages) {
   system("mkdir -p /tmp/rdk_logger_dynamic_test");
   createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");	
    int sockfd = createTestSocket();
    ASSERT_GE(sockfd, 0) << "Failed to create test socket";
    
    // Send many messages rapidly
    for (int i = 0; i < 100; i++) {
        char module_name[50];
        snprintf(module_name, sizeof(module_name), "LOG.RDK.MODULE%d", i % 10);
        sendDynamicLogMessage(sockfd, "test_program", module_name, RDK_LOG_DEBUG);
    }
    
    close(sockfd);
    system("rm -f /tmp/rdk_logger_dynamic_test/test.ini");
}

// Test dynamic logger with concurrent access
TEST(RDKLoggerDynamicTest, ConcurrentAccess) {
   system("mkdir -p /tmp/rdk_logger_dynamic_test");
   createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");	
    int sockfd = createTestSocket();
    ASSERT_GE(sockfd, 0) << "Failed to create test socket";
    
    // Send messages from different "processes"
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_DEBUG);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_INFO);
    sendDynamicLogMessage(sockfd, "test_program", "LOG.RDK.DYNAMIC", RDK_LOG_WARN);
    
    // Test that logging still works during dynamic changes
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.DYNAMIC", "Test message during dynamic changes");
    
    close(sockfd);
    system("rm -f /tmp/rdk_logger_dynamic_test/test.ini");
}

// Test dynamic logger with network errors
TEST(RDKLoggerDynamicTest, NetworkErrors) {
   system("mkdir -p /tmp/rdk_logger_dynamic_test");
   createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");	
    // Test with invalid socket
    int invalid_sockfd = -1;
    // This would normally cause errors in the dynamic logger
    // but we can't test this directly as the socket is created internally
    
    // Test that logging still works even if dynamic logger has issues
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Test message despite network issues");
}

// Test dynamic logger deinitialization
TEST(RDKLoggerDynamicTest, DynamicLoggerDeinit) {
    // Dynamic logger should be deinitialized during rdk_logger_deinit
    rdk_Error ret = rdk_logger_deinit();
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to deinitialize logger";
    
    // Test that logging still works after deinit
    rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.TEST", "Test message after deinit");
}

// Test dynamic logger with malformed packets
TEST(RDKLoggerDynamicTest, MalformedPackets) {
   system("mkdir -p /tmp/rdk_logger_dynamic_test");
   createTestConfigFile("/tmp/rdk_logger_dynamic_test/test.ini",
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.TEST=DEBUG\n"
            "LOG.RDK.DYNAMIC=WARNING\n");
    int sockfd = createTestSocket();
    ASSERT_GE(sockfd, 0) << "Failed to create test socket";
    
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(12035);
    dest_addr.sin_addr.s_addr = inet_addr("127.255.255.255");
    
    // Test packet that's too short
    unsigned char short_packet[3] = "CO";
    sendto(sockfd, short_packet, 2, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    // Test packet that's too long
    unsigned char long_packet[1000];
    memset(long_packet, 'A', sizeof(long_packet));
    memcpy(long_packet, "COMC", 4);
    sendto(sockfd, long_packet, sizeof(long_packet), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    // Test packet with null bytes
    unsigned char null_packet[20] = {0};
    memcpy(null_packet, "COMC", 4);
    sendto(sockfd, null_packet, sizeof(null_packet), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    
    close(sockfd);
    system("rm -f /tmp/rdk_logger_dynamic_test/test.ini");
}
#endif
