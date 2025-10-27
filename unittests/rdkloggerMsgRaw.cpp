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
Test Case : Testing rdk_logger function rdk_dbg_MsgRaw
*******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rdk_logger.h"
#include "gtest_app.h"


TEST(rdkloggerMsgRaw, logModule_ONLYERROR_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.ONLYERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.ONLYERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.ONLYERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.ONLYERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.ONLYERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.ONLYERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.ONLYERROR", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_ONLYWARNING_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.ONLYWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.ONLYWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.ONLYWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.ONLYWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.ONLYWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.ONLYWARNING" , "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.ONLYWARNING", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_ONLYINFO_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.ONLYINFO", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.ONLYINFO", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.ONLYINFO", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.ONLYINFO", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.ONLYINFO", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.ONLYINFO", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.ONLYINFO", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_ONLYDEBUG_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.ONLYDEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.ONLYDEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.ONLYDEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.ONLYDEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.ONLYDEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.ONLYDEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.ONLYDEBUG", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_ONLYNOTICE_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.ONLYNOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.ONLYNOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.ONLYNOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.ONLYNOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.ONLYNOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.ONLYNOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.ONLYNOTICE", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_ONLYTRACE_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.ONLYTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.ONLYTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.ONLYTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.ONLYTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.ONLYTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.ONLYTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.ONLYTRACE", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}


TEST(rdkloggerMsgRaw, logModule_ONLYALL_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.ONLYALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.ONLYALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.ONLYALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.ONLYALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.ONLYALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.ONLYALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.ONLYALL", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_NODEBUG_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.NODEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.NODEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.NODEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.NODEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.NODEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.NODEBUG", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.NODEBUG", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_NOERROR_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.NOERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.NOERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.NOERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.NOERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.NOERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.NOERROR", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.NOERROR", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_NOWARNING_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.NOWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.NOWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.NOWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.NOWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.NOWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.NOWARNING", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.NOWARNING", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_NONOTICE_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.NONOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.NONOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.NONOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.NONOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.NONOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.NONOTICE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.NONOTICE", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_NOTRACE_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.NOTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.NOTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.NOTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.NOTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.NOTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.NOTRACE", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.NOTRACE", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_NOFATAL_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.NOFATAL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.NOFATAL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.NOFATAL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.NOFATAL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.NOFATAL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.NOFATAL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.NOFATAL", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_NOALL_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.NOALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.NOALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.NOALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.NOALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.NOALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.NOALL", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.NOALL", "gtest case \"%s\"\n",test_info_->name());
  //ret = rdk_logger_deinit();
  //ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}

TEST(rdkloggerMsgRaw, logModule_ALLLOGLEVELS_output_to_file)
{
  rdk_Error ret = RDK_SUCCESS;
  char conf_file[] = GTEST_DEBUG_INI_FILE;
  ret = rdk_logger_init(conf_file);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed"<<ret<<"Could not open user configuration file:"<<conf_file;

  rdk_dbg_MsgRaw (RDK_LOG_ERROR, "LOG.RDK.ALLLOGLEVELS", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_FATAL, "LOG.RDK.ALLLOGLEVELS", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_WARN, "LOG.RDK.ALLLOGLEVELS", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_NOTICE, "LOG.RDK.ALLLOGLEVELS", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_DEBUG, "LOG.RDK.ALLLOGLEVELS", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_INFO, "LOG.RDK.ALLLOGLEVELS", "gtest case \"%s\"\n",test_info_->name());
  rdk_dbg_MsgRaw (RDK_LOG_TRACE, "LOG.RDK.ALLLOGLEVELS", "gtest case \"%s\"\n",test_info_->name());
//  ret = rdk_logger_deinit();
//  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}
