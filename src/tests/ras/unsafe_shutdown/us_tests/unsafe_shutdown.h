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

#ifndef UNSAFE_SHUTDOWN_H
#define UNSAFE_SHUTDOWN_H

#include "configXML/local_dimm_configuration.h"
#include "gtest/gtest.h"
#include "gtest_utils/gtest_utils.h"
#include "inject_utils.h"
#include "pool_data/pool_data.h"
#include "poolset/poolset_management.h"
#include "shell/i_shell.h"

extern std::unique_ptr<LocalDimmConfiguration> local_dimm_config;
extern std::vector<DimmCollection> local_us_dimm_colls;
extern std::vector<DimmCollection> local_non_us_dimm_colls;

class UnsafeShutdown : public ::testing::Test {
 public:
  IShell shell_;
  InjectManager inject_mgmt_{local_dimm_config->GetTestDir()};

  /* Test values used for checking data consistency on obj/log/blk pools. */
  std::vector<int> obj_data_ = {-2, 0, 12345, 1412, 1231, 23, 432, 34, 3};
  std::vector<int> blk_data_ = {-2, 0, 12345, 1412, 1231, 23, 432, 34, 3};
  std::string log_data_ =
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
      "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
      "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
      "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
      "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
      "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
      "mollit anim id est laborum.";

  std::string GetNormalizedTestName();
  void Repair(std::string pool_file_path);
  void CreatePassedMarker();
  bool PassedOnPreviousPhase();

  ~UnsafeShutdown();

 private:
  std::string GetPassedMarkerPath();
};

#endif  // UNSAFE_SHUTDOWN_H