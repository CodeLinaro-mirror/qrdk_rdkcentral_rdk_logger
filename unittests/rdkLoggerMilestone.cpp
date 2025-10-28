#define LOGMILESTONE

#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include "rdk_logger_milestone.h"

#ifdef LOGMILESTONE
#define MILESTONE_LOG_FILENAME "/opt/logs/rdk_milestones.log"
#else
#define MILESTONE_LOG_FILENAME "/rdklogs/logs/rdk_milestones.log"
#endif

// Helper to remove the log file before/after each test
void RemoveMilestoneLog() {
    std::remove(MILESTONE_LOG_FILENAME);
}

TEST(RdkLoggerMilestoneTest, GetUptimeMSReturnsPositiveValue) {
    unsigned long long uptime = getUptimeMS();
    EXPECT_GT(uptime, 0ULL);
}

TEST(RdkLoggerMilestoneTest, LogMilestoneWritesToFile) {
    RemoveMilestoneLog();
    const char* test_code = "TEST_CODE";
    logMilestone(test_code);

    std::ifstream infile(MILESTONE_LOG_FILENAME);
    ASSERT_TRUE(infile.is_open());
    std::string line;
    std::getline(infile, line);
    infile.close();

    // The log line should start with the code and have a colon
    ASSERT_NE(line.find(test_code), std::string::npos);
    ASSERT_NE(line.find(":"), std::string::npos);
    RemoveMilestoneLog();
}

TEST(RdkLoggerMilestoneTest, LogMilestoneAppendsToFile) {
    RemoveMilestoneLog();
    logMilestone("CODE1");
    logMilestone("CODE2");

    std::ifstream infile(MILESTONE_LOG_FILENAME);
    ASSERT_TRUE(infile.is_open());

    std::string line1, line2;
    std::getline(infile, line1);
    std::getline(infile, line2);
    infile.close();

    ASSERT_NE(line1.find("CODE1"), std::string::npos);
    ASSERT_NE(line2.find("CODE2"), std::string::npos);

    RemoveMilestoneLog();
}
