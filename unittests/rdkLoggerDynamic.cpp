#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
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
    rdk_Error ret = RDK_SUCCESS;
    char conf_file[] = GTEST_DEBUG_INI_FILE;	
    EXPECT_EQ(rdk_logger_init(conf_file), 0);

    // Spawn a thread to run rdklogctrl (client)
    pthread_t client_thread;
    ASSERT_EQ(0, pthread_create(&client_thread, nullptr, run_rdklogctrl, nullptr));

    // Wait briefly to allow message to be sent
    usleep(500000); // 0.5 seconds

    // Log a message using the high-level API, which will exercise dynamic logger code
    rdk_logger_msg_printf(RDK_LOG_ERROR, "LOG.RDK.TESTMOD", "Test message for dynamic logger\n");

    // Clean up
    pthread_join(client_thread, nullptr);
}

