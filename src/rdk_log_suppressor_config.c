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
 * @brief Reads RFC parameters from the INI config file and populates
 *        rdk_suppressor_config_t.
 *
 * Config file selection (same pattern as cpuprocanalyzer):
 *   /nvram/rdk_log_suppressor.ini   — RFC override, written by PAM on TR-181 Set,
 *                                     survives reboot (used if file exists)
 *   /etc/rdk_log_suppressor.ini     — read-only default shipped by Yocto recipe
 *
 * AC-4: Feature is disabled by default. Enabled only when
 *   FEATURE.RDKLogSuppressor.Enable = true in the selected config file.
 *   Value is fixed for the process lifetime — not re-polled.
 *   A startup INFO log always confirms the resolved state.
 *
 * AC-5: MaxPatternLength read from FEATURE.RDKLogSuppressor.MaxPatternLength.
 *   Valid range [1, 20]. Out-of-range values fall back to 10 with a WARNING.
 *
 * AC-6: history_buffer_size = 2 * max_pattern_length (computed here).
 */

#include "rdk_log_suppressor.h"
#include "rdk_debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Config file paths — override takes precedence when it exists (survives reboot) */
#define SUPPRESSOR_CFG_OVERRIDE "/nvram/rdk_log_suppressor.ini"
#define SUPPRESSOR_CFG_DEFAULT  "/etc/rdk_log_suppressor.ini"

/* INI key names */
#define KEY_ENABLE      "FEATURE.RDKLogSuppressor.Enable"
#define KEY_MAX_PATTERN "FEATURE.RDKLogSuppressor.MaxPatternLength"

/* Module name used for startup INFO/WARNING logs */
#define LOG_MODULE "LOG.RDK.SUPPRESSOR"

/* -----------------------------------------------------------------------
 * Parse one KEY = VALUE line.
 * Returns 1 if the key matches and value is copied to out, 0 otherwise.
 * --------------------------------------------------------------------- */
static int parse_ini_value(const char *line, const char *key,
                            char *out, size_t out_sz)
{
    /* Skip comment and blank lines */
    if (!line || line[0] == '#' || line[0] == '\n' || line[0] == '\r')
        return 0;

    const char *eq = strchr(line, '=');
    if (!eq)
        return 0;

    /* Extract and trim key */
    char kbuf[128];
    int klen = (int)(eq - line);
    if (klen <= 0 || klen >= (int)sizeof(kbuf))
        return 0;
    strncpy(kbuf, line, (size_t)klen);
    kbuf[klen] = '\0';

    /* Trim trailing whitespace from key */
    int i = klen - 1;
    while (i >= 0 && (kbuf[i] == ' ' || kbuf[i] == '\t'))
        kbuf[i--] = '\0';

    if (strcmp(kbuf, key) != 0)
        return 0;

    /* Extract and trim value */
    const char *vstart = eq + 1;
    while (*vstart == ' ' || *vstart == '\t')
        vstart++;

    strncpy(out, vstart, out_sz - 1);
    out[out_sz - 1] = '\0';

    /* Trim trailing whitespace / newline */
    int vlen = (int)strlen(out) - 1;
    while (vlen >= 0 && (out[vlen] == ' ' || out[vlen] == '\t' ||
                          out[vlen] == '\n' || out[vlen] == '\r'))
        out[vlen--] = '\0';

    return 1;
}

/* -----------------------------------------------------------------------
 * Read a single key value from an INI file.
 * Returns 1 on success, 0 if key not found or file not readable.
 * --------------------------------------------------------------------- */
static int ini_get(const char *filepath, const char *key,
                   char *out, size_t out_sz)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp)
        return 0;

    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (parse_ini_value(line, key, out, out_sz))
        {
            found = 1;
            break;
        }
    }
    fclose(fp);
    return found;
}

void rdk_suppressor_read_config(rdk_suppressor_config_t *config)
{
    if (!config)
        return;

    /* --- Defaults (AC-4: disabled by default) --- */
    config->enabled            = false;
    config->max_pattern_length = RDK_SUPPRESSOR_DEFAULT_MAX_PATTERN_LENGTH;

    /* Select config file: nvram override survives reboot; /etc is the shipped default */
    const char *cfg_path = (access(SUPPRESSOR_CFG_OVERRIDE, F_OK) == 0)
                           ? SUPPRESSOR_CFG_OVERRIDE
                           : SUPPRESSOR_CFG_DEFAULT;

    char buf[32];

    /* --- AC-4: Read Enable --- */
    if (ini_get(cfg_path, KEY_ENABLE, buf, sizeof(buf)))
    {
        config->enabled = (strcmp(buf, "true") == 0);
    }
    /* If key is missing, default (false) stays — no warning needed */

    /* --- AC-5: Read MaxPatternLength --- */
    if (ini_get(cfg_path, KEY_MAX_PATTERN, buf, sizeof(buf)))
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

    /* --- AC-6: Derive history buffer size --- */
    config->history_buffer_size = 2u * config->max_pattern_length;

    /* --- AC-4: Startup INFO log always confirms resolved state --- */
    RDK_LOG(RDK_LOG_INFO, LOG_MODULE,
            "RDKLogSuppressor: %s (MaxPatternLength=%u, HistorySize=%u) [cfg: %s]\n",
            config->enabled ? "enabled" : "disabled",
            config->max_pattern_length,
            config->history_buffer_size,
            cfg_path);
}
