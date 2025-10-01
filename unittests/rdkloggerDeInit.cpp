#include <gtest/gtest.h>
#include "rdk_logger.h"  // Assuming rdk_dbg_deinit is declared here
#include "../src/include/rdk_debug_priv.h"

TEST(rdkDbgDeinit, deinit_after_init)
{
    rdk_Error ret = rdk_dbg_init(); // Assuming this exists; if not, skip
    ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_dbg_init failed with return: " << ret;

    ret = rdk_dbg_deinit();
    ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_dbg_deinit failed with return: " << ret;
}

TEST(rdkDbgDeinit, deinit_without_init)
{
    rdk_Error ret = rdk_dbg_deinit();
    ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_dbg_deinit failed when called without init, return: " << ret;
}

TEST(rdkDbgDeinit, deinit_multiple_calls)
{
    rdk_Error ret = rdk_dbg_init(); // Assuming this exists; if not, skip
    ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_dbg_init failed with return: " << ret;

    ret = rdk_dbg_deinit();
    ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_dbg_deinit first call failed with return: " << ret;

    ret = rdk_dbg_deinit();
    ASSERT_EQ(ret, RDK_SUCCESS) << "rdk_dbg_deinit second call failed with return: " << ret;
}
