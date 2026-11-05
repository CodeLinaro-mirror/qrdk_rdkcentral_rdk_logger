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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <fstream>
#include <iterator>
#include <string>
#include "rdk_logger.h"
#include "gtest_app.h"
#include "rdk_logger_milestone.h"


TEST(RDKLoggerUtilityTest, LogOnboardFunctionality) {
    char conf_file[] = GTEST_DEBUG_INI_FILE;	
    rdk_Error ret = rdk_logger_init(conf_file);
    ASSERT_EQ(ret, RDK_SUCCESS) << "Failed to initialize logger";
    // Test onboard logging
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Test onboard message");
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Onboard message with format: %d", 123);
    rdk_logger_log_onboard("LOG.RDK.ONBOARD", "Onboard message with multiple args: %s %d %f", "test", 456, 3.14);
    logMilestone("APPLICATION_READY");

    // Should work correctly
}
// Test rdk_logger_log_onboard with different log levels
TEST(RDKLoggerUtilityTest, LogOnboardDifferentLevels) {
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    rdk_Error ret = rdk_logger_init(conf_file);	
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

  TEST(RDKLoggerUtilityTest, LogOnboardHandlesCRLFInput) {
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    ASSERT_EQ(rdk_logger_init(conf_file), RDK_SUCCESS);
    if (access("/nvram/.device_onboarded", F_OK) == 0 ||
        access("/nvram/DISABLE_ONBOARD_LOGGING", F_OK) == 0)
        GTEST_SKIP() << "Onboarding logging is disabled by a device gate";

    const std::string token = "M004_" + std::to_string(static_cast<long>(getpid()));
    const char* onboard_log_paths[] = {
      "/tmp/rdk_logger_onboarding_test.log",
      "/rdklogs/logs/OnBoardingLog.txt.0",
      "/opt/logs/OnBoardingLog.txt.0",
      "ON"
    };
    testing::internal::CaptureStdout();
    rdk_logger_log_onboard("LOG.RDK.ONBOARD\n[FATAL] forged module",
                            "Onboard message %s\r\n[FATAL] forged record %s\n",
                            token.c_str(), token.c_str());
    fflush(stdout);
    std::string output = testing::internal::GetCapturedStdout();
    if (output.find(token) == std::string::npos) {
      for (const char* path : onboard_log_paths) {
        std::ifstream onboard_log(path);
        std::string contents((std::istreambuf_iterator<char>(onboard_log)),
                   std::istreambuf_iterator<char>());
        if (contents.find(token) != std::string::npos) {
          output = contents;
          break;
        }
        }
    }

    ASSERT_NE(output.find("Onboard message " + token), std::string::npos)
      << "Onboarding output was not captured; checked stdout and configured log paths: "
      << output;
    EXPECT_EQ(output.find("\n[FATAL] forged module"), std::string::npos)
      << "Module input created a forged record: " << output;
    EXPECT_EQ(output.find("\n[FATAL] forged record " + token), std::string::npos)
      << "Message input created a forged record: " << output;
  }
