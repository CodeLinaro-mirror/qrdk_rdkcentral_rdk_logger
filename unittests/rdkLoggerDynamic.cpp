#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include "rdk_dynamic_logger.h"
#include "rdk_logger.h"
#include "gtest_app.h"

// Thread function to invoke rdklogctrl client via system command
void* run_rdklogctrl(void* arg) {
    // You may need to adjust the path if rdklogctrl isn't in $PATH
    const char* cmd = "rdklogctrl Receiver LOG.RDK.TESTMOD ERROR";
    int ret = system(cmd);
    // Optionally check ret for success/failure
    return nullptr;
}

TEST(RdkDynamicLoggerTest, MessageProcessingViaSystem) {
    // Initialize the dynamic logger (server)
    rdk_dyn_log_init();

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, nullptr));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Process any pending requests
    rdk_dyn_log_process_pending_request();

    // Clean up
    pthread_join(client_thread, nullptr);
    rdk_dyn_log_deinit();

    // Successful execution means code coverage hit
    SUCCEED();
}

