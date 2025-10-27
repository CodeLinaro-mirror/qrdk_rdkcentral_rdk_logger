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
Test Case : Testing rdk_logger function rdk_logger_msg_printf
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "rdk_logger.h"
#include "gtest_app.h"

TEST(rdkLogLevelFromString, test_log_level_from_string)
{
    rdk_LogLevel level = RDK_LOG_NONE;
    rdk_Error rdk_err = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;

    rdk_err = rdk_logger_init(conf_file);
    ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
    level = rdk_logger_level_from_string("TEST");
    EXPECT_EQ(level, RDK_LOG_NONE)<<"rdk_logger_level_from_string failed with return:\""<<level<<"\"";
}

TEST(rdkLogLevelFromString, ONLYFATAL_log_level_from_string)
{
    rdk_LogLevel level = RDK_LOG_NONE;
    rdk_Error rdk_err = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;

    rdk_err = rdk_logger_init(conf_file);
    ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
    level = rdk_logger_level_from_string("FATAL");
    EXPECT_EQ(level, RDK_LOG_FATAL)<<"rdk_logger_level_from_string failed with return:\""<<level<<"\"";     
}
TEST(rdkLogLevelFromString, ONLYERROR_log_level_from_string)
{
    rdk_LogLevel level = RDK_LOG_NONE;
    rdk_Error rdk_err = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;

    rdk_err = rdk_logger_init(conf_file);
    ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
    level = rdk_logger_level_from_string("ERROR");
    EXPECT_EQ(level, RDK_LOG_ERROR)<<"rdk_logger_level_from_string failed with return:\""<<level<<"\"";     
}
TEST(rdkLogLevelFromString, ONLYWARN_log_level_from_string)
{
    rdk_LogLevel level = RDK_LOG_NONE;
    rdk_Error rdk_err = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;

    rdk_err = rdk_logger_init(conf_file);
    ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
    level = rdk_logger_level_from_string("WARN");
    EXPECT_EQ(level, RDK_LOG_WARN)<<"rdk_logger_level_from_string failed with return:\""<<level<<"\"";     
}
TEST(rdkLogLevelFromString, ONLYNOTICE_log_level_from_string)
{
    rdk_LogLevel level = RDK_LOG_NONE;
    rdk_Error rdk_err = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;

    rdk_err = rdk_logger_init(conf_file);
    ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
    level = rdk_logger_level_from_string("NOTICE");
    EXPECT_EQ(level, RDK_LOG_NOTICE)<<"rdk_logger_level_from_string failed with return:\""<<level<<"\"";     
}
TEST(rdkLogLevelFromString, ONLYINFO_log_level_from_string)
{
    rdk_LogLevel level = RDK_LOG_FATAL;
    rdk_Error rdk_err = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;

    rdk_err = rdk_logger_init(conf_file);
    ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
    level = rdk_logger_level_from_string("INFO");
    EXPECT_EQ(level, RDK_LOG_INFO)<<"rdk_logger_level_from_string failed with return:\""<<level<<"\"";     
}
TEST(rdkLogLevelFromString, ONLYDEBUG_log_level_from_string)
{
    rdk_LogLevel level = RDK_LOG_NONE;
    rdk_Error rdk_err = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;

    rdk_err = rdk_logger_init(conf_file);
    ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
    level = rdk_logger_level_from_string("DEBUG");
    EXPECT_EQ(level, RDK_LOG_DEBUG)<<"rdk_logger_level_from_string failed with return:\""<<level<<"\"";     
}
TEST(rdkLogLevelFromString, ONLYTRACE_log_level_from_string)
{
    rdk_LogLevel level = RDK_LOG_NONE;
    rdk_Error rdk_err = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;

    rdk_err = rdk_logger_init(conf_file);
    ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
    level = rdk_logger_level_from_string("TRACE");
    EXPECT_EQ(level, RDK_LOG_TRACE)<<"rdk_logger_level_from_string failed with return:\""<<level<<"\"";     
}
TEST(rdkLogLevelFromString, ONLYNONE_log_level_from_string)
{
    rdk_LogLevel level = RDK_LOG_NONE;
    rdk_Error rdk_err = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;

    rdk_err = rdk_logger_init(conf_file);
    ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
    level = rdk_logger_level_from_string("NONE");
    EXPECT_EQ(level, RDK_LOG_NONE)<<"rdk_logger_level_from_string failed with return:\""<<level<<"\"";
   
     rdk_err = rdk_logger_deinit();
  ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<rdk_err<<"\" "<<"DEBUG_CONF_FILE is not found";    
}
