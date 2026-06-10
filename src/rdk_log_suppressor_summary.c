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
 * @file rdk_log_suppressor_summary.c
 * @brief Formats [SUPPRESS] summary lines (AC-2).
 *
 * AC-2 format rules:
 *   Single-message pattern (pattern_length == 1):
 *     [SUPPRESS] "<message>" repeated N times (M messages suppressed for X seconds)
 *
 *   Multi-message pattern (pattern_length >= 2):
 *     [SUPPRESS] L-message pattern repeated N times (M messages suppressed for X seconds)
 *
 * Nothing is emitted if repeat_count == 0 — the caller is responsible for
 * only calling this when something was actually suppressed.
 */

#include "rdk_log_suppressor_summary.h"
#include <stdio.h>
#include <string.h>

void rdk_suppressor_format_summary(
    const rdk_suppressor_state_t  *state,
    const rdk_suppressor_config_t *config,
    char                          *out,
    size_t                         out_sz)
{
    if (!state || !config || !out || out_sz == 0)
        return;

    out[0] = '\0';

    if (state->repeat_count == 0)
        return;

    double duration = difftime(state->last_timestamp, state->first_timestamp);
    unsigned int suppressed = state->repeat_count * (unsigned int)state->pattern_length;

    if (state->pattern_length == 1)
    {
        /* AC-2: single-message format */
        snprintf(out, out_sz,
                 "[SUPPRESS] \"%s\" repeated %u times "
                 "(%u messages suppressed for %.0f seconds)",
                 state->pattern[0].message,
                 state->repeat_count,
                 suppressed,
                 duration);
    }
    else
    {
        /* AC-2: multi-message format */
        snprintf(out, out_sz,
                 "[SUPPRESS] %d-message pattern repeated %u times "
                 "(%u messages suppressed for %.0f seconds)",
                 state->pattern_length,
                 state->repeat_count,
                 suppressed,
                 duration);
    }
}
