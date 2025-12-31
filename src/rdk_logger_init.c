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

/**
 * @file rdk_logger_init.c
 * This source file contains the APIs for RDK logger initializer.
 */

/**
* @defgroup rdk_logger
* @{
* @defgroup src
* @{
**/


#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdatomic.h>
#include <pthread.h>
#include "rdk_logger.h"
#include "rdk_debug_priv.h"
#include "rdk_dynamic_logger.h"

static pthread_mutex_t gInitMutex = PTHREAD_MUTEX_INITIALIZER;

atomic_bool isLogInited = false;

static void __attribute__((constructor)) _rdk_logger_init (void)
{
    /* Perform Logger Internal Init */
    rdk_dbg_priv_init();

    return;
}

/**
 * @brief Initialize the logger. Sets up the environment variable storage by parsing
 * debug configuration file then Initialize the debug support to the underlying platform.
 *
 * @note  Requests not to send SIGPIPE on errors on stream oriented sockets
 * when the other end breaks the connection. The EPIPE error is still returned.
 *
 * @param[in] debugConfigFile The character pointer variable of debug configuration file.
 *
 * @return Returns 0 if initialization of RDK logger module is successful, else it returns -1.
 */
rdk_Error rdk_logger_init(const char* debugConfigFile)
{
    rdk_Error ret = RDK_SUCCESS;
    pthread_mutex_lock(&gInitMutex);
    if (!atomic_load(&isLogInited))
    {
        if (NULL == debugConfigFile)
        {
            debugConfigFile = DEBUG_CONF_FILE;
        }

        /* Perform Logger Internal Init */
        ret = rdk_dbg_priv_config(debugConfigFile);

        if (RDK_SUCCESS == ret)
        {
            /* Perform Dynamic Logger Internal Init */
            rdk_dyn_log_init();

            atomic_store(&isLogInited, true);
            /**
             * Requests not to send SIGPIPE on errors on stream oriented
             * sockets when the other end breaks the connection. The EPIPE
             * error is still returned.
             */
            signal(SIGPIPE, SIG_IGN);
        }
        else
        {
            printf("Parsing debug config file %s failed\n", debugConfigFile);
        }
    }
    pthread_mutex_unlock(&gInitMutex);
    return ret;
}

rdk_Error rdk_logger_ext_init(const rdk_logger_ext_config_t* config)
{
    rdk_Error ret = RDK_SUCCESS;
    if (!atomic_load(&isLogInited))
    {
        ret = RDK_LOGGER_INIT();
    }

    if (RDK_SUCCESS == ret)
    {
        pthread_mutex_lock(&gInitMutex);
        ret = rdk_dbg_priv_ext_init(config);
        pthread_mutex_unlock(&gInitMutex);
    }

    return ret;
}

/**
 * @brief Cleanup the logger instantiation.
 *
 * @return Returns 0 if the call is successful else return -1.
 */
rdk_Error rdk_logger_deinit()
{
    pthread_mutex_lock(&gInitMutex);
    if (atomic_load(&isLogInited))
    {
        rdk_dyn_log_deinit();
        atomic_store(&isLogInited, false);
    }
    pthread_mutex_unlock(&gInitMutex);

    return RDK_SUCCESS;
}
