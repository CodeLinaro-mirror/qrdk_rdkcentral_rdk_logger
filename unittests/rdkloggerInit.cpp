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
#include <unistd.h>
#include "rdk_logger.h"
#include "gtest_app.h"

/*DIABLED some test cases as there's a bug "https://bugzilla.redhat.com/show_bug.cgi?id=1901955" in log4c library, calling log4c_init twice causing segmentation fault, hence keeping rdk_logger_deinit()'s default changes which does not allow calling rdk_logger_init twice*/


TEST(rdkloggerInit, rdkLoggerInit_with_DebugConfFile)
{
  rdk_Error ret = -1;
  char *conf_file = NULL;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,-1)<<"rdk_logger_init failed with return:\""<<ret<<"\" "<<"config_file ini is not found";
}

TEST(rdkloggerInit, DISABLED_rdkLoggerInitDeinit_with_DebugConfFile)
{
  rdk_Error ret = RDK_SUCCESS;
  char *conf_file = NULL;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed with return:\""<<ret<<"\" "<<"config_file ini is not found";
  ret = rdk_logger_deinit();
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"config_file ini is not found";
}

TEST(rdkloggerInit, DISABLED_rdkLoggerDeInit_with_UserConfFile)
{
  rdk_Error ret = RDK_SUCCESS;
  ret = rdk_logger_deinit();
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:"<<ret;
}

TEST(rdkloggerInit, DISABLED_neg_rdkLoggerInit_with_UserConfFile)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed:"<<ret<<", could not open user configuration file:"<<conf_file;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed:"<<ret<<", could not open user configuration file:"<<conf_file;
  ret = rdk_logger_deinit();
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:"<<ret;
}

TEST(rdkloggerInit, rdkLoggerInit_check_enabled)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed:"<<ret<<", could not open user configuration file:"<<conf_file;
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYERROR", RDK_LOG_ERROR);
  EXPECT_EQ(ret, TRUE)<<"rdk_logger_is_logLevel_enabled for RDK_LOG_ERROR failed with return:\""<<ret<<"\"";

  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:"<<ret;
}

TEST(rdkloggerInit, rdkLogger_ReInit_check_enabled)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed:"<<ret<<", could not open user configuration file:"<<conf_file;
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYERROR", RDK_LOG_ERROR);
  EXPECT_EQ(ret,TRUE)<<"rdk_logger_is_logLevel_enabled for RDK_LOG_ERROR failed with return:\""<<ret<<"\"";

  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed:"<<ret<<", could not open user configuration file:"<<conf_file;
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYERROR", RDK_LOG_ERROR);
  EXPECT_EQ(ret,TRUE)<<"rdk_logger_is_logLevel_enabled for RDK_LOG_ERROR failed with return:\""<<ret<<"\"";

 // ret = rdk_logger_deinit();
 // ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:"<<ret;

}

TEST(rdkloggerInit, DISABLED_rdkLoggerReInit_GetSet_enabled_deinit)
{
  rdk_Error ret = RDK_SUCCESS;
   rdk_logger_Bool ret_dbg = TRUE;
  char conf_file[] = GTEST_DEBUG_INI_FILE;

  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed:"<<ret<<", could not open user configuration file:"<<conf_file;
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYERROR", RDK_LOG_ERROR);
  EXPECT_EQ(ret,TRUE)<<"rdk_logger_is_logLevel_enabled for RDK_LOG_ERROR failed with return:\""<<ret<<"\"";
  rdk_logger_msg_printf (RDK_LOG_ERROR, "LOG.RDK.ONLYERROR", "1 gtest case \"%s\"\n",test_info_->name());
  ret_dbg = rdk_logger_enable_logLevel("LOG.RDK.ONLYERROR", RDK_LOG_ERROR, 0);
  EXPECT_EQ(ret_dbg, FALSE)<<"rdk_logger_enable_logLevel failed with return:\""<<ret_dbg<<"\"";
  rdk_logger_msg_printf (RDK_LOG_ERROR, "LOG.RDK.ONLYERROR", "2 cgtest case \"%s\" %s\n",test_info_->name(),"--This log should not print--");
  ret_dbg = rdk_logger_enable_logLevel("LOG.RDK.ONLYERROR", RDK_LOG_ERROR, 1);
  EXPECT_EQ(ret_dbg, TRUE)<<"rdk_logger_enable_logLevel failed with return:\""<<ret_dbg<<"\"";
  rdk_logger_msg_printf (RDK_LOG_ERROR, "LOG.RDK.ONLYERROR", "3 gtest case \"%s\"\n",test_info_->name());
  ret = rdk_logger_deinit();
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:"<<ret;

  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed:"<<ret<<" Could not open user configuration file:"<<conf_file;
  ret = rdk_logger_is_logLevel_enabled( "LOG.RDK.ONLYERROR", RDK_LOG_ERROR);
  EXPECT_EQ(ret,TRUE)<<"rdk_logger_is_logLevel_enabled for RDK_LOG_ERROR failed with return:\""<<ret<<"\"";
  rdk_logger_msg_printf (RDK_LOG_ERROR, "LOG.RDK.ONLYERROR", "4 gtest case \"%s\" %s\n",test_info_->name(),"-- print this--");
  ret = rdk_logger_enable_logLevel("LOG.RDK.ONLYERROR", RDK_LOG_ERROR, 0);
  EXPECT_EQ(ret, FALSE)<<"rdk_logger_enable_logLevel failed with return:\""<<ret<<"\"";
  rdk_logger_msg_printf (RDK_LOG_ERROR, "LOG.RDK.ONLYERROR", "5 gtest case \"%s\" %s\n",test_info_->name(),"--This log should not print--");
  ret_dbg = rdk_logger_enable_logLevel("LOG.RDK.ONLYERROR", RDK_LOG_ERROR, 1);
  EXPECT_EQ(ret_dbg, TRUE)<<"rdk_logger_enable_logLevel failed with return:\""<<ret_dbg<<"\"";
//FIX ME  
  rdk_logger_msg_printf (RDK_LOG_ERROR, "LOG.RDK.ONLYERROR", "gtest case \"%s\" %s\n",test_info_->name(),"-- print this--");
  ret = rdk_logger_deinit();
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:"<<ret;
}

TEST(rdkloggerInit, DISABLED_neg_rdkLogger_Init_DeInit_twice)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed:"<<ret<<", could not open user configuration file:"<<conf_file;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed:"<<ret<<", could not open user configuration file:"<<conf_file;
  ret = rdk_logger_deinit();
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:"<<ret;
  ret = rdk_logger_deinit();
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:"<<ret;
}

TEST(rdkloggerInit, DISABLED_neg_rdkLoggerInit_without_ValidConfFile)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = "No_INI_file.ini";
  ret = rdk_logger_init(conf_file);
  ASSERT_NE(ret,RDK_SUCCESS)<<"rdk_logger_init failed:"<<ret<<", could not open user configuration file:"<<conf_file;
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:"<<ret;
}
