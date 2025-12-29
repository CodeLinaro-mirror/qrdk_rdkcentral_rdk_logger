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

#include <rdk_logger.h>
#ifdef __cplusplus
extern "C"
{
#endif

void rdk_dbg_priv_init(void);
rdk_Error rdk_dbg_priv_config(const char *pConfigFile);
void rdk_dbg_priv_deinit(void);
rdk_Error rdk_dbg_priv_ext_init (const rdk_logger_ext_config_t* config);
void rdk_dbg_priv_log_msg(rdk_LogLevel level, const char *module_name, const char* format, va_list args);
bool rdk_dbg_priv_log_reconfig(const char *pModuleName, rdk_LogLevel level);

#ifdef __cplusplus
}
#endif

#endif /* _RDK_DBG_PRIV_H */

