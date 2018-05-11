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

#include "unsafe_shutdown.h"

std::string UnsafeShutdown::GetNormalizedTestName() {
  std::string full_test_name = gtest_utils::GetFullTestName();
  string_utils::ReplaceAll(full_test_name, SEPARATOR, std::string{"_"});
  std::vector<std::string> test_suffixes{"_before_us", "_after_first_us",
                                         "_after_second_us"};
  for (const auto &suffix : test_suffixes) {
    string_utils::ReplaceAll(full_test_name, suffix, std::string{""});
  }
  return full_test_name;
}

std::string UnsafeShutdown::GetPassedMarkerPath() {
  return local_dimm_config->GetTestDir() + GetNormalizedTestName() + "_passed";
}

void UnsafeShutdown::Repair(std::string pool_file_path) {
  std::string cmd = "pmempool check -ry " + pool_file_path;
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(0, out.GetExitCode()) << cmd << std::endl << out.GetContent();
}

bool UnsafeShutdown::PassedOnPreviousPhase() {
  bool ret = ApiC::RegularFileExists(GetPassedMarkerPath());
  if (ret) {
    ApiC::RemoveFile(GetPassedMarkerPath());
  }
  return ret;
}

void UnsafeShutdown::CreatePassedMarker() {
  if (gtest_utils::ThisTestPassed()) {
    if (ApiC::CreateFileT(GetPassedMarkerPath(), "") == -1) {
      throw std::invalid_argument("Could not create passed marker file: " +
                                  GetPassedMarkerPath());
    };
  }
}

UnsafeShutdown::~UnsafeShutdown() {
  CreatePassedMarker();
}
