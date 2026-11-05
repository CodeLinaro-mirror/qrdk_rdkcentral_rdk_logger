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

#if !defined(_RDK_DBG_PRIV_DYNLOG_H)
#define _RDK_DBG_PRIV_DYNLOG_H

#include <stdint.h>
#include <rdk_logger.h>

#define RDK_DYNAMIC_LOG_APP_NAME_MAX 64
#define RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX 64

#define RDK_DYNAMIC_SOCKET_PATH_PREFIX "/tmp/.logger"
#define RDK_DYNAMIC_LOG_SIGNATURE "COMC"
#define RDK_DYNAMIC_LOG_SIGNATURE_LEN 5

typedef struct {
	uint8_t signature[RDK_DYNAMIC_LOG_SIGNATURE_LEN];
	uint8_t log_level;
	char app_name[RDK_DYNAMIC_LOG_APP_NAME_MAX + 1];
	char component_name[RDK_DYNAMIC_LOG_COMPONENT_NAME_MAX + 1];
} rdk_dynamic_log_message_t;

void rdk_dyn_log_init(void);
void rdk_dyn_log_deinit(void);
void rdk_dyn_log_process_pending_request(void);
#endif /* _RDK_DBG_PRIV_DYNLOG_H */
