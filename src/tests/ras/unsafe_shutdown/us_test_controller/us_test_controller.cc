/*
 * Copyright 2018, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "us_test_controller.h"

void USTestController::SetUp() {
  ASSERT_TRUE(WaitForDutsConnection(15));
}

void USTestController::TearDown() {
  for (auto& dut : *ras_config) {
    std::string cmd = dut.GetBinDir() + SEPARATOR + "UNSAFE_SHUTDOWN cleanup";
    std::cout << "Executing command: " << cmd << std::endl;
    auto out = dut.ExecuteCmd(cmd);
    ASSERT_EQ(out.GetExitCode(), 0) << "Cleanup phase failed" << std::endl
                                    << out.GetContent();
  }
}

int USTestController::PhaseExecute(const std::string& filter,
                                 const std::string& arg) {
  auto& primary_dut = ras_config->operator[](0);

  std::string cmd = primary_dut.GetBinDir() + SEPARATOR + "UNSAFE_SHUTDOWN " +
                    arg + " " + filter;
  std::cout << "Executing command: " << cmd << std::endl;

  auto out = primary_dut.ExecuteCmd(cmd);
  std::cout << out.GetContent() << std::endl;
  return out.GetExitCode();
}

bool USTestController::RunPowerCycle() {
  bool ret = true;

  std::vector<std::future<Output<char>>> threads;
  for (auto& dut : *ras_config) {
    threads.emplace_back(std::async(&DUT::PowerCycle, &dut));
  }
  for (const auto& thread : threads) {
    thread.wait();
  }
  for (auto& thread : threads) {
    auto out = thread.get();
    if (out.GetExitCode() != 0) {
      std::cerr << out.GetContent() << std::endl
                << "Exit code: " << out.GetExitCode() << std::endl;
      ret = false;
    }
  }

  return ret;
}

bool USTestController::WaitForDutsConnection(unsigned int timeout) {
  bool ret = true;

  std::vector<std::future<bool>> threads;
  for (auto& dut : *ras_config) {
    threads.emplace_back(std::async(&DUT::WaitForConnection, &dut, timeout));
  }
  for (const auto& thread : threads) {
    thread.wait();
  }
  for (auto& thread : threads) {
    if (!thread.get()) {
      ret = false;
    }
  }

  return ret;
}

TEST_F(USTestController, USC_TEST) {
  /* Phase 1 with US inject */
  std::string before_filter = "--gtest_filter=\"" + *filter + "*_before_us*\"";
  int exit_code = PhaseExecute(before_filter, "inject");
  EXPECT_EQ(0, exit_code) << "Some tests failed";
  ASSERT_NE(exit_codes::dimm_operation_failed, exit_code) << "USC inject failed";
  ASSERT_FALSE(AllTestsFailed(exit_code)) << "All tests failed";

  /* Power cycle */
  ASSERT_TRUE(RunPowerCycle()) << "Power cycle on DUTs failed";
  ASSERT_TRUE(WaitForDutsConnection(15 * 60))
      << "Could not connect to at least one of DUTs";

  /* Phase 2 with US inject */
  std::string after_first_filter =
      "--gtest_filter=\"" + *filter + "*_after_first_us*\"";
  exit_code = PhaseExecute(after_first_filter, "inject");
  EXPECT_EQ(0, exit_code) << "Some tests failed";
  ASSERT_NE(exit_codes::dimm_operation_failed, exit_code) << "USC inject failed";
  ASSERT_FALSE(AllTestsFailed(exit_code)) << "All tests failed";

  /* Power cycle */
  ASSERT_TRUE(RunPowerCycle()) << "Power cycle on DUTs failed";
  ASSERT_TRUE(WaitForDutsConnection(15 * 60))
      << "Could not connect to at least one of DUTs";

  /* Phase 3 */
  std::string after_second_filter =
      "--gtest_filter=\"" + *filter + "*_after_second_us*\"";
  exit_code = PhaseExecute(after_second_filter, "");
  EXPECT_EQ(0, exit_code) << "Some tests failed";
  ASSERT_FALSE(AllTestsFailed(exit_code)) << "All tests failed";
}
