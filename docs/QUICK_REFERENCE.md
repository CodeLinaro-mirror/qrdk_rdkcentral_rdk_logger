# RDK Logger Quick Reference

## Essential Functions

### Initialization
```c
#include "rdk_logger.h"

// Standard initialization (uses /etc/debug.ini or /nvram/debug.ini)
RDK_LOGGER_INIT();

// Custom config file
rdk_logger_init("/path/to/custom/debug.ini");

// Extended initialization with file logging
rdk_logger_ext_config_t config = {
    .fileName = "app.log",
    .logdir = "/var/log/",
    .maxSize = 1024*1024,    // 1MB
    .maxCount = 5            // Keep 5 files
};
rdk_logger_ext_init(&config);

// Cleanup
rdk_logger_deinit();
```

### Logging
```c
// Basic logging
RDK_LOG(RDK_LOG_INFO, "LOG.RDK.MYAPP", "Message\n");

// Formatted logging
RDK_LOG(RDK_LOG_ERROR, "LOG.RDK.MYAPP", "Error %d: %s\n", errno, strerror(errno));

// Performance-conscious logging
if (rdk_dbg_enabled("LOG.RDK.MYAPP", RDK_LOG_DEBUG)) {
    char *expensive_data = generate_debug_info();
    RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.MYAPP", "Debug: %s\n", expensive_data);
    free(expensive_data);
}
```

## Log Levels (Priority Order)

| Level | Macro | When to Use |
|-------|-------|-------------|
| FATAL | `RDK_LOG_FATAL` | System crash, unusable |
| ERROR | `RDK_LOG_ERROR` | Operation failures |
| WARN  | `RDK_LOG_WARN` | Potential problems |
| NOTICE| `RDK_LOG_NOTICE` | Important events |
| INFO  | `RDK_LOG_INFO` | General information |
| DEBUG | `RDK_LOG_DEBUG` | Debugging details |
| TRACE | `RDK_LOG_TRACE` | Function tracing |

## Configuration (debug.ini)

```ini
# Default level for all modules
LOG.RDK.DEFAULT=WARNING

# Module-specific levels
LOG.RDK.NETWORK=DEBUG
LOG.RDK.AUDIO=INFO
LOG.RDK.VIDEO=NOTICE
LOG.RDK.TEST=NONE
```

### Config File Locations (Priority Order)
1. `/nvram/debug.ini` (override - highest priority)
2. `/etc/debug.ini` (default)
3. Custom path via `rdk_logger_init(path)`

## Runtime Control

### rdklogctrl Command
```bash
# Set log level for module
rdklogctrl <pid> <module_name> <level>

# Example for PID 1122
rdklogctrl 1122 LOG.RDK.RTMESS DEBUG

# Examples
rdklogctrl "$(pidof -s myapp)" LOG.RDK.NETWORK DEBUG
rdklogctrl "$(pidof -s receiver)" LOG.RDK.AUDIO INFO
rdklogctrl "$(pidof -s player)" LOG.RDK.VIDEO ERROR

# Disable specific level (~ prefix)
rdklogctrl "$(pidof -s myapp)" LOG.RDK.NETWORK ~DEBUG

# Available levels
FATAL ERROR WARN NOTICE INFO DEBUG TRACE NONE
```

### Programmatic Control
```c
// Check if level is enabled
if (rdk_dbg_enabled("LOG.RDK.MYAPP", RDK_LOG_DEBUG)) {
    // Debug logging is enabled
}

// Enable/disable level at runtime  
rdk_logger_enable_logLevel("LOG.RDK.MYAPP", RDK_LOG_DEBUG, TRUE);   // Enable
rdk_logger_enable_logLevel("LOG.RDK.MYAPP", RDK_LOG_DEBUG, FALSE);  // Disable

// Convert string to log level
rdk_LogLevel level = rdk_logger_level_from_string("INFO");
```

## Module Naming Convention

### Correct Format
```c
#define MODULE_NAME "LOG.RDK.MYAPP"
#define NETWORK_MODULE "LOG.RDK.NETWORK" 
#define AUDIO_MODULE "LOG.RDK.AUDIO"
```

### Common Mistakes
```c
// ❌ Wrong - missing LOG.RDK prefix
RDK_LOG(RDK_LOG_INFO, "MYAPP", "Message\n");

// ❌ Wrong - case sensitivity
RDK_LOG(RDK_LOG_INFO, "log.rdk.myapp", "Message\n");

// ❌ Wrong - extra spaces
RDK_LOG(RDK_LOG_INFO, "LOG.RDK. MYAPP", "Message\n");

// ✅ Correct
RDK_LOG(RDK_LOG_INFO, "LOG.RDK.MYAPP", "Message\n");
```

## Common Patterns

### Application Template
```c
#include "rdk_logger.h"
#include <signal.h>

#define MODULE_NAME "LOG.RDK.MYAPP"

void cleanup_handler(int sig) {
    RDK_LOG(RDK_LOG_NOTICE, MODULE_NAME, "Shutting down (signal %d)\n", sig);
    rdk_logger_deinit();
    exit(0);
}

int main() {
    // Setup signal handlers
    signal(SIGTERM, cleanup_handler);
    signal(SIGINT, cleanup_handler);
    
    // Initialize logger
    if (RDK_LOGGER_INIT() != RDK_SUCCESS) {
        fprintf(stderr, "Logger init failed\n");
        return 1;
    }
    
    RDK_LOG(RDK_LOG_NOTICE, MODULE_NAME, "Application started\n");
    
    // Main application logic
    RDK_LOG(RDK_LOG_INFO, MODULE_NAME, "Processing...\n");
    
    // Cleanup
    RDK_LOG(RDK_LOG_NOTICE, MODULE_NAME, "Application finished\n");
    rdk_logger_deinit();
    return 0;
}
```

### Error Handling Pattern
```c
int connect_to_server(const char *hostname, int port) {
    RDK_LOG(RDK_LOG_INFO, MODULE_NAME, "Connecting to %s:%d\n", hostname, port);
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        RDK_LOG(RDK_LOG_ERROR, MODULE_NAME, 
                "Socket creation failed: %s\n", strerror(errno));
        return -1;
    }
    
    // Connection logic...
    
    if (connect_result < 0) {
        RDK_LOG(RDK_LOG_ERROR, MODULE_NAME,
                "Connection to %s:%d failed: %s\n", 
                hostname, port, strerror(errno));
        close(sock);
        return -1;
    }
    
    RDK_LOG(RDK_LOG_NOTICE, MODULE_NAME, 
            "Successfully connected to %s:%d\n", hostname, port);
    return sock;
}
```

### Performance-Conscious Logging
```c
void process_data(const data_t *data) {
    RDK_LOG(RDK_LOG_INFO, MODULE_NAME, "Processing data batch\n");
    
    // Only generate expensive debug info if debug logging is enabled
    if (rdk_dbg_enabled(MODULE_NAME, RDK_LOG_DEBUG)) {
        char debug_info[1024];
        generate_debug_summary(data, debug_info, sizeof(debug_info));
        RDK_LOG(RDK_LOG_DEBUG, MODULE_NAME, "Data details: %s\n", debug_info);
    }
    
    // Trace function entry/exit only if trace is enabled
    if (rdk_dbg_enabled(MODULE_NAME, RDK_LOG_TRACE)) {
        RDK_LOG(RDK_LOG_TRACE, MODULE_NAME, "Entering %s\n", __FUNCTION__);
    }
    
    // Process data...
    
    if (rdk_dbg_enabled(MODULE_NAME, RDK_LOG_TRACE)) {
        RDK_LOG(RDK_LOG_TRACE, MODULE_NAME, "Exiting %s\n", __FUNCTION__);
    }
}
```

## Build Integration

### Makefile
```makefile
# Compiler flags
CFLAGS += -I/usr/include/rdklogger

# Linker flags  
LDFLAGS += -lrdkloggers -llog4c -lglib-2.0

# Example target
myapp: myapp.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
```

### CMake
```cmake
# Find required libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)

# Link libraries
target_link_libraries(myapp
    rdkloggers
    log4c
    ${GLIB_LIBRARIES}
)

# Include directories
target_include_directories(myapp PRIVATE
    /usr/include/rdklogger
    ${GLIB_INCLUDE_DIRS}
)
```

## Debugging Tips

### Check Configuration
```bash
# Verify config file exists and is readable
cat /etc/debug.ini

# Test with specific module
grep "LOG.RDK.MYAPP" /etc/debug.ini

# Check default level
grep "LOG.RDK.DEFAULT" /etc/debug.ini
```

### Test Logging
```c
// Test program to verify logging works
#include "rdk_logger.h"

int main() {
    printf("Testing RDK Logger...\n");
    
    if (RDK_LOGGER_INIT() != RDK_SUCCESS) {
        printf("❌ Init failed\n");
        return 1;
    }
    printf("✅ Init succeeded\n");
    
    const char *module = "LOG.RDK.TEST";
    
    printf("Testing log levels:\n");
    RDK_LOG(RDK_LOG_FATAL, module, "❌ FATAL message\n");
    RDK_LOG(RDK_LOG_ERROR, module, "🔴 ERROR message\n");
    RDK_LOG(RDK_LOG_WARN, module, "🟡 WARN message\n");
    RDK_LOG(RDK_LOG_NOTICE, module, "🔵 NOTICE message\n");
    RDK_LOG(RDK_LOG_INFO, module, "ℹ️ INFO message\n");
    RDK_LOG(RDK_LOG_DEBUG, module, "🐛 DEBUG message\n");
    RDK_LOG(RDK_LOG_TRACE, module, "🔍 TRACE message\n");
    
    rdk_logger_deinit();
    printf("✅ Test completed\n");
    return 0;
}
```

### Runtime Control Test
```bash
# Start your application in background
./myapp &
APP_PID=$!

# Change log level
rdklogctrl "$APP_PID" LOG.RDK.TEST DEBUG

# Check if change took effect (application should show more logs)

# Cleanup
kill $APP_PID
```

## Error Codes

| Code | Meaning | Action |
|------|---------|---------|
| 0 | `RDK_SUCCESS` | Operation successful |
| Non-zero | Various errors | Check config file, permissions, dependencies |

## Dependencies

### Required Libraries
- `librdkloggers` - RDK Logger library
- `liblog4c` - Log4C logging framework  
- `libglib-2.0` - GLib utility library

### Optional Tools
- `rdklogctrl` - Runtime log control utility
- `rdklogmilestone` - Milestone logging utility

This quick reference provides the most commonly needed information for day-to-day RDK Logger usage.
