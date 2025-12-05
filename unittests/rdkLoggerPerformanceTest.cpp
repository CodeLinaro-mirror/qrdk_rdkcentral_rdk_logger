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
Test Case : Testing RDK Logger Performance and Stress Testing
*******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <signal.h>
#include "rdk_logger.h"
#include "gtest_app.h"

class RDKLoggerPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        system("mkdir -p /tmp/rdk_logger_performance_test");
        
        // Create test configuration file
        createTestConfigFile("/tmp/rdk_logger_performance_test/test.ini", 
            "LOG.RDK.DEFAULT=INFO\n"
            "LOG.RDK.PERFORMANCE=DEBUG\n"
            "LOG.RDK.STRESS=TRACE\n");
    }
    
    void TearDown() override {
        // Cleanup
        system("rm -rf /tmp/rdk_logger_performance_test");
        rdk_logger_deinit();
    }
    
    void createTestConfigFile(const char* filename, const char* content) {
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "%s", content);
            fclose(file);
        }
    }
    
    double getTimeDifference(struct timeval* start, struct timeval* end) {
        return (end->tv_sec - start->tv_sec) + (end->tv_usec - start->tv_usec) / 1000000.0;
    }
    
    void* threadFunction(void* arg) {
        int thread_id = *(int*)arg;
        for (int i = 0; i < 1000; i++) {
            rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", 
                "Thread %d message %d", thread_id, i);
        }
        return NULL;
    }
};

// Test basic logging performance
TEST_F(RDKLoggerPerformanceTest, BasicLoggingPerformance) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log 10000 messages
    for (int i = 0; i < 10000; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", 
            "Performance test message %d", i);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 10000 messages in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 10000.0 / elapsed);
    
    // Should complete in reasonable time (less than 10 seconds)
    EXPECT_LT(elapsed, 10.0) << "Logging should be reasonably fast";
}

// Test logging with different log levels
TEST_F(RDKLoggerPerformanceTest, DifferentLogLevelsPerformance) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log messages with different levels
    for (int i = 0; i < 1000; i++) {
        rdk_logger_msg_printf(RDK_LOG_FATAL, "LOG.RDK.PERFORMANCE", "Fatal message %d", i);
        rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.PERFORMANCE", "Error message %d", i);
        rdk_logger_msg_printf(RDK_LOG_WARN, "LOG.RDK.PERFORMANCE", "Warning message %d", i);
        rdk_logger_msg_printf(RDK_LOG_NOTICE, "LOG.RDK.PERFORMANCE", "Notice message %d", i);
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", "Info message %d", i);
        rdk_logger_msg_printf(RDK_LOG_DEBUG, "LOG.RDK.PERFORMANCE", "Debug message %d", i);
        rdk_logger_msg_printf(RDK_LOG_TRACE, "LOG.RDK.PERFORMANCE", "Trace message %d", i);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 7000 messages with different levels in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 7000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 10.0) << "Logging with different levels should be reasonably fast";
}

// Test logging with different module names
TEST_F(RDKLoggerPerformanceTest, DifferentModuleNamesPerformance) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log messages with different module names
    for (int i = 0; i < 1000; i++) {
        char module_name[50];
        snprintf(module_name, sizeof(module_name), "LOG.RDK.MODULE%d", i % 100);
        rdk_logger_msg_printf(RDK_LOG_INFO, module_name, "Message %d", i);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 1000 messages with different modules in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 1000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 5.0) << "Logging with different modules should be reasonably fast";
}

// Test logging with long messages
TEST_F(RDKLoggerPerformanceTest, LongMessagesPerformance) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Create a long message
    char long_message[1000];
    memset(long_message, 'A', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log messages with long content
    for (int i = 0; i < 1000; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", 
            "Long message %d: %s", i, long_message);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 1000 long messages in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 1000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 10.0) << "Logging long messages should be reasonably fast";
}

// Test logging with complex format strings
TEST_F(RDKLoggerPerformanceTest, ComplexFormatStringsPerformance) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log messages with complex format strings
    for (int i = 0; i < 1000; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", 
            "Complex format: %d %s %f %c %x %o %p", 
            i, "test", 3.14, 'A', i, i, &i);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 1000 complex format messages in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 1000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 10.0) << "Logging complex format strings should be reasonably fast";
}

// Test logging with rapid calls
TEST_F(RDKLoggerPerformanceTest, RapidCallsPerformance) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log messages rapidly
    for (int i = 0; i < 5000; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", "Rapid message %d\n", i);
        rdk_logger_msg_printf(RDK_LOG_DEBUG, "LOG.RDK.PERFORMANCE", "Rapid debug %d\n", i);
        rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.PERFORMANCE", "Rapid error %d\n", i);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 5000 rapid messages in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 5000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 15.0) << "Rapid logging should be reasonably fast";
}

// Test logging with memory usage
TEST_F(RDKLoggerPerformanceTest, MemoryUsageTest) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct rusage usage_before, usage_after;
    getrusage(RUSAGE_SELF, &usage_before);
    
    // Log many messages
    for (int i = 0; i < 10000; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", 
            "Memory test message %d", i);
    }
    
    getrusage(RUSAGE_SELF, &usage_after);
    
    long memory_diff = usage_after.ru_maxrss - usage_before.ru_maxrss;
    printf("Memory usage increased by %ld KB\n", memory_diff);
    
    // Memory usage should be reasonable (less than 10MB)
    EXPECT_LT(memory_diff, 10240) << "Memory usage should be reasonable";
}

// Test logging with CPU usage
TEST_F(RDKLoggerPerformanceTest, CPUUsageTest) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct rusage usage_before, usage_after;
    getrusage(RUSAGE_SELF, &usage_before);
    
    // Log many messages
    for (int i = 0; i < 10000; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", 
            "CPU test message %d", i);
    }
    
    getrusage(RUSAGE_SELF, &usage_after);
    
    long cpu_diff = usage_after.ru_utime.tv_sec - usage_before.ru_utime.tv_sec;
    printf("CPU usage increased by %ld seconds\n", cpu_diff);
    
    // CPU usage should be reasonable (less than 5 seconds)
    EXPECT_LT(cpu_diff, 5) << "CPU usage should be reasonable";
}

// Test logging with file I/O stress
TEST_F(RDKLoggerPerformanceTest, FileIOStressTest) {
    rdk_LogOutput_File testPolicy;
    strncpy(testPolicy.fileName, "stress_test.log", sizeof(testPolicy.fileName)-1);
    testPolicy.fileName[sizeof(testPolicy.fileName) - 1] = '\0';
    strncpy(testPolicy.fileLocation, "/tmp/rdk_logger_performance_test", sizeof(testPolicy.fileLocation)-1);
    testPolicy.fileLocation[sizeof(testPolicy.fileLocation) - 1] = '\0';
    testPolicy.fileSizeMax = 1024; // 2MB
    testPolicy.fileCountMax = 5;
    rdk_logger_ext_config_t config;
    memset(&config, 0, sizeof(config));
    config.pCategoryName = "LOG.RDK.PERFORMANCE";
    config.loglevel = RDK_LOG_DEBUG;
    config.appender = RDKLOG_OUTPUT_FILE;
    config.layout = RDKLOG_FORMAT_WITH_DATETIME;
    config.pFilePolicy = &testPolicy;
    
    rdk_Error ret = rdk_logger_ext_init(&config);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Extended initialization should succeed";
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log many messages to trigger file rotation
    for (int i = 0; i < 5000; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", 
            "File I/O stress test message %d", i);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 5000 messages with file rotation in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 5000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 20.0) << "File I/O stress test should be reasonably fast";
}

// Test logging with signal handling
TEST_F(RDKLoggerPerformanceTest, SignalHandlingTest) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    // Test logging during signal handling
    signal(SIGUSR1, SIG_IGN);
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log messages while sending signals
    for (int i = 0; i < 1000; i++) {
        if (i % 100 == 0) {
            kill(getpid(), SIGUSR1);
        }
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", 
            "Signal test message %d", i);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 1000 messages with signals in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 1000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 10.0) << "Signal handling test should be reasonably fast";
}

// Test logging with error conditions
TEST_F(RDKLoggerPerformanceTest, ErrorConditionsTest) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log messages with various error conditions
    for (int i = 0; i < 1000; i++) {
        // Test with NULL parameters
        rdk_logger_msg_printf(RDK_LOG_INFO, NULL, "NULL module test %d", i);
        //rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", NULL);
        
        // Test with invalid log levels
        rdk_logger_msg_printf((rdk_LogLevel)999, "LOG.RDK.PERFORMANCE", "Invalid level test %d", i);
        
        // Test with empty strings
        rdk_logger_msg_printf(RDK_LOG_INFO, "", "Empty module test %d", i);
        //rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", "");
        
        // Test with very long strings
        char long_string[1000];
        memset(long_string, 'A', sizeof(long_string) - 1);
        long_string[sizeof(long_string) - 1] = '\0';
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", "Long string test %d: %s", i, long_string);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 5000 messages with error conditions in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 5000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 15.0) << "Error conditions test should be reasonably fast";
}

// Test logging with different output destinations
TEST_F(RDKLoggerPerformanceTest, DifferentOutputDestinationsTest) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log messages with different modules (different output destinations)
    for (int i = 0; i < 1000; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", "Performance module message %d", i);
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.STRESS", "Stress module message %d", i);
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.DEFAULT", "Default module message %d", i);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 3000 messages with different destinations in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 3000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 10.0) << "Different output destinations test should be reasonably fast";
}

// Test logging with format string performance
TEST_F(RDKLoggerPerformanceTest, FormatStringPerformanceTest) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log messages with various format strings
    for (int i = 0; i < 1000; i++) {
        rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.PERFORMANCE", 
            "Format test %d: %s %d %f %c %x %o %p %s %d %f", 
            i, "string", i, 3.14, 'A', i, i, &i, "another", i, 2.71);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 1000 messages with complex format strings in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 1000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 10.0) << "Format string performance test should be reasonably fast";
}
// Test logging with onboard function performance
TEST_F(RDKLoggerPerformanceTest, OnboardFunctionPerformanceTest) {
    rdk_Error ret = rdk_logger_init("/tmp/rdk_logger_performance_test/test.ini");
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    // Log messages using onboard function
    for (int i = 0; i < 1000; i++) {
        rdk_logger_log_onboard("LOG.RDK.PERFORMANCE", "Onboard test %d", i);
    }
    
    gettimeofday(&end, NULL);
    double elapsed = getTimeDifference(&start, &end);
    
    printf("Logged 1000 messages with onboard function in %.3f seconds (%.0f messages/sec)\n", 
           elapsed, 1000.0 / elapsed);
    
    // Should complete in reasonable time
    EXPECT_LT(elapsed, 10.0) << "Onboard function performance test should be reasonably fast";
}
