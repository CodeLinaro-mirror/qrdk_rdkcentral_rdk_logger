#########################################################################
# Copyright 2024 Comcast Cable Communications Management, LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#########################################################################

cmake_minimum_required(VERSION 3.22)
project(rdk_logger_gtest)
# Find the gtest package
find_package(GTest REQUIRED)

enable_testing()

link_directories("${DEP_LIB_PATH}/")

set(RDKLOGCTRL_PATH "/usr/bin/install/rdklogctrl")
add_definitions(-DRDKLOGCTRL_PATH="${RDKLOGCTRL_PATH}")

# Add the test executable
add_executable(
  rdk_logger_gtest
  rdkLoggerDynamic.cpp
  rdkloggerInit.cpp
  rdkloggerMsgPrintf.cpp
  rdkloggerMsgRaw.cpp
  rdkloggerMsgRaw1.cpp
  rdkloggerMsgVsprintf.cpp
  rdkIsDbgEnabled.cpp
  rdkIsDbgEnable.cpp
  #rdkDbgEnabled.cpp
  rdkLogLevelFromString.cpp
  rdkloggerExtInit.cpp
  rdkLoggerConfigTest.cpp
  rdkLoggerErrorTest.cpp
  #  rdkLoggerDynamicTest.cpp
  rdkLoggerRotationTest.cpp
  rdkLoggerUtilityTest.cpp
  # rdkLoggerPerformanceTest.cpp
  main.cpp
)
target_compile_definitions(rdk_logger_gtest PRIVATE GTEST_DEBUG_INI_FILE="sample_gtest_debug.ini")

install(TARGETS rdk_logger_gtest
    RUNTIME DESTINATION install/bin
)

install (CODE "execute_process (COMMAND ${CMAKE_COMMAND} -E copy Gtest_log4crc ${CMAKE_INSTALL_PREFIX}/log4crc)")

if (NOT GTEST_INCLUDE_DIR)
    target_link_libraries(rdk_logger_gtest  -L${DEP_LIB_PATH} -lrdkloggers -llog4c -l:libgtest.a gcov)
else ()
    target_link_libraries(rdk_logger_gtest -L${DEP_LIB_PATH} -lrdkloggers -llog4c gtest gcov)
endif()

# Link the gtest libraries to the test executable
target_link_libraries(
  rdk_logger_gtest
  GTest::gtest_main
)
include_directories(${DEP_LIB_PATH}/../include)
include(GoogleTest)
gtest_discover_tests(rdk_logger_gtest)
find_package(Threads)
