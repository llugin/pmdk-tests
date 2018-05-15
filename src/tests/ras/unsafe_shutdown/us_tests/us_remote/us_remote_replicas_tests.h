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

#ifndef US_REMOTE_REPLICAS_TESTS_H
#define US_REMOTE_REPLICAS_TESTS_H

#include "configXML/remote_dimm_configuration.h"
#include "unsafe_shutdown.h"

extern std::unique_ptr<RemoteDimmNode> remote_dimm_config;
extern std::vector<std::string> remote_us_dimm_mountpoints;
extern std::vector<std::string> remote_non_us_dimm_mountpoints;

struct remote_poolset {
  std::string host = remote_dimm_config->GetAddress();
  std::vector<std::string> us_dimm_mountpoints;
  Poolset poolset;
  std::string GetReplicaLine();
};

struct remote_poolset_tc {
  std::vector<remote_poolset> remote_poolsets;
  Poolset poolset;
  std::vector<DimmCollection> us_dimm_colls;
  bool enough_dimms;
  bool is_syncable;
};

class SyncRemoteReplica
    : public UnsafeShutdown,
      public ::testing::WithParamInterface<remote_poolset_tc> {
 public:
  void SetUp() override;
  Output<char> CreateRemotePoolsetFile(Poolset& poolset, std::string host);
  void CheckUnsafeShutdownRemote(const remote_poolset& rp);
};

std::ostream& operator<<(std::ostream& stream, remote_poolset_tc const& p);

std::vector<remote_poolset_tc> GetPoolsetsWithRemoteReplicaParams();

#endif  // US_REMOTE_REPLICAS_TESTS_H
