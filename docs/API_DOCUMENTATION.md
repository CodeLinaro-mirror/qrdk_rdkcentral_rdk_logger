# RDK Logger API Documentation

## Table of Contents
1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Installation & Build](#installation--build)
4. [Quick Start](#quick-start)
5. [Core APIs](#core-apis)
6. [Configuration](#configuration)
7. [Runtime Control](#runtime-control)
8. [Log Levels](#log-levels)
9. [Utilities](#utilities)
10. [Examples](#examples)
11. [Error Handling](#error-handling)
12. [Best Practices](#best-practices)

## Overview

RDK Logger is a comprehensive logging framework designed for RDK (Reference Design Kit) components. It provides:

- **Centralized Configuration**: Single `debug.ini` file for all components
- **Runtime Control**: Dynamic log level changes without restart
- **Multi-level Logging**: Support for FATAL, ERROR, WARN, NOTICE, INFO, DEBUG, TRACE
- **Module-specific Control**: Independent log levels per component/module
- **Performance Optimized**: Minimal overhead when logging is disabled
- **Thread-safe**: Safe for multi-threaded applications

## Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Application   │───▶│   RDK Logger     │───▶│   Log Output    │
│                 │    │                  │    │  (stdout/file)  │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                               │
                               ▼
                       ┌──────────────────┐
                       │   debug.ini      │
                       │  Configuration   │
                       └──────────────────┘
```

The RDK Logger sits between your application and the actual log output, providing filtering, formatting, and runtime control capabilities.

## Installation & Build

### Prerequisites
- `liblog4c` - Log4C logging library
- `libglib-2.0` - GLib library
- `autotools` - For building from source

### Build Instructions
```bash
# Configure and build
./configure
make

# Install
sudo make install
```

### Linking in Your Project
```bash
# Compile flags
gcc -o myapp myapp.c -lrdkloggers -llog4c -lglib-2.0
```

## Quick Start

### 1. Include Headers
```c
#include "rdk_logger.h"
```

### 2. Initialize Logger
```c
int main() {
    // Initialize with default config file (/etc/debug.ini)
    if (RDK_LOGGER_INIT() != RDK_SUCCESS) {
        fprintf(stderr, "Failed to initialize RDK Logger\n");
        return -1;
    }
    
    // Your application code here
    
    // Clean up
    rdk_logger_deinit();
    return 0;
}
```

### 3. Log Messages
```c
// Basic logging
RDK_LOG(RDK_LOG_INFO, "LOG.RDK.MYAPP", "Application started successfully\n");

// Formatted logging
int port = 8080;
RDK_LOG(RDK_LOG_NOTICE, "LOG.RDK.MYAPP", "Server listening on port %d\n", port);

// Error logging
RDK_LOG(RDK_LOG_ERROR, "LOG.RDK.MYAPP", "Failed to connect to database: %s\n", strerror(errno));
```

## Core APIs

### Initialization Functions

#### `rdk_logger_init()`
```c
rdk_Error rdk_logger_init(const char* debugConfigFile);
```
**Purpose**: Initialize the RDK Logger with a specific configuration file.

**Parameters**:
- `debugConfigFile`: Path to the debug.ini configuration file

**Returns**: `RDK_SUCCESS` on success, error code otherwise

**Example**:
```c
rdk_Error ret = rdk_logger_init("/etc/debug.ini");
if (ret != RDK_SUCCESS) {
    printf("Logger initialization failed\n");
}
```

#### `rdk_logger_ext_init()`
```c
rdk_Error rdk_logger_ext_init(const rdk_logger_ext_config_t* config);
```
**Purpose**: Extended initialization with custom log file configuration.

**Parameters**:
- `config`: Pointer to extended configuration structure

**Configuration Structure**:
```c
typedef struct rdk_logger_ext_config_t {
    char fileName[RDK_LOGGER_EXT_FILENAME_SIZE];  // Log file name
    char logdir[RDK_LOGGER_EXT_LOGDIR_SIZE];      // Log directory path
    long maxSize;                                  // Max file size in bytes
    long maxCount;                                 // Max number of log files
} rdk_logger_ext_config_t;
```

**Example**:
```c
rdk_logger_ext_config_t config = {
    .fileName = "myapp.log",
    .logdir = "/var/log/",
    .maxSize = 1024 * 1024,  // 1MB
    .maxCount = 5            // Keep 5 log files
};
rdk_logger_ext_init(&config);
```

#### `rdk_logger_deinit()`
```c
rdk_Error rdk_logger_deinit(void);
```
**Purpose**: Clean up and deinitialize the logger.

**Returns**: `RDK_SUCCESS` on success

### Logging Functions

#### `RDK_LOG` Macro
```c
#define RDK_LOG rdk_logger_msg_printf
```
**Purpose**: Primary logging macro for formatted messages.

**Syntax**:
```c
RDK_LOG(level, module, format, ...);
```

**Parameters**:
- `level`: Log level (see [Log Levels](#log-levels))
- `module`: Module name string (should match debug.ini entries)
- `format`: Printf-style format string
- `...`: Variable arguments for format string

#### `rdk_logger_msg_printf()`
```c
void rdk_logger_msg_printf(rdk_LogLevel level, const char *module, const char *format, ...);
```
**Purpose**: Core logging function with printf-style formatting.

**Example**:
```c
rdk_logger_msg_printf(RDK_LOG_DEBUG, "LOG.RDK.NETWORK", 
                      "Connection established to %s:%d\n", hostname, port);
```

#### `rdk_logger_msg_vsprintf()`
```c
void rdk_logger_msg_vsprintf(rdk_LogLevel level, const char *module, const char *format, va_list args);
```
**Purpose**: Logging function that accepts a `va_list` for wrapper functions.

**Example**:
```c
void my_log_wrapper(rdk_LogLevel level, const char *module, const char *format, ...) {
    va_list args;
    va_start(args, format);
    rdk_logger_msg_vsprintf(level, module, format, args);
    va_end(args);
}
```

### Query and Control Functions

#### `rdk_logger_is_logLevel_enabled()`
```c
rdk_logger_Bool rdk_logger_is_logLevel_enabled(const char *module, rdk_LogLevel level);
```
**Purpose**: Check if a specific log level is enabled for a module.

**Returns**: `TRUE` if enabled, `FALSE` otherwise

**Example**:
```c
if (rdk_logger_is_logLevel_enabled("LOG.RDK.MYAPP", RDK_LOG_DEBUG)) {
    // Expensive debug computation only if debug logging is enabled
    char *debug_info = generate_debug_info();
    RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.MYAPP", "Debug info: %s\n", debug_info);
    free(debug_info);
}
```

#### `rdk_logger_enable_logLevel()`
```c
rdk_logger_Bool rdk_logger_enable_logLevel(const char *module, rdk_LogLevel logLevel, rdk_logger_Bool enableLogLvl);
```
**Purpose**: Dynamically enable or disable a log level for a module at runtime.

**Parameters**:
- `module`: Module name
- `logLevel`: Log level to modify
- `enableLogLvl`: `TRUE` to enable, `FALSE` to disable

**Returns**: `TRUE` if successful

**Example**:
```c
// Enable debug logging temporarily
rdk_logger_enable_logLevel("LOG.RDK.MYAPP", RDK_LOG_DEBUG, TRUE);

// ... debug operations ...

// Disable debug logging
rdk_logger_enable_logLevel("LOG.RDK.MYAPP", RDK_LOG_DEBUG, FALSE);
```

### Utility Functions

#### `rdk_logger_level_from_string()`
```c
rdk_LogLevel rdk_logger_level_from_string(const char* level);
```
**Purpose**: Convert string representation to log level enum.

**Parameters**:
- `level`: String like "INFO", "DEBUG", "ERROR"

**Returns**: Corresponding `rdk_LogLevel` value, or `RDK_LOG_NONE` if invalid

**Example**:
```c
rdk_LogLevel level = rdk_logger_level_from_string("DEBUG");
if (level != RDK_LOG_NONE) {
    rdk_logger_enable_logLevel("LOG.RDK.MYAPP", level, TRUE);
}
```

#### `rdk_logger_log_onboard()`
```c
void rdk_logger_log_onboard(const char *module, const char *msg, ...);
```
**Purpose**: Special logging function for onboard/milestone logging.

**Example**:
```c
rdk_logger_log_onboard("LOG.RDK.SYSTEM", "System initialization completed");
```

## Configuration

### debug.ini File Format

The `debug.ini` file controls logging behavior for all modules:

```ini
##########################################################################
# RDK Logger Configuration
# Format: LOG.RDK.<MODULE>=<LEVEL>
##########################################################################

# Default log level for all modules
LOG.RDK.DEFAULT=WARNING

# Module-specific log levels
LOG.RDK.NETWORK=DEBUG      # Network module: debug and above
LOG.RDK.DATABASE=INFO      # Database module: info and above  
LOG.RDK.SECURITY=ERROR     # Security module: error and above
LOG.RDK.TESTMODULE=NONE    # Test module: no logging
```

### Configuration Locations

1. **Primary**: `/etc/debug.ini`
2. **Override**: `/nvram/debug.ini` (takes precedence if exists)
3. **Custom**: Specified in `rdk_logger_init()` call

### Module Naming Convention

- **RDK Components**: Must start with `LOG.RDK.`
- **Example**: `LOG.RDK.NETWORK`, `LOG.RDK.AUDIO`, `LOG.RDK.VIDEO`
- **Default Module**: `LOG.RDK.DEFAULT` sets the fallback level

## Runtime Control

### Using rdklogctrl Utility

The `rdklogctrl` command-line utility allows runtime modification of log levels:

```bash
# Set log level for a module
rdklogctrl <pid> <module_name> <log_level>

# Examples
rdklogctrl "$(pidof -s myapp)" LOG.RDK.NETWORK DEBUG
rdklogctrl "$(pidof -s receiver)" LOG.RDK.AUDIO INFO
rdklogctrl "$(pidof -s player)" LOG.RDK.VIDEO ERROR

# Disable specific log level (using ~ prefix)
rdklogctrl "$(pidof -s myapp)" LOG.RDK.NETWORK ~DEBUG
```

**Parameters**:
- `pid`: Process ID of the target application; `rdklogctrl` resolves its name from `/proc`
- `module_name`: Module name (must match debug.ini format)
- `log_level`: FATAL, ERROR, WARN, NOTICE, INFO, DEBUG, TRACE, NONE

### Programmatic Runtime Control

```c
// Enable debug logging at runtime
rdk_logger_enable_logLevel("LOG.RDK.MYAPP", RDK_LOG_DEBUG, TRUE);

// Check if logging is enabled before expensive operations
if (rdk_dbg_enabled("LOG.RDK.MYAPP", RDK_LOG_TRACE)) {
    generate_detailed_trace();
}
```

## Log Levels

### Available Log Levels

| Level | Value | Description | Use Case |
|-------|-------|-------------|----------|
| `RDK_LOG_FATAL` | 0 | System unusable | Critical failures, system crash |
| `RDK_LOG_ERROR` | 1 | Error conditions | Errors that affect functionality |
| `RDK_LOG_WARN` | 2 | Warning conditions | Potential problems, degraded performance |
| `RDK_LOG_NOTICE` | 3 | Normal significant condition | Important normal events |
| `RDK_LOG_INFO` | 4 | Informational messages | General application flow |
| `RDK_LOG_DEBUG` | 5 | Debug-level messages | Detailed debugging information |
| `RDK_LOG_TRACE` | 6 | Trace-level messages | Very detailed execution tracing |
| `RDK_LOG_NONE` | 7 | No logging | Disable all logging |

### Log Level Hierarchy

Setting a log level enables that level and all higher priority levels:

```
TRACE → DEBUG → INFO → NOTICE → WARN → ERROR → FATAL
```

**Example**: Setting `DEBUG` enables DEBUG, INFO, NOTICE, WARN, ERROR, and FATAL messages.

### Legacy Compatibility

For backward compatibility with legacy RDK components:

```c
#define RDK_LOG_TRACE1 RDK_LOG_TRACE
#define RDK_LOG_TRACE2 RDK_LOG_TRACE
// ... up to RDK_LOG_TRACE9
```

## Utilities

### rdklogctrl - Runtime Log Control

**Location**: `utils/rdklogctrl`

**Purpose**: Command-line utility for modifying log levels of running processes.

**Usage**:
```bash
rdklogctrl <pid> <module_name> <log_level>
```

**Examples**:
```bash
# Enable debug logging for network module
rdklogctrl "$(pidof -s receiver)" LOG.RDK.NETWORK DEBUG

# Disable all logging for test module  
rdklogctrl "$(pidof -s myapp)" LOG.RDK.TEST NONE

# Turn off only error logs (keep others)
rdklogctrl "$(pidof -s player)" LOG.RDK.AUDIO ~ERROR
```

### rdklogmilestone - Milestone Utility

**Location**: `utils/rdklogmilestone.c`

**Purpose**: C utility for milestone logging from applications.

**Usage**:
```bash
rdklogmilestone "APPLICATION_READY"
```

## Examples

### Basic Application Integration

```c
#include <stdio.h>
#include <stdlib.h>
#include "rdk_logger.h"

#define MODULE_NAME "LOG.RDK.MYAPP"

int main() {
    // Initialize logger
    if (RDK_LOGGER_INIT() != RDK_SUCCESS) {
        fprintf(stderr, "Failed to initialize RDK Logger\n");
        return EXIT_FAILURE;
    }
    
    // Log application startup
    RDK_LOG(RDK_LOG_NOTICE, MODULE_NAME, "Application starting up\n");
    
    // Example operations with different log levels
    RDK_LOG(RDK_LOG_INFO, MODULE_NAME, "Initializing components\n");
    
    // Conditional debug logging for performance
    if (rdk_dbg_enabled(MODULE_NAME, RDK_LOG_DEBUG)) {
        RDK_LOG(RDK_LOG_DEBUG, MODULE_NAME, "Debug mode enabled\n");
    }
    
    // Simulated error condition
    int error_code = 42;
    if (error_code != 0) {
        RDK_LOG(RDK_LOG_ERROR, MODULE_NAME, 
                "Operation failed with error code %d\n", error_code);
    }
    
    // Log application shutdown
    RDK_LOG(RDK_LOG_NOTICE, MODULE_NAME, "Application shutting down\n");
    
    rdk_logger_deinit();
    return EXIT_SUCCESS;
}
```

### Network Service Example

```c
#include "rdk_logger.h"
#include <sys/socket.h>
#include <netinet/in.h>

#define NET_MODULE "LOG.RDK.NETWORK"

int start_server(int port) {
    int server_fd;
    struct sockaddr_in address;
    
    RDK_LOG(RDK_LOG_INFO, NET_MODULE, "Starting server on port %d\n", port);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        RDK_LOG(RDK_LOG_ERROR, NET_MODULE, 
                "Failed to create socket: %s\n", strerror(errno));
        return -1;
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        RDK_LOG(RDK_LOG_ERROR, NET_MODULE, 
                "Failed to bind to port %d: %s\n", port, strerror(errno));
        close(server_fd);
        return -1;
    }
    
    if (listen(server_fd, 10) < 0) {
        RDK_LOG(RDK_LOG_ERROR, NET_MODULE, 
                "Failed to listen on socket: %s\n", strerror(errno));
        close(server_fd);
        return -1;
    }
    
    RDK_LOG(RDK_LOG_NOTICE, NET_MODULE, 
            "Server successfully started and listening on port %d\n", port);
    
    return server_fd;
}

void handle_client(int client_fd, struct sockaddr_in *client_addr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, INET_ADDRSTRLEN);
    
    RDK_LOG(RDK_LOG_INFO, NET_MODULE, 
            "New client connected from %s:%d\n", 
            client_ip, ntohs(client_addr->sin_port));
    
    // Handle client communication...
    
    RDK_LOG(RDK_LOG_DEBUG, NET_MODULE, 
            "Processing request from %s\n", client_ip);
    
    // When done
    RDK_LOG(RDK_LOG_INFO, NET_MODULE, 
            "Client %s disconnected\n", client_ip);
    close(client_fd);
}
```

### Configuration File Example

Create `/etc/debug.ini`:
```ini
##########################################################################
# Production Configuration
##########################################################################

# Default level - only warnings and errors
LOG.RDK.DEFAULT=WARN

# Network debugging enabled
LOG.RDK.NETWORK=DEBUG

# Application flow information
LOG.RDK.MYAPP=INFO

# Security - errors only
LOG.RDK.SECURITY=ERROR

# Disable test modules in production
LOG.RDK.TEST=NONE
LOG.RDK.UNITTEST=NONE

# Media components
LOG.RDK.AUDIO=NOTICE
LOG.RDK.VIDEO=NOTICE
```

### Wrapper Function Example

```c
#include "rdk_logger.h"
#include <stdarg.h>

// Custom logging wrapper with timestamp
void my_log(rdk_LogLevel level, const char *module, const char *function, 
            int line, const char *format, ...) {
    if (!rdk_dbg_enabled(module, level)) {
        return;  // Early return if logging disabled
    }
    
    va_list args;
    va_start(args, format);
    
    // Create enhanced format with function and line info
    char enhanced_format[512];
    snprintf(enhanced_format, sizeof(enhanced_format), 
             "[%s:%d] %s", function, line, format);
    
    rdk_logger_msg_vsprintf(level, module, enhanced_format, args);
    va_end(args);
}

// Convenience macro
#define MY_LOG(level, module, format, ...) \
    my_log(level, module, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

// Usage
int main() {
    RDK_LOGGER_INIT();
    
    MY_LOG(RDK_LOG_INFO, "LOG.RDK.MYAPP", "Application started\n");
    MY_LOG(RDK_LOG_DEBUG, "LOG.RDK.MYAPP", "Variable value: %d\n", 42);
    
    rdk_logger_deinit();
    return 0;
}
```

## Error Handling

### Return Codes

All initialization functions return `rdk_Error` type:

```c
typedef uint32_t rdk_Error;
#define RDK_SUCCESS 0
```

**Common Error Scenarios**:
- Configuration file not found
- Invalid configuration format
- Memory allocation failures
- Log4C initialization errors

### Error Handling Pattern

```c
rdk_Error init_logging(const char *config_file) {
    rdk_Error ret = rdk_logger_init(config_file);
    if (ret != RDK_SUCCESS) {
        fprintf(stderr, "Logger initialization failed: %u\n", ret);
        fprintf(stderr, "Falling back to stdout logging\n");
        // Implement fallback logging mechanism
        return ret;
    }
    return RDK_SUCCESS;
}
```

### Graceful Degradation

```c
int main() {
    // Try to initialize logger
    if (RDK_LOGGER_INIT() != RDK_SUCCESS) {
        fprintf(stderr, "Warning: RDK Logger initialization failed\n");
        fprintf(stderr, "Continuing with fprintf logging\n");
        
        // Application can continue with basic printf/fprintf
        printf("Application started (no RDK logging)\n");
    } else {
        RDK_LOG(RDK_LOG_NOTICE, "LOG.RDK.MYAPP", "Application started\n");
    }
    
    // Rest of application...
    
    return 0;
}
```

## Best Practices

### 1. Module Naming
```c
// Good - descriptive and consistent
#define AUDIO_MODULE "LOG.RDK.AUDIO"
#define NETWORK_MODULE "LOG.RDK.NETWORK"
#define DATABASE_MODULE "LOG.RDK.DATABASE"

// Avoid - generic or unclear names  
#define MODULE "LOG.RDK.APP"
#define MY_MODULE "LOG.RDK.THING"
```

### 2. Performance Optimization
```c
// Check if logging is enabled before expensive operations
if (rdk_dbg_enabled("LOG.RDK.MYAPP", RDK_LOG_DEBUG)) {
    char *expensive_debug_info = generate_debug_dump();
    RDK_LOG(RDK_LOG_DEBUG, "LOG.RDK.MYAPP", "Debug: %s\n", expensive_debug_info);
    free(expensive_debug_info);
}

// Use appropriate log levels
RDK_LOG(RDK_LOG_FATAL, module, "System crash: %s\n", reason);    // System unusable
RDK_LOG(RDK_LOG_ERROR, module, "Failed to connect: %s\n", err);  // Error conditions
RDK_LOG(RDK_LOG_WARN, module, "Retrying connection\n");          // Warnings
RDK_LOG(RDK_LOG_NOTICE, module, "Service started\n");           // Important events
RDK_LOG(RDK_LOG_INFO, module, "Processing request\n");          // General info
RDK_LOG(RDK_LOG_DEBUG, module, "Variable x = %d\n", x);         // Debug details
RDK_LOG(RDK_LOG_TRACE, module, "Entering function\n");          // Function tracing
```

### 3. Thread Safety
```c
// RDK Logger is thread-safe, but avoid race conditions in your logic
void worker_thread(void *arg) {
    int thread_id = *(int*)arg;
    
    RDK_LOG(RDK_LOG_INFO, "LOG.RDK.WORKER", 
            "Worker thread %d started\n", thread_id);
    
    // Worker logic...
    
    RDK_LOG(RDK_LOG_INFO, "LOG.RDK.WORKER", 
            "Worker thread %d finished\n", thread_id);
}
```

### 4. Configuration Management
```c
// Check for override config file
const char* get_config_file() {
    if (access("/nvram/debug.ini", F_OK) == 0) {
        return "/nvram/debug.ini";  // Development/debug config
    }
    return "/etc/debug.ini";        // Production config
}

// Initialize with appropriate config
rdk_Error ret = rdk_logger_init(get_config_file());
```

### 5. Structured Logging
```c
// Include relevant context in log messages
RDK_LOG(RDK_LOG_ERROR, "LOG.RDK.NETWORK", 
        "Connection failed: host=%s, port=%d, error=%s\n", 
        hostname, port, strerror(errno));

// Use consistent formats for similar events
RDK_LOG(RDK_LOG_INFO, "LOG.RDK.SERVICE", 
        "SERVICE_START: name=%s, pid=%d, version=%s\n", 
        service_name, getpid(), version);
```

### 6. Cleanup
```c
// Always clean up in signal handlers and exit paths
void signal_handler(int sig) {
    RDK_LOG(RDK_LOG_NOTICE, "LOG.RDK.MYAPP", 
            "Received signal %d, shutting down\n", sig);
    rdk_logger_deinit();
    exit(0);
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    RDK_LOGGER_INIT();
    
    // Application logic...
    
    rdk_logger_deinit();
    return 0;
}
```

---

**Note**: This documentation covers RDK Logger API version as implemented in the current codebase. For the most up-to-date information, refer to the source code and header files.
