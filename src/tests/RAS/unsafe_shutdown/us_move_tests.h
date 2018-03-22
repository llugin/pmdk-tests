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

#ifndef US_MOVE_TESTS_H
#define US_MOVE_TESTS_H

#include "unsafe_shutdown.h"

#include <algorithm>
#include <memory>
#include <vector>
#include "configXML/local_configuration.h"
#include "dimm_device.h"
#include "gtest/gtest.h"
#include "poolset/poolset.h"

extern std::unique_ptr<LocalConfiguration> local_config;
extern std::vector<DimmDevice> us_dimms;
extern std::vector<DimmDevice> non_us_dimms;
extern std::vector<DimmDevice> remote_us_dimms;
extern std::vector<DimmDevice> remote_non_us_dimms;

struct param_with_us {
  std::string src_pool_dir;
  std::string dest_pool_dir;
  std::vector<DimmDevice> us_dimms;
  bool enough_dimms;
};
std::ostream& operator<<(std::ostream& stream, param_with_us const& m);

struct param_no_us {
  std::string src_pool_dir;
  std::string dest_pool_dir;
  bool enough_dimms;
};

std::ostream& operator<<(std::ostream& stream, param_no_us const& m);
class MoveDirtyPool : public UnsafeShutdown,
                      public ::testing::WithParamInterface<param_with_us> {
 protected:
  void SetUp() override;
  std::string dest_pool_path_;
  std::string src_pool_path_;
};

class MoveCleanPool : public UnsafeShutdown,
                      public ::testing::WithParamInterface<param_with_us> {
 protected:
  std::string dest_pool_path_;
  std::string src_pool_path_;
  void SetUp() override;
};

class MoveCleanPoolWithoutUS
    : public UnsafeShutdown,
      public ::testing::WithParamInterface<param_no_us> {
 protected:
  std::string dest_pool_path_;
  std::string src_pool_path_;
  void SetUp() override;
};

class MoveDirtyPoolWithoutUS
    : public UnsafeShutdown,
      public ::testing::WithParamInterface<param_no_us> {
 protected:
  std::string dest_pool_path_;
  std::string src_pool_path_;
  void SetUp() override;
};

std::vector<param_no_us> GetParamsNoUS();
std::vector<param_with_us> GetParamsWithUS();

#endif  // US_MOVE_TESTS_H
