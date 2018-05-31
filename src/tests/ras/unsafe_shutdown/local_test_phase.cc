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
#include "local_test_phase.h"

LocalTestPhase::LocalTestPhase() {
  if (local_dimm_config_.ReadConfigFile() != 0) {
    throw std::invalid_argument(
        "Reading config file for local dimm configuration failed");
  }

  if (local_dimm_config_.GetSize() > 0) {
    unsafe_dimm_mountpoints_.emplace_back(local_dimm_config_[0]);
  }

  if (local_dimm_config_.GetSize() > 1) {
    safe_dimm_mountpoints_.emplace_back(local_dimm_config_[1]);
  }

  for (int i = 2; i < local_dimm_config_.GetSize(); ++i) {
    unsafe_dimm_mountpoints_.emplace_back(local_dimm_config_[i]);
  }
}

int LocalTestPhase::SetUp() {
  for (const auto &dimm_collection : local_dimm_config_) {
    if (ApiC::CreateDirectoryT(dimm_collection) != 0) {
      return 1;
    }
  }
  return 0;
}

int LocalTestPhase::Inject() {
#ifdef __linux__
  InjectManager inject_mgmt{local_dimm_config_.GetTestDir(), strategy_};

  if (inject_mgmt.RecordUSC(local_dimm_config_.GetDimmCollections()) != 0) {
    return 1;
  }

  std::vector<DimmCollection> unsafe_dimm_namespaces;
  for (const auto &mountpoint : unsafe_dimm_mountpoints_) {
    unsafe_dimm_namespaces.emplace_back(DimmCollection{mountpoint});
  }

  if (inject_mgmt.Inject(unsafe_dimm_namespaces)) {
    return 1;
  }
#endif


  return 0;
}


int LocalTestPhase::CheckUSC() {
#ifdef __linux__
  InjectManager inject_mgmt{local_dimm_config_.GetTestDir(), strategy_};
  std::vector<DimmCollection> safe_dimm_namespaces;
  std::vector<DimmCollection> unsafe_dimm_namespaces;

  for (auto &mountpoint : unsafe_dimm_mountpoints_) {
    unsafe_dimm_namespaces.emplace_back(DimmCollection{mountpoint});
  }

  for (auto &mountpoint : safe_dimm_mountpoints_) {
    safe_dimm_namespaces.emplace_back(DimmCollection{mountpoint});
  }
  if (!inject_mgmt.SafelyShutdown(safe_dimm_namespaces) ||
      !inject_mgmt.UnsafelyShutdown(unsafe_dimm_namespaces)) {
    return 1;
  }
#endif
  return 0;
}

int LocalTestPhase::CleanUp() {
  ApiC::CleanDirectory(local_dimm_config_.GetTestDir());
  ApiC::RemoveDirectoryT(local_dimm_config_.GetTestDir());

  for (const auto &dimm_collection : local_dimm_config_) {
    ApiC::CleanDirectory(dimm_collection);
    ApiC::RemoveDirectoryT(dimm_collection);
  }
  return 0;
}
