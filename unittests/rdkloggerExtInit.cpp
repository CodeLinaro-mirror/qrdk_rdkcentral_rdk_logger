#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rdk_logger.h"
#include "gtest_app.h"



rdk_logger_ext_config_t config = {
    .fileName = "myapp.log",
    .logdir = "/var/log/",
    .maxSize = 1024 * 1024,  // 1MB
    .maxCount = 5            // Keep 5 log files
};

TEST(rdkloggerExtInit, test)
{
  rdk_Error ret = RDK_SUCCESS;
  ret = rdk_logger_ext_init(&config);
  ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_init failed with return:\""<<ret<<"\" "<<"config_file ini is not found";

 // ret = rdk_logger_deinit();
 // ASSERT_EQ(ret,RDK_SUCCESS)<<"rdk_logger_deinit failed with return:\""<<ret<<"\" "<<"DEBUG_CONF_FILE is not found";
}
