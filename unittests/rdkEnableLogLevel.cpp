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
#if 1
TEST(rdkEnableLogLevel, NULL_log_enabled)
{

  rdk_logger_Bool ret =  TRUE;
  rdk_Error rdk_err = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;

  rdk_err = rdk_logger_init(conf_file);
  ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
  ret = rdk_logger_enable_logLevel(NULL, RDK_LOG_FATAL, TRUE);
  EXPECT_EQ(ret, FALSE)<<"rdk_logger_enabled_loglevel failed with return:\""<<ret<<"\"";
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}
#endif
TEST(rdkEnableLogLevel, ONLYFATAL_log_enabled)
{

  rdk_logger_Bool ret =  TRUE;
  rdk_Error rdk_err = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;

  rdk_err = rdk_logger_init(conf_file);
  ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
  ret = rdk_logger_enable_logLevel("LOG.RDK.ONLYFATAL", RDK_LOG_FATAL, TRUE);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_enabled_loglevel failed with return:\""<<ret<<"\"";
  rdk_logger_msg_printf(RDK_LOG_FATAL, "LOG.RDK.ONLYFATAL", "gtest case \"%s\"\n", test_info_->name());
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYFATAL", RDK_LOG_FATAL);
  rdk_logger_msg_printf(RDK_LOG_FATAL, "LOG.RDK.ONLYFATAL", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}


TEST(rdkEnableLogLevel, ONLYERROR_log_enabled)
{
  rdk_logger_Bool ret =  TRUE;
  rdk_Error rdk_err = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  rdk_err = rdk_logger_init(conf_file);
  ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
  ret = rdk_logger_enable_logLevel("LOG.RDK.ONLYERROR", RDK_LOG_ERROR, TRUE);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_enabled_loglevel failed with return:\""<<ret<<"\"";
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYERROR", RDK_LOG_ERROR);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_is_logLevel_enabled for RDK_LOG_ERROR failed with return:\""<<ret<<"\"";
  rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.ONLYERROR", "gtest case \"%s\"\n",test_info_->name());
}
TEST(rdkEnableLogLevel, ONLYWARNING_log_enabled)
{
  rdk_logger_Bool ret = TRUE;
  rdk_Error rdk_err = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  rdk_err = rdk_logger_init(conf_file);
  ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
  ret = rdk_logger_enable_logLevel("LOG.RDK.ONLYWARNING", RDK_LOG_WARN, TRUE);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_enabled_loglevel failed with return:\""<<ret<<"\"";
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYWARNING", RDK_LOG_WARN);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_is_logLevel_enabled failed with return:\""<<ret<<"\"";
  rdk_logger_msg_printf(RDK_LOG_WARN, "LOG.RDK.ONLYWARNING", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}
TEST(rdkEnableLogLevel, ONLYINFO_log_enabled)
{
  rdk_logger_Bool ret =  TRUE;
  rdk_Error rdk_err = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  rdk_err = rdk_logger_init(conf_file);
  ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
  ret = rdk_logger_enable_logLevel("LOG.RDK.ONLYINFO", RDK_LOG_INFO, TRUE);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_enabled_loglevel failed with return:\""<<ret<<"\"";
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYINFO", RDK_LOG_INFO);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_is_logLevel_enabled failed with return:\""<<ret<<"\"";
  rdk_logger_msg_printf(RDK_LOG_INFO, "LOG.RDK.ONLYINFO", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}
TEST(rdkEnableLogLevel, ONLYDEBUG_log_enabled)
{
  rdk_logger_Bool ret =  TRUE;
  rdk_Error rdk_err = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  rdk_err = rdk_logger_init(conf_file);
  ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
  ret = rdk_logger_enable_logLevel("LOG.RDK.ONLYDEBUG", RDK_LOG_DEBUG, TRUE);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_enabled_loglevel failed with return:\""<<ret<<"\"";
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYDEBUG", RDK_LOG_DEBUG);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_is_logLevel_enabled failed with return:\""<<ret<<"\"";
  rdk_logger_msg_printf(RDK_LOG_DEBUG, "LOG.RDK.ONLYDEBUG", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";

}
TEST(rdkEnableLogLevel, ONLYNOTICE_log_enabled)
{
  rdk_logger_Bool ret = TRUE;
  rdk_Error rdk_err = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  rdk_err = rdk_logger_init(conf_file);
  ASSERT_EQ(rdk_err,RDK_SUCCESS)<<"rdk_logger_init failed with err:\""<<rdk_err<<"\", Could not open user configuration file:"<<conf_file;
    ret = rdk_logger_enable_logLevel("LOG.RDK.ONLYNOTICE", RDK_LOG_NOTICE, TRUE);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_enabled_loglevel failed with return:\""<<ret<<"\"";
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYNOTICE", RDK_LOG_NOTICE);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_is_logLevel_enabled failed with return:\""<<ret<<"\"";
  rdk_logger_msg_printf(RDK_LOG_NOTICE, "LOG.RDK.ONLYNOTICE", "gtest case \"%s\"\n",test_info_->name());
//  ret = rdk_logger_deinit();
//  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}
