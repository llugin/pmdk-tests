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

#include "us_remote_replicas_tests.h"

void SyncRemoteReplica::SetUp() {
  remote_bin_dir_ = "/home/leszekl/repos/pmdk-tests/build/";  // TODO

  remote_poolset_tc param = GetParam();
  ASSERT_TRUE(param.enough_dimms) << "Not enough dimms specified to run test.";
  injecter_.reset(new Injecter{param.us_dimms});
}

void SyncRemoteReplica::InjectRemote(remote_poolset rp) {
  std::string remote_injecter_path = remote_bin_dir_ + "US_REMOTE_INJECTER";
  std::string mountpoints_arg;
  for (const DimmDevice& d : rp.us_dimms) {
    mountpoints_arg += d.GetMountpoint() + " ";
  };
  SshRunner ssh_runner{rp.host};
  std::string cmd = remote_injecter_path + " inject " + mountpoints_arg;
  auto out = ssh_runner.ExecuteRemote(cmd);
  ASSERT_EQ(out.GetExitCode(), ssh_runner.connection_error_)
      << cmd << std::endl
      << "Injection did not end with reboot";
}

void SyncRemoteReplica::ConfirmInjectedRemote(remote_poolset rp) {
  std::string remote_injecter_path = remote_bin_dir_ + "US_REMOTE_INJECTER";
  std::string mountpoints_arg;
  for (const DimmDevice& d : rp.us_dimms) {
    mountpoints_arg += d.GetMountpoint() + " ";
  };
  SshRunner ssh_runner{rp.host};
  ASSERT_TRUE(ssh_runner.WaitForConnection(15))
      << "Remote host " << rp.host << " is not responding after timeout.";
  auto out = ssh_runner.ExecuteRemote(remote_injecter_path + " confirm " +
                                      mountpoints_arg);
  ASSERT_EQ(out.GetExitCode(), 0) << "US injection was unsuccesful."
                                  << std::endl
                                  << out.GetContent();
}

std::ostream& operator<<(std::ostream& stream, remote_poolset_tc const& p) {
  std::string content = "\n";
  for (const auto& line : p.poolset.GetContent()) {
    content = content + line + '\n';
  }

  content += "\nRemote poolsets:\n";

  for (const auto& rps : p.remote_poolsets) {
    content += rps.poolset.GetFullPath() + "\n";
    for (const auto& line : rps.poolset.GetContent()) {
      content = content + line + '\n';
    }
    content += "\n";
  }

  content += "Unsafe shutdown on: ";
  for (const auto& dimm : p.us_dimms) {
    content += dimm.GetMountpoint() + "\t";
  }

  stream << content;
  return stream;
}

std::string remote_poolset::GetSectionLine() {
  return "REPLICA " + this->host + " ../../" + this->poolset.GetFullPath();
}

/**
 * TC10_SYNC_WITH_REMOTE_REPLICAS
 * Create poolset with remote replicas specified by parameter write data,
 * trigger US.
 * If syncable: restore pool from replica and confirm written data correctness
 * else: confirm syncing not possible
 * \test
 *         \li \c Step1. Create pool from poolset with primary pool on US DIMM
 * and replicas according to given parameter. / SUCCESS
 *          \li \c Step2. Write pattern to primary pool persistently.
 *          \li \c Step3. Trigger US on specified dimms, reboot, confirm USC
 * incremented / SUCCESS
 *          \li \c Step4. Open the pool / FAIL
 *          \li \c Step5. If syncable: restore the pool from replica / SUCCESS
 *          \li \c Step6. If syncable: open the pool / SUCCESS
 *          \li \c Step7. If syncable: Read pattern from pool, confirm it
 * is the same as written in Step2 / SUCCESS
 */
TEST_P(SyncRemoteReplica, TC10_SYNC_REMOTE_REPLICA_before_us) {
  remote_poolset_tc param = GetParam();
  Poolset ps = param.poolset;

  PoolsetManagement psm;
  ASSERT_EQ(psm.CreatePoolsetFile(ps), 0)
      << "error while creating local poolset file";

  ASSERT_TRUE(psm.PoolsetFileExists(ps)) << "Poolset file " << ps.GetFullPath()
                                         << " does not exist";

  for (auto rp : param.remote_poolsets) {
    auto out = CreateRemotePoolsetFile(rp.poolset, rp.host);
    ASSERT_EQ(out.GetExitCode(), 0) << "Error while creating remote poolset: "
                                    << out.GetContent();
  }

  std::string cmd = "pmempool create obj " + ps.GetFullPath();
  auto out = shell_.ExecuteCommand(cmd);
  ASSERT_EQ(out.GetExitCode(), 0) << cmd << std::endl
                                  << "errno: " << errno << std::endl
                                  << out.GetContent();

  PMEMobjpool* pop = pmemobj_open(ps.GetFullPath().c_str(), nullptr);
  ASSERT_NE(pop, nullptr) << "Error while pool opening" << std::endl
                          << pmemobj_errormsg();

  ObjData<int> pd{pop};
  pd.WriteData();

  injecter_->InjectUS();
  for (const auto& rp : param.remote_poolsets) {
    if (rp.us_dimms.size() > 0) {
      InjectRemote(rp);
    }
  }
}

TEST_P(SyncRemoteReplica, TC10_SYNC_REMOTE_REPLICA_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";
  ASSERT_TRUE(injecter_->ConfirmRebootedWithUS());

  remote_poolset_tc param = GetParam();
  for (const auto& rp : param.remote_poolsets) {
    ASSERT_NO_FATAL_FAILURE(ConfirmInjectedRemote(rp));
  }

  Poolset ps = param.poolset;
  PoolsetManagement psm;
  ASSERT_TRUE(psm.PoolsetFileExists(ps)) << "Poolset file " << ps.GetFullPath()
                                         << " does not exist";

  PMEMobjpool* pop = pmemobj_open(ps.GetFullPath().c_str(), nullptr);
  ASSERT_EQ(pop, nullptr) << "Pool after US was opened but should be not";

  std::string cmd = "pmempool sync " + ps.GetFullPath();
  auto out = shell_.ExecuteCommand(cmd);

  int sync_exit_code = (param.is_syncable ? 0 : 1);
  ASSERT_EQ(out.GetExitCode(), sync_exit_code)
      << cmd << " result was other than expected." << std::endl
      << out.GetContent();

  pop = pmemobj_open(ps.GetFullPath().c_str(), nullptr);

  if (!param.is_syncable) {
    ASSERT_EQ(pop, nullptr) << "Unsyncable pool was opened but should be not";
    Repair(ps.GetFullPath());
    pop = pmemobj_open(ps.GetFullPath().c_str(), nullptr);
  }

  ASSERT_NE(pop, nullptr) << "Syncable pool could not be opened after sync";
  ObjData<int> pd{pop};
  pd.AssertDataCorrect();
  pmemobj_close(pop);
}

std::vector<remote_poolset_tc> GetPoolsetsWithRemoteReplicaParams() {
  std::vector<remote_poolset_tc> ret;

  /* Remote replica on healthy DIMM, local replica on US DIMM */
  {
    remote_poolset_tc tc;
    if (us_dimms.size() >= 2 && non_us_dimms.size() >= 1 &&
        remote_non_us_dimms.size() >= 1) {
      tc.enough_dimms = true;
      remote_poolset rp;
      rp.host = "localhost";  // TODO
      rp.us_dimms = {};
      std::string remote_replica_path =
          remote_non_us_dimms[0].GetMountpoint() + SEPARATOR + "remote1";
      rp.poolset =
          Poolset{remote_non_us_dimms[0].GetMountpoint(),
                  "remote_pool1.set",
                  {{"PMEMPOOLSET", "18MB " + remote_replica_path + ".part0",
                    "9MB " + remote_replica_path + ".part1"}}};

      tc.remote_poolsets = {rp};

      std::string local_master_path =
          us_dimms[0].GetMountpoint() + SEPARATOR + "master7";
      std::string local_replica_path =
          us_dimms[1].GetMountpoint() + SEPARATOR + "replica7";
      tc.poolset = Poolset{
          // clang-format off
      us_dimms[0].GetMountpoint(),
      "pool7.set",
      {{"PMEMPOOLSET",
        "9MB " + local_master_path + ".part0",
        "9MB " + local_master_path + ".part1",
        "9MB " + local_master_path + ".part2"},
       {"REPLICA",
        "9MB " + local_replica_path + ".part0",
        "18MB " + local_replica_path + ".part1"},
       {rp.GetSectionLine()}}  // clang-format on
      };
      tc.us_dimms = {us_dimms[0], us_dimms[1]};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Remote replica on US DIMM, local replica on healthy DIMM */
  {
    remote_poolset_tc tc;
    if (remote_us_dimms.size() >= 1 && non_us_dimms.size() >= 1) {
      tc.enough_dimms = true;
      remote_poolset rp;
      rp.host = "localhost";  // TODO;
      rp.us_dimms = {remote_us_dimms.front()};
      std::string remote_replica_path =
          remote_us_dimms[0].GetMountpoint() + SEPARATOR + "remote2";
      rp.poolset =
          Poolset{remote_us_dimms[0].GetMountpoint(),
                  "remote_pool2.set",
                  {{"PMEMPOOLSET", "18MB " + remote_replica_path + ".part0",
                    "9MB " + remote_replica_path + ".part1"}}};
      tc.remote_poolsets = {rp};
      std::string local_master_path =
          non_us_dimms[0].GetMountpoint() + SEPARATOR + "master8";
      std::string local_replica_path =
          non_us_dimms[0].GetMountpoint() + SEPARATOR + "replica8";
      tc.poolset =
          Poolset{us_dimms[0].GetMountpoint(),  // clang-format off
      "pool_8.set",
      {{"PMEMPOOLSET",
        "9MB " + local_master_path + ".part0",
        "9MB " + local_master_path + ".part1",
        "9MB " + local_master_path + ".part2"},
       {"REPLICA",
        "9MB " + local_replica_path + ".part0",
        "18MB " + local_replica_path + ".part1"},
       {rp.GetSectionLine()}}};  // clang-format on
      tc.us_dimms = {us_dimms[0]};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Remote replica on US DIMM, local replica on US DIMM */
  {
    remote_poolset_tc tc;

    if (remote_us_dimms.size() >= 1 && non_us_dimms.size() >= 1 &&
        us_dimms.size() >= 1) {
      tc.enough_dimms = true;
      remote_poolset rp;
      rp.host = "localhost";
      rp.us_dimms = {remote_us_dimms.front()};
      std::string remote_replica_path =
          remote_us_dimms[0].GetMountpoint() + SEPARATOR + "remote3";
      rp.poolset =
          Poolset{remote_us_dimms[0].GetMountpoint(),
                  "remote_pool3.set",  // clang-format off
           {{"PMEMPOOLSET",
             "18MB " + remote_replica_path + ".part0",
             "9MB " + remote_replica_path + ".part1"}}};
      // clang-format on

      std::string local_master_path =
          us_dimms[0].GetMountpoint() + SEPARATOR + "master9";
      std::string local_replica_path =
          non_us_dimms[0].GetMountpoint() + SEPARATOR + "replica9";
      tc.remote_poolsets = {rp};
      tc.poolset =
          Poolset{us_dimms[0].GetMountpoint(),
                  "pool_9.set",  // clang-format off
      {{"PMEMPOOLSET",
        "9MB " + local_master_path + ".part0",
        "9MB " + local_master_path + ".part1",
        "9MB " + local_master_path + ".part2"},
       {"REPLICA",
        "9MB " + local_replica_path + ".part0",
        "18MB " + local_replica_path + ".part1"},
       {rp.GetSectionLine()}}};
      // clang-format on
      tc.us_dimms = {us_dimms[0]};
      tc.is_syncable = false;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Remote replica on US DIMM, remote replica on non-DIMM */
  {
    remote_poolset_tc tc;
    if (remote_us_dimms.size() >= 1 && us_dimms.size() >= 1) {
      tc.enough_dimms = true;
      remote_poolset rp1;
      rp1.host = "localhost";
      rp1.us_dimms = {remote_us_dimms.front()};
      std::string remote_replica_path1 =
          remote_us_dimms[0].GetMountpoint() + SEPARATOR + "remote4";
      rp1.poolset =
          Poolset{remote_us_dimms[0].GetMountpoint(),
                  "remote_pool4.set",
                  {{"PMEMPOOLSET", "18MB " + remote_replica_path1 + ".part0",
                    "9MB " + remote_replica_path1 + ".part1"}}};

      remote_poolset rp2;
      rp2.host = "localhost";
      rp2.us_dimms = {};
      std::string remote_replica_path2 =
          local_config->GetTestDir() + SEPARATOR + "remote5";
      rp2.poolset =
          Poolset{local_config->GetTestDir(),  // remote config
                  "remote_pool5.set",
                  {{"PMEMPOOLSET", "18MB " + remote_replica_path2 + ".part0",
                    "9MB " + remote_replica_path2 + ".part1"}}};
      tc.remote_poolsets = {rp1, rp2};
      std::string local_master_path =
          us_dimms[0].GetMountpoint() + SEPARATOR + "master10";
      tc.poolset =
          Poolset{us_dimms[0].GetMountpoint(),
                  "pool_10.set",
                  {{"PMEMPOOLSET", "9MB " + local_master_path + ".part0",
                    "9MB " + local_master_path + ".part1",
                    "9MB " + local_master_path + ".part2"},
                   {rp1.GetSectionLine()},
                   {rp2.GetSectionLine()}}};
      tc.us_dimms = {us_dimms[0]};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Remote replica on US_DIMM */
  {
    remote_poolset_tc tc;
    if (remote_us_dimms.size() >= 1 && us_dimms.size() >= 1) {
      tc.enough_dimms = true;
      remote_poolset rp1;
      rp1.host = "localhost";
      rp1.us_dimms = {remote_us_dimms.front()};
      std::string remote_replica_path1 =
          remote_us_dimms[0].GetMountpoint() + SEPARATOR + "remote4";
      rp1.poolset =
          Poolset{remote_us_dimms[0].GetMountpoint(),
                  "remote_pool4.set",
                  {{"PMEMPOOLSET", "18MB " + remote_replica_path1 + ".part0",
                    "9MB " + remote_replica_path1 + ".part1"}}};

      remote_poolset rp2;
      rp2.host = "localhost";
      rp2.us_dimms = {};
      std::string remote_replica_path2 =
          local_config->GetTestDir() + SEPARATOR + "remote5";
      rp2.poolset =
          Poolset{local_config->GetTestDir(),  // remote config
                  "remote_pool5.set",
                  {{"PMEMPOOLSET", "18MB " + remote_replica_path2 + ".part0",
                    "9MB " + remote_replica_path2 + ".part1"}}};
      tc.remote_poolsets = {rp1, rp2};
      std::string local_master_path =
          us_dimms[0].GetMountpoint() + SEPARATOR + "master10";
      tc.poolset =
          Poolset{us_dimms[0].GetMountpoint(),
                  "pool_10.set",
                  {{"PMEMPOOLSET", "9MB " + local_master_path + ".part0",
                    "9MB " + local_master_path + ".part1",
                    "9MB " + local_master_path + ".part2"},
                   {rp1.GetSectionLine()},
                   {rp2.GetSectionLine()}}};
      tc.us_dimms = {us_dimms[0]};
      tc.is_syncable = true;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  return ret;
}

INSTANTIATE_TEST_CASE_P(
    UnsafeShutdown, SyncRemoteReplica,
    ::testing::ValuesIn(GetPoolsetsWithRemoteReplicaParams()));
