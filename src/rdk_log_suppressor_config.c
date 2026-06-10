/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
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
 * @file rdk_log_suppressor_config.c
 * @brief Reads RFC parameters from syscfg and populates rdk_suppressor_config_t.
 *
 * AC-4: Feature is disabled by default. Enabled only when
 *   RDKLogSuppressorEnable == "true" in syscfg.
 *   Value is fixed for the process lifetime — not re-polled.
 *   A startup INFO log always confirms the resolved state.
 *
 * AC-5: MaxPatternLength read from RDKLogSuppressorMaxPatternLength.
 *   Valid range [1, 20]. Out-of-range values fall back to 10 with a WARNING.
 *
 * AC-6: history_buffer_size = 2 * max_pattern_length (computed here).
 */

#include "rdk_log_suppressor.h"
#include "rdk_debug.h"
#include <syscfg/syscfg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYSCFG_KEY_ENABLE      "RDKLogSuppressorEnable"
#define SYSCFG_KEY_MAX_PATTERN "RDKLogSuppressorMaxPatternLength"

/* Module name used for startup INFO/WARNING logs */
#define LOG_MODULE "LOG.RDK.SUPPRESSOR"

void rdk_suppressor_read_config(rdk_suppressor_config_t *config)
{
    char buf[32] = {0};

    if (!config)
        return;

    /* Defaults */
    config->enabled            = false;
    config->max_pattern_length = RDK_SUPPRESSOR_DEFAULT_MAX_PATTERN_LENGTH;

    /* --- AC-4: Read Enable --- */
    if (syscfg_get(NULL, SYSCFG_KEY_ENABLE, buf, sizeof(buf)) == 0)
    {
        config->enabled = (strcmp(buf, "true") == 0);
    }
    else
    {
        RDK_LOG(RDK_LOG_WARN, LOG_MODULE,
                "RDKLogSuppressor: syscfg_get(%s) failed, defaulting to disabled\n",
                SYSCFG_KEY_ENABLE);
    }

    /* --- AC-5: Read MaxPatternLength --- */
    memset(buf, 0, sizeof(buf));
    if (syscfg_get(NULL, SYSCFG_KEY_MAX_PATTERN, buf, sizeof(buf)) == 0)
    {
        int val = atoi(buf);
        if (val >= 1 && val <= (int)RDK_SUPPRESSOR_MAX_PATTERN_LENGTH_LIMIT)
        {
            config->max_pattern_length = (unsigned int)val;
        }
        else
        {
            RDK_LOG(RDK_LOG_WARN, LOG_MODULE,
                    "RDKLogSuppressor: MaxPatternLength value %d out of range [1-%u], "
                    "using default %u\n",
                    val, RDK_SUPPRESSOR_MAX_PATTERN_LENGTH_LIMIT,
                    RDK_SUPPRESSOR_DEFAULT_MAX_PATTERN_LENGTH);
            config->max_pattern_length = RDK_SUPPRESSOR_DEFAULT_MAX_PATTERN_LENGTH;
        }
    }
    else
    {
        RDK_LOG(RDK_LOG_WARN, LOG_MODULE,
                "RDKLogSuppressor: syscfg_get(%s) failed, using default %u\n",
                SYSCFG_KEY_MAX_PATTERN, RDK_SUPPRESSOR_DEFAULT_MAX_PATTERN_LENGTH);
    }

    /* --- AC-6: Derive history buffer size --- */
    config->history_buffer_size = 2u * config->max_pattern_length;

    /* --- AC-4: Startup INFO log always confirms resolved state --- */
    RDK_LOG(RDK_LOG_INFO, LOG_MODULE,
            "RDKLogSuppressor: %s (MaxPatternLength=%u, HistorySize=%u)\n",
            config->enabled ? "enabled" : "disabled",
            config->max_pattern_length,
            config->history_buffer_size);
}
