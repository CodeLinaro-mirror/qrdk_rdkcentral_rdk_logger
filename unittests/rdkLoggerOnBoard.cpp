#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <cstdio>
#include <cstring>
#include "rdk_logger.h"
#include "gtest_app.h"


// Define test-specific paths
#define TEST_ONBOARDING_LOG_FILE "test_onboarding.log"
#define TEST_DEVICE_ONBOARDED "test_device_onboarded"
#define TEST_DISABLE_ONBOARDING "test_disable_onboarding"

// You might need to redefine these in your test build
#define ONBOARDING_LOG_FILE TEST_ONBOARDING_LOG_FILE
#define DEVICE_ONBOARDED TEST_DEVICE_ONBOARDED
#define DISABLE_ONBOARDING TEST_DISABLE_ONBOARDING


