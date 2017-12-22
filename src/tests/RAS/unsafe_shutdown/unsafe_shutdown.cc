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
  auto &test_info = GetTestInfo();
  std::string test_name = std::string{test_info.test_case_name()} + "_" +
                          std::string{test_info.name()};
  test_name = std::regex_replace(test_name, std::regex(SEPARATOR), "_");

  std::string test_suffixes = "_before_us|_after_first_us|_after_second_us";
  test_name = std::regex_replace(test_name, std::regex(test_suffixes), "");
  return test_name;
}

void UnsafeShutdown::StampPassedResult() {
  if (GetTestInfo().result()->Passed()) {
    std::string file_name =
        local_config->GetTestDir() + GetNormalizedTestName() + "_passed";
    std::fstream passed_file(file_name, std::ios::out);
    passed_file.close();
  }
}

bool UnsafeShutdown::PassedOnPreviousPhase() {
  std::string passed_file_path =
      local_config->GetTestDir() + GetNormalizedTestName() + "_passed";
  bool ret = ApiC::RegularFileExists(passed_file_path);
  ApiC::RemoveFile(passed_file_path);
  return ret;
}

Output<char> UnsafeShutdown::CreateRemotePoolsetFile(Poolset &poolset,
                                                     std::string host) {
  std::string c;
  for (const auto &line : poolset.GetContent()) {
    c += line + '\n';
  }

  SshRunner ssh_runner{host};
  auto out =
      ssh_runner.ExecuteRemote("echo \'" + c + "\' > " + poolset.GetFullPath());
  return out;
}

void UnsafeShutdown::Repair(std::string pool_file_path) {
  std::string cmd = "pmempool check -ry " + pool_file_path;
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(0, out.GetExitCode()) << cmd << std::endl << out.GetContent();
}

UnsafeShutdown::~UnsafeShutdown() {
  StampPassedResult();
}

const testing::TestInfo &UnsafeShutdown::GetTestInfo() {
  return *::testing::UnitTest::GetInstance()->current_test_info();
}
