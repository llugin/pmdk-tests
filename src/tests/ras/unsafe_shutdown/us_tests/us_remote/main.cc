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

#include "configXML/local_dimm_configuration.h"
#include "configXML/remote_dimm_configuration.h"
#include "exit_codes.h"
#include "gtest/gtest.h"
#include "gtest_utils/gtest_utils.h"
#include "inject_utils.h"
#include "shell/i_shell.h"

std::unique_ptr<LocalDimmConfiguration> local_dimm_config{
    new LocalDimmConfiguration()};
std::unique_ptr<RemoteDimmNode> remote_dimm_config;

std::vector<DimmCollection> local_us_dimm_colls;
std::vector<DimmCollection> local_non_us_dimm_colls;
std::vector<std::string> remote_us_dimm_mountpoints;
std::vector<std::string> remote_non_us_dimm_mountpoints;



void InitializeLocalDimms() {
  if (local_dimm_config->GetSize() > 0) {
    local_us_dimm_colls.emplace_back(local_dimm_config.get()->operator[](0));
  }

  if (local_dimm_config->GetSize() > 1) {
    local_non_us_dimm_colls.emplace_back(
        local_dimm_config.get()->operator[](1));
  }

  for (int i = 2; i < local_dimm_config->GetSize(); ++i) {
    local_us_dimm_colls.emplace_back(local_dimm_config.get()->operator[](i));
  }
}


void InitializeRemoteDimms() {
  if (remote_dimm_config->GetSize() > 0) {
    remote_us_dimm_mountpoints.emplace_back(
        remote_dimm_config.get()->operator[](0));
  }
  if (remote_dimm_config->GetSize() > 1) {
    remote_non_us_dimm_mountpoints.emplace_back(
        remote_dimm_config.get()->operator[](1));
  }

  for (int i = 2; i < remote_dimm_config->GetSize(); ++i) {
    remote_us_dimm_mountpoints.emplace_back(
        remote_dimm_config.get()->operator[](i));
  }
}


void InitializeDimms() {
InitializeLocalDimms();
InitializeRemoteDimms();
}
void CleanUp() {
  ApiC::CleanDirectory(local_dimm_config->GetTestDir());
  ApiC::RemoveDirectoryT(local_dimm_config->GetTestDir());

  for (const auto &dimm_collection : *local_dimm_config) {
    ApiC::CleanDirectory(dimm_collection.GetMountpoint());
    ApiC::RemoveDirectoryT(dimm_collection.GetMountpoint());
  }
}

int InjectRemote() {
  std::string remote_agent_path =
      remote_dimm_config->GetBinsDir() + SEPARATOR + "US_REMOTE_AGENT";
  std::string mountpoints_arg;
  for (auto &mnt : remote_us_dimm_mountpoints) {
    mountpoints_arg += mnt + " ";
  }
  IShell shell{remote_dimm_config->GetAddress()};
  std::string cmd = remote_agent_path + " inject " + mountpoints_arg;

  auto out = shell.ExecuteCommand(cmd);
  if (out.GetExitCode() != 0) {
    std::cerr << cmd << ": " << out.GetContent() << std::endl;
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  int ret = 0;
  try {
    if (local_dimm_config->ReadConfigFile() != 0) {
      return 1;
    }
    RemoteDimmConfigurationsCollection remote_dimm_configs;
    if (remote_dimm_configs.ReadConfigFile() != 0) {
      return 1;
    }
    remote_dimm_config = std::unique_ptr<RemoteDimmNode>(new RemoteDimmNode{remote_dimm_configs[0]});

    for (const auto &dimm_collection : *local_dimm_config) {
      ApiC::CreateDirectoryT(dimm_collection.GetMountpoint());
    }

    InitializeDimms();
    ::testing::InitGoogleTest(&argc, argv);

    const std::vector<std::string> args(argv + 1, argv + argc);
    if (std::find(args.begin(), args.end(), "cleanup") != args.end()) {
      CleanUp();
      return 0;
    }

    ret = RUN_ALL_TESTS();

    if (gtest_utils::PartiallyPassed()) {
      ret = exit_codes::partially_passed;
    }

    InjectManager inject_manager{local_dimm_config->GetTestDir()};

    if (inject_manager.RecordUSCAll(local_dimm_config->GetDimmCollections()) !=
        0) {
      return exit_codes::dimm_operation_failed;
    }

    if (std::find(args.begin(), args.end(), "inject") != args.end() &&
        inject_manager.InjectAll(local_us_dimm_colls) != 0 &&
        InjectRemote() != 0) {
      return exit_codes::dimm_operation_failed;
    }
  } catch (const std::exception &e) {
    std::cerr << "Exception was caught: " << e.what() << std::endl;
    ret = 1;
  }

  return ret;
}
