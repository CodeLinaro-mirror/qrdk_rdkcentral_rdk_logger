/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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

#if !defined(_RDK_DBG_PRIV_H)
#define _RDK_DBG_PRIV_H

#include <rdk_logger_types.h>
#include <rdk_logger.h>
#include <rdk_error.h>

#ifdef __cplusplus
extern "C"
{
#endif

void rdk_dbg_priv_init(void);
void rdk_dbg_priv_config(void);
void rdk_dbg_priv_deinit(void);
void rdk_dbg_priv_ext_init(const char* logdir, const char* log_file_name, long maxCount, long maxSize, rdk_LogAppenderType appender_type, rdk_LogLevel log_level, rdk_LogLayout layout_name);
void rdk_dbg_priv_shutdown();

void rdk_dbg_priv_log_msg(rdk_LogLevel level, const char *module_name, const char* format, va_list args);
void rdk_dbg_priv_reconfig(const char *pModuleName, const char *pLogLevel);

void rdk_dbg_init();
void rdk_dbg_deinit();

#ifdef __cplusplus
}
#endif

#endif /* _RDK_DBG_PRIV_H */

