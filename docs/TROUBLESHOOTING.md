# RDK Logger Troubleshooting Guide

## Common Issues and Solutions

### 1. Logger Initialization Failures

#### Issue: `rdk_logger_init()` returns error
**Symptoms:**
```
Logger initialization failed with return: -1
config_file ini is not found
```

**Causes and Solutions:**

**Cause 1: Configuration file not found**
```bash
# Check if config file exists
ls -la /etc/debug.ini
ls -la /nvram/debug.ini
```

**Solution:**
```bash
# Create default configuration
sudo cp debug.ini.sample /etc/debug.ini
sudo chmod 644 /etc/debug.ini
```

**Cause 2: Permission issues**
```bash
# Check file permissions
ls -la /etc/debug.ini
```

**Solution:**
```bash
# Fix permissions
sudo chown root:root /etc/debug.ini
sudo chmod 644 /etc/debug.ini
```

**Cause 3: Invalid configuration format**
```ini
# Wrong format
INVALID.FORMAT=DEBUG

# Correct format  
LOG.RDK.MYAPP=DEBUG
```

#### Issue: Multiple initialization calls
**Problem:**
```c
// This can cause issues
RDK_LOGGER_INIT();
RDK_LOGGER_INIT();  // Second call may fail
```

**Solution:**
```c
static int logger_initialized = 0;

int init_logger_once() {
    if (!logger_initialized) {
        if (RDK_LOGGER_INIT() == RDK_SUCCESS) {
            logger_initialized = 1;
            return 0;
        }
        return -1;
    }
    return 0;  // Already initialized
}
```

### 2. No Log Output

#### Issue: Messages not appearing despite configuration

**Debugging Steps:**

**Step 1: Verify initialization**
```c
#include "rdk_logger.h"

int main() {
    printf("Before RDK_LOGGER_INIT\n");
    rdk_Error ret = RDK_LOGGER_INIT();
    printf("RDK_LOGGER_INIT returned: %d\n", ret);
    
    printf("Testing log output...\n");
    RDK_LOG(RDK_LOG_ERROR, "LOG.RDK.TEST", "Test error message\n");
    RDK_LOG(RDK_LOG_INFO, "LOG.RDK.TEST", "Test info message\n");
    
    rdk_logger_deinit();
    return 0;
}
```

**Step 2: Check configuration**
```bash
# Verify config file content
cat /etc/debug.ini

# Check for your module
grep "LOG.RDK.TEST" /etc/debug.ini

# Check default level
grep "LOG.RDK.DEFAULT" /etc/debug.ini
```

**Step 3: Test log level checking**
```c
// Debug log level checking
const char *module = "LOG.RDK.TEST";
printf("ERROR enabled: %d\n", rdk_dbg_enabled(module, RDK_LOG_ERROR));
printf("WARN enabled: %d\n", rdk_dbg_enabled(module, RDK_LOG_WARN));
printf("INFO enabled: %d\n", rdk_dbg_enabled(module, RDK_LOG_INFO));
printf("DEBUG enabled: %d\n", rdk_dbg_enabled(module, RDK_LOG_DEBUG));
```

**Step 4: Check environment**
```bash
# Verify log4c is available
ldd your_application | grep log4c

# Check if log4c config exists
find /etc -name "*log4c*" 2>/dev/null
```

### 3. Runtime Log Control Issues

#### Issue: `rdklogctrl` not working

**Problem:** Changes don't take effect
```bash
# Command appears to work but no change
rdklogctrl "$(pidof -s myapp)" LOG.RDK.NETWORK DEBUG
```

**Debugging:**

**Step 1: Verify process name**
```bash
# Check exact process name
pgrep -o myapp

# Use exact name from ps output
rdklogctrl "$(pidof -s myapp)" LOG.RDK.NETWORK DEBUG
```

**Step 2: Check process accessibility**
```bash
# Verify process is running as expected user
ps -ef | grep myapp

# Check if process accepts socket connections
ss -xlp | grep '/tmp/.logger_'
```

**Step 3: Test with simple module**
```bash
# Try with a known module first
rdklogctrl "$(pidof -s myapp)" LOG.RDK.DEFAULT INFO
```

#### Issue: Dynamic logging not persistent
**Problem:** Log level changes revert after application restart

**Explanation:** `rdklogctrl` changes are runtime-only. For persistent changes, modify `debug.ini`:

```bash
# For persistent changes
sudo nano /etc/debug.ini

# Add or modify
LOG.RDK.NETWORK=DEBUG
```

### 4. Performance Issues

#### Issue: Logging causing performance degradation

**Problem:** Application becomes slow when debug logging is enabled

**Solution 1: Conditional logging**
```c
// Wrong - always evaluates expensive_function()
RDK_LOG(RDK_LOG_DEBUG, MODULE_NAME, "Debug: %s\n", expensive_function());

// Right - only calls expensive_function() if debug is enabled
if (rdk_dbg_enabled(MODULE_NAME, RDK_LOG_DEBUG)) {
    RDK_LOG(RDK_LOG_DEBUG, MODULE_NAME, "Debug: %s\n", expensive_function());
}
```

**Solution 2: Optimize log levels**
```ini
# Production config - minimal logging
LOG.RDK.DEFAULT=ERROR
LOG.RDK.CRITICAL_MODULE=WARN

# Only enable debug for specific investigation
LOG.RDK.PROBLEM_MODULE=DEBUG
```

**Solution 3: Measure logging overhead**
```c
#include <time.h>

void measure_logging_performance() {
    clock_t start = clock();
    
    for (int i = 0; i < 10000; i++) {
        RDK_LOG(RDK_LOG_INFO, "LOG.RDK.PERF", "Test message %d\n", i);
    }
    
    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("10000 log messages took %f seconds\n", time_spent);
}
```

### 5. Memory Issues

#### Issue: Memory leaks or crashes

**Problem:** Application crashes or shows memory leaks related to logging

**Solution 1: Proper cleanup**
```c
#include <signal.h>

void cleanup_handler(int sig) {
    RDK_LOG(RDK_LOG_NOTICE, "LOG.RDK.MAIN", "Received signal %d, cleaning up\n", sig);
    rdk_logger_deinit();
    exit(0);
}

int main() {
    // Register signal handlers
    signal(SIGTERM, cleanup_handler);
    signal(SIGINT, cleanup_handler);
    
    RDK_LOGGER_INIT();
    
    // Application logic...
    
    rdk_logger_deinit();
    return 0;
}
```

**Solution 2: Check for double-free issues**
```c
// Avoid static string modification
char buffer[256];
snprintf(buffer, sizeof(buffer), "Dynamic message: %d", value);
RDK_LOG(RDK_LOG_INFO, MODULE_NAME, "%s\n", buffer);
```

**Solution 3: Valgrind debugging**
```bash
# Run with valgrind to detect memory issues
valgrind --leak-check=full --show-leak-kinds=all ./your_app

# Look for leaks in librdkloggers or liblog4c
```

### 6. Multi-threading Issues

#### Issue: Garbled log output or crashes in multi-threaded applications

**Problem:** Multiple threads logging simultaneously causing corruption

**Solution 1: RDK Logger is thread-safe (internal synchronization)**
```c
#include <pthread.h>

void* worker_thread(void* arg) {
    int thread_id = *(int*)arg;
    
    // This is safe - RDK Logger handles synchronization
    RDK_LOG(RDK_LOG_INFO, "LOG.RDK.WORKER", 
            "Thread %d: Starting work\n", thread_id);
    
    // Do work...
    
    RDK_LOG(RDK_LOG_INFO, "LOG.RDK.WORKER", 
            "Thread %d: Work completed\n", thread_id);
    
    return NULL;
}
```

**Solution 2: If issues persist, add application-level synchronization**
```c
#include <pthread.h>

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

#define SAFE_LOG(level, module, format, ...) do { \
    pthread_mutex_lock(&log_mutex); \
    RDK_LOG(level, module, format, ##__VA_ARGS__); \
    pthread_mutex_unlock(&log_mutex); \
} while(0)
```

### 7. Configuration Issues

#### Issue: Module-specific settings not working

**Problem:** Settings in debug.ini not taking effect for specific modules

**Debug steps:**

**Step 1: Verify module name format**
```ini
# Wrong formats
MYAPP=DEBUG           # Missing LOG.RDK prefix
LOG.RDK.MyApp=DEBUG   # Case sensitive
LOG.RDK. MYAPP=DEBUG  # Extra space

# Correct format
LOG.RDK.MYAPP=DEBUG
```

**Step 2: Check for syntax errors**
```bash
# Validate config file syntax
grep -n "=" /etc/debug.ini | grep -v "^#"

# Check for invalid characters
hexdump -C /etc/debug.ini | head -20
```

**Step 3: Test with minimal config**
```ini
# Minimal test config
LOG.RDK.DEFAULT=INFO
LOG.RDK.TEST=DEBUG
```

#### Issue: Case sensitivity problems

**Problem:** Module names not matching due to case differences

```c
// These are different modules
RDK_LOG(RDK_LOG_INFO, "LOG.RDK.MyApp", "Message\n");      // MyApp
RDK_LOG(RDK_LOG_INFO, "LOG.RDK.MYAPP", "Message\n");      // MYAPP
RDK_LOG(RDK_LOG_INFO, "LOG.RDK.myapp", "Message\n");      // myapp
```

**Solution:** Use consistent naming convention
```c
// Define module name once
#define MODULE_NAME "LOG.RDK.MYAPP"

// Use throughout application
RDK_LOG(RDK_LOG_INFO, MODULE_NAME, "Message\n");
```

### 8. Build and Linking Issues

#### Issue: Undefined references during compilation

**Problem:**
```
undefined reference to `rdk_logger_init'
undefined reference to `rdk_logger_msg_printf'
```

**Solution 1: Check linking order**
```bash
# Wrong order
gcc -lrdkloggers myapp.c -llog4c -lglib-2.0

# Correct order
gcc myapp.c -lrdkloggers -llog4c -lglib-2.0
```

**Solution 2: Verify library installation**
```bash
# Check if libraries are installed
ldconfig -p | grep rdkloggers
ldconfig -p | grep log4c

# Find library files
find /usr -name "librdkloggers*" 2>/dev/null
find /usr -name "liblog4c*" 2>/dev/null
```

**Solution 3: Use pkg-config if available**
```bash
# Check for pkg-config files
pkg-config --list-all | grep rdk

# If available, use pkg-config
gcc myapp.c `pkg-config --cflags --libs rdklogger`
```

### 9. Log File Issues

#### Issue: Log files not being created or rotated

**Problem:** Using `rdk_logger_ext_init()` but files not appearing

**Debug steps:**

**Step 1: Check directory permissions**
```c
rdk_logger_ext_config_t config = {
    .fileName = "myapp.log",
    .logdir = "/var/log/",      // Check if writable
    .maxSize = 1024 * 1024,
    .maxCount = 5
};

// Test directory access
if (access("/var/log/", W_OK) != 0) {
    perror("Cannot write to /var/log/");
}
```

**Step 2: Use accessible directory**
```c
rdk_logger_ext_config_t config = {
    .fileName = "myapp.log",
    .logdir = "/tmp/",          // Use /tmp for testing
    .maxSize = 1024 * 1024,
    .maxCount = 5
};
```

**Step 3: Check log4c configuration**
```bash
# Look for log4c config files
find /etc -name "*log4c*" 2>/dev/null

# Check log4c configuration
export LOG4C_APPENDER_FILE_FILENAME="/tmp/test.log"
./your_app
```

### 10. Debugging Tools and Techniques

#### Environment Variables
```bash
# Enable log4c debugging
export LOG4C_PRIORITY=debug
export LOG4C_APPENDER=stdout

# Run your application
./your_app
```

#### Debugging Configuration Loading
```c
#include "rdk_logger.h"

void debug_config_loading() {
    printf("Checking config files:\n");
    
    // Check primary config
    if (access("/etc/debug.ini", F_OK) == 0) {
        printf("✓ /etc/debug.ini exists\n");
    } else {
        printf("✗ /etc/debug.ini not found\n");
    }
    
    // Check override config
    if (access("/nvram/debug.ini", F_OK) == 0) {
        printf("✓ /nvram/debug.ini exists (will be used)\n");
    } else {
        printf("✗ /nvram/debug.ini not found\n");
    }
    
    // Test initialization
    rdk_Error ret = RDK_LOGGER_INIT();
    printf("Logger init result: %d (%s)\n", ret, 
           ret == RDK_SUCCESS ? "SUCCESS" : "FAILED");
}
```

#### Network Debugging for rdklogctrl
```bash
# List per-process dynamic logger sockets
ss -xlpn | grep '/tmp/.logger_'

# Verify socket ownership and permissions
ls -l /tmp/.logger_<process_name>.<pid>`
```

### Quick Reference Checklist

When troubleshooting RDK Logger issues:

1. **✓ Check configuration file exists and is readable**
2. **✓ Verify module names follow LOG.RDK.* convention**
3. **✓ Confirm logger initialization succeeded**
4. **✓ Test with simple log levels (ERROR, INFO)**
5. **✓ Check process name for rdklogctrl commands**
6. **✓ Verify library linking is correct**
7. **✓ Test with minimal configuration**
8. **✓ Check file permissions for log directories**
9. **✓ Use debugging tools (valgrind, gdb, strace)**
10. **✓ Enable log4c debugging if needed**

This troubleshooting guide should help resolve most common issues with RDK Logger implementation and usage.
