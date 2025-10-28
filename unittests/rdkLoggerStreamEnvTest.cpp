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
#include "rdk_logger.h"
#include "gtest_app.h"
#include "log4c.h"

TEST(StreamEnvOpenTest, SimplePath_NoEnvVars_Stdout)
{
    // Initialize logger first
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    rdk_logger_init(conf_file);
    
    // Create a NEW appender (not opened yet, so fp will be NULL)
    log4c_appender_t* appender = log4c_appender_new("stdout");
    ASSERT_NE(appender, nullptr) << "Failed to create appender";
    
    // Set the type to stream_env (this registers the open callback)
    log4c_appender_set_type(appender, log4c_appender_type_get("stream_env"));
    
    // Now open it - this will call stream_env_open with fp=NULL
    // This WILL execute beyond line 615 because fp is NULL on first open
    int ret = log4c_appender_open(appender);
    EXPECT_EQ(ret, 0) << "stream_env_open should succeed";
    
    // Verify the file pointer is set to stdout
    FILE* fp = (FILE*)log4c_appender_get_udata(appender);
    EXPECT_EQ(fp, stdout) << "File pointer should be stdout";
    
    // Close the appender
    log4c_appender_close(appender);
}

TEST(StreamEnvOpenTest, EnvVariable_ValidPath_ParsesCorrectly)
{
    // Initialize logger
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    rdk_logger_init(conf_file);
    
    // Set an environment variable for testing
    setenv("RDK_TEST_LOG_DIR", "/tmp", 1);
    
    // Create appender with env var in name: $(RDK_TEST_LOG_DIR)/test.log
    log4c_appender_t* appender = log4c_appender_new("$(RDK_TEST_LOG_DIR)/streamenv_test.log");
    ASSERT_NE(appender, nullptr) << "Failed to create appender";
    
    // Use stream_env_append type (append=1 path)
    log4c_appender_set_type(appender, log4c_appender_type_get("stream_env_append"));
    
    // Open - this parses $(RDK_TEST_LOG_DIR) and resolves to /tmp/streamenv_test.log
    int ret = log4c_appender_open(appender);
    EXPECT_EQ(ret, 0) << "stream_env_open should parse env var and succeed";
    
    // Verify file pointer is not NULL and not stdout/stderr
    FILE* fp = (FILE*)log4c_appender_get_udata(appender);
    EXPECT_NE(fp, nullptr) << "File pointer should not be null";
    EXPECT_NE(fp, stdout) << "File pointer should not be stdout";
    EXPECT_NE(fp, stderr) << "File pointer should not be stderr";
    
    // Close and cleanup
    log4c_appender_close(appender);
    unlink("/tmp/streamenv_test.log");
    unsetenv("RDK_TEST_LOG_DIR");
}

TEST(StreamEnvOpenTest, EnvVariable_NotFound_ReturnsError)
{
    // Initialize logger
    char conf_file[] = GTEST_DEBUG_INI_FILE;
    rdk_logger_init(conf_file);
    
    // Ensure the env var doesn't exist
    unsetenv("RDK_NONEXISTENT_VAR_12345");
    
    // Create appender with non-existent env var
    log4c_appender_t* appender = log4c_appender_new("$(RDK_NONEXISTENT_VAR_12345)/test.log");
    ASSERT_NE(appender, nullptr) << "Failed to create appender";
    
    log4c_appender_set_type(appender, log4c_appender_type_get("stream_env"));
    
    // Open should fail because getenv returns NULL
    // This triggers: goto parse_error at line 641
    int ret = log4c_appender_open(appender);
    EXPECT_EQ(ret, -1) << "stream_env_open should fail with parse error";
}

