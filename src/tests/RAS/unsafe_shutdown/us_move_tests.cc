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

#include "us_move_tests.h"

std::ostream& operator<<(std::ostream& stream, param_with_us const& m) {
  std::string content = "\nUnsafe shutdown on: ";
  for (const auto& dimm : m.us_dimms) {
    content += dimm.GetMountpoint() + "\t";
  }
  content += "\nmv " + m.src_pool_dir + " " + m.dest_pool_dir;
  stream << content;
  return stream;
}

std::ostream& operator<<(std::ostream& stream, param_no_us const& m) {
  stream << "\nmv " + m.src_pool_dir + " " + m.dest_pool_dir;
  return stream;
}

std::vector<param_with_us> GetParamsWithUS() {
  std::vector<param_with_us> ret;

  /* Move pool from US DIMM to non-DIMM */
  {
    param_with_us tc;
    if (us_dimms.size() >= 1) {
      tc.enough_dimms = true;
      tc.src_pool_dir = us_dimms[0].GetMountpoint() + SEPARATOR;
      tc.dest_pool_dir = local_config->GetTestDir() + SEPARATOR;
      tc.us_dimms = {us_dimms[0]};
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Move pool from non-DIMM to US DIMM */
  {
    param_with_us tc;
    if (us_dimms.size() >= 1) {
      tc.enough_dimms = true;
      tc.src_pool_dir = local_config->GetTestDir() + SEPARATOR;
      tc.dest_pool_dir = us_dimms[0].GetMountpoint() + SEPARATOR;
      tc.us_dimms = {us_dimms[0]};
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Move pool from US DIMM to non-US DIMM */
  {
    param_with_us tc;
    if (us_dimms.size() >= 1 && non_us_dimms.size() >= 1) {
      tc.enough_dimms = true;
      tc.src_pool_dir = us_dimms[0].GetMountpoint() + SEPARATOR;
      tc.dest_pool_dir = non_us_dimms[0].GetMountpoint() + SEPARATOR;
      tc.us_dimms = {us_dimms[0]};
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Move pool from US DIMM to US DIMM */
  {
    param_with_us tc;
    if (us_dimms.size() >= 2) {
      tc.enough_dimms = true;
      tc.src_pool_dir = us_dimms[1].GetMountpoint() + SEPARATOR;
      tc.dest_pool_dir = us_dimms[0].GetMountpoint() + SEPARATOR;
      tc.us_dimms = {us_dimms[0], us_dimms[1]};
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  return ret;
}

void MoveCleanPool::SetUp() {
  param_with_us param = GetParam();
  ASSERT_TRUE(param.enough_dimms) << "Not enough dimms specified to run test.";
  src_pool_path_ = param.src_pool_dir + GetNormalizedTestName() + "_pool";
  dest_pool_path_ = param.dest_pool_dir + GetNormalizedTestName() + "_pool";
  injecter_.reset(new Injecter{param.us_dimms});
}

/**
 * TC05_MOVE_POOL_CLEAN
 * Check if pool moved between devices in a manner specified by test parameters
 * and closed properly can be opened.
 * Trigger unsafe shutdown after closing the pool.
 * \test
 *          \li \c Step1. Create pool on device. / SUCCESS
 *          \li \c Step2. Write pattern persistently to pool, close the pool
 *          / SUCCESS
 *          \li \c Step3. Increment USC on DIMM specified by parameter, reboot
 *          \li Step4. Confirm USC incremented / SUCCESS
 *          \li \c Step4. Move the pool to different device. / SUCCESS
 *          \li \c Step5. Open the pool. / SUCCESS
 *          \li \c Step6. Verify written pattern. / SUCCESS
 */
TEST_P(MoveCleanPool, TC05_MOVE_POOL_CLEAN_before_us) {
  /* Step1. */
  PMEMobjpool* pop =
      pmemobj_create(src_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0644);
  ASSERT_NE(pop, nullptr);

  /* Step2 */
  ObjData<int> pd{pop};
  pd.WriteData();
  pmemobj_close(pop);

  /* Step3 */
  injecter_->InjectUS();
}

TEST_P(MoveCleanPool, TC05_MOVE_POOL_CLEAN_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";
  /* Step4. */
  ASSERT_TRUE(injecter_->ConfirmRebootedWithUS());

  /* Step5. */
  auto out =
      shell_.ExecuteCommand("mv " + src_pool_path_ + " " + dest_pool_path_);
  ASSERT_EQ(out.GetExitCode(), 0) << out.GetContent() << std::endl;

  /* Step6. */
  PMEMobjpool* pop = pmemobj_open(dest_pool_path_.c_str(), nullptr);
  ASSERT_NE(pop, nullptr);

  /* Step6. */
  ObjData<int> pd{pop};
  pd.AssertDataCorrect();
}

INSTANTIATE_TEST_CASE_P(UnsafeShutdown, MoveCleanPool,
                        ::testing::ValuesIn(GetParamsWithUS()));

void MoveDirtyPool::SetUp() {
  param_with_us param = GetParam();
  ASSERT_TRUE(param.enough_dimms)
      << "Test needs more dimms than are specified.";
  src_pool_path_ = param.src_pool_dir + GetNormalizedTestName() + "_pool";
  dest_pool_path_ = param.dest_pool_dir + GetNormalizedTestName() + "_pool";
  injecter_.reset(new Injecter{param.us_dimms});
}

/**
 * TC06_MOVE_POOL_DIRTY
 * Check if pool moved between devices in a manner specified by test parameters
 * and closed properly can be opened.
 * Trigger unsafe shutdown after closing the pool.
 * \test
 *          \li \c Step1. Create pool on device. / SUCCESS
 *          \li \c Step2. Write pattern persistently to pool / SUCCESS
 *          \li \c Step3. Increment USC on DIMM specified by parameter, reboot,
 *          confirm USC incremented / SUCCESS
 *          \li \c Step4. Move the pool to different device. / SUCCESS
 *          \li \c Step5. Open the pool. / FAILURE
 *          \li \c Step6. Force open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 */
TEST_P(MoveDirtyPool, TC06_MOVE_POOL_DIRTY_before_us) {
  /* Step1. */
  PMEMobjpool* pop =
      pmemobj_create(src_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0644);
  ASSERT_NE(pop, nullptr) << "Pool creating failed" << std::endl
                          << pmemobj_errormsg();

  /* Step2 */
  ObjData<int> pd{pop};
  pd.WriteData();

  /* Step3 */
  injecter_->InjectUS();
}

TEST_P(MoveDirtyPool, TC06_MOVE_POOL_DIRTY_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";
  ASSERT_TRUE(injecter_->ConfirmRebootedWithUS());

  /* Step4. */
  auto out =
      shell_.ExecuteCommand("mv " + src_pool_path_ + " " + dest_pool_path_);
  ASSERT_EQ(out.GetExitCode(), 0) << "Moving operation failed" << std::endl
                                  << out.GetContent() << std::endl;

  /* Step5. */
  ASSERT_EQ(pmemobj_open(dest_pool_path_.c_str(), nullptr), nullptr)
      << "Dirty pool after moving was opened but should be not";

  ASSERT_NO_FATAL_FAILURE(Repair(dest_pool_path_.c_str()));
}

INSTANTIATE_TEST_CASE_P(UnsafeShutdown, MoveDirtyPool,
                        ::testing::ValuesIn(GetParamsWithUS()));

std::vector<param_no_us> GetParamsNoUS() {
  std::vector<param_no_us> ret;

  /* Move pool from non-US Dimm to non-Dimm */
  {
    param_no_us tc;
    if (non_us_dimms.size() >= 1) {
      tc.enough_dimms = true;
      tc.src_pool_dir = non_us_dimms[0].GetMountpoint() + SEPARATOR;
      tc.dest_pool_dir = local_config->GetTestDir() + SEPARATOR;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  /* Move pool from non-Dimm to non-US Dimm */
  {
    param_no_us tc;
    if (non_us_dimms.size() >= 1) {
      tc.enough_dimms = true;
      tc.src_pool_dir = local_config->GetTestDir() + SEPARATOR;
      tc.dest_pool_dir = non_us_dimms[0].GetMountpoint() + SEPARATOR;
    } else {
      tc.enough_dimms = false;
    }
    ret.emplace_back(tc);
  }

  return ret;
}

void MoveCleanPoolWithoutUS::SetUp() {
  param_no_us param = GetParam();
  ASSERT_TRUE(param.enough_dimms)
      << "Test needs more dimms to run than was specified.";
  src_pool_path_ = param.src_pool_dir + GetNormalizedTestName() + "_pool";
  dest_pool_path_ = param.dest_pool_dir + GetNormalizedTestName() + "_pool";
}

/**
 * TC07_MOVE_POOL_CLEAN_WITHOUT_US
 * Check if pool moved between devices in a manner specified by test parameters
 * and shut properly can be opened. No unsafe shutdown is triggered.
 * \test
 *          \li \c Step1. Create pool on device. / SUCCESS
 *          \li \c Step2. Write pattern persistently to pool, close the pool. / SUCCESS
 *          \li \c Step3. Move the pool to other device. / SUCCESS
 *          \li \c Step4. Open the pool. / SUCCESS
 *          \li \c Step5. Verify written pattern. / SUCCESS
 */
TEST_P(MoveCleanPoolWithoutUS, TC07_MOVE_POOL_CLEAN_WITHOUT_US_before_us) {
  /* Step1 */
  PMEMobjpool* pop =
      pmemobj_create(src_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0644);
  ASSERT_NE(pop, nullptr) << "Pool could not be created" << std::endl
                          << pmemobj_errormsg();

  /* Step2 */
  ObjData<int> pd{pop};
  pd.WriteData();
  pmemobj_close(pop);
}

TEST_P(MoveCleanPoolWithoutUS, TC07_MOVE_POOL_CLEAN_WITHOUT_US_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step3 */
  auto out =
      shell_.ExecuteCommand("mv " + src_pool_path_ + " " + dest_pool_path_);
  ASSERT_EQ(out.GetExitCode(), 0) << "Moving operation failed" << std::endl
                                  << out.GetContent() << std::endl;

  /* Step4. */
  PMEMobjpool* pop = pmemobj_open(dest_pool_path_.c_str(), nullptr);
  ASSERT_NE(pop, nullptr);

  /* Step5. */
  ObjData<int> pd{pop};
  pd.AssertDataCorrect();
}

INSTANTIATE_TEST_CASE_P(UnsafeShutdown, MoveCleanPoolWithoutUS,
                        ::testing::ValuesIn(GetParamsNoUS()));

void MoveDirtyPoolWithoutUS::SetUp() {
  param_no_us param = GetParam();
  ASSERT_TRUE(param.enough_dimms)
      << "Test needs more dimms to run than was specified.";
  src_pool_path_ = param.src_pool_dir + GetNormalizedTestName() + "_pool";
  dest_pool_path_ = param.dest_pool_dir + GetNormalizedTestName() + "_pool";
}

/**
 * TC08_MOVE_POOL_DIRTY_WITHOUT_US
 * Check if pool moved between devices in a manner specified by test parameters
 * can be opened without been closed properly. No unsafe shutdown is triggered.
 * \test
 *          \li \c Step1. Create pool on device. / SUCCESS
 *          \li \c Step2. Write pattern persistently to pool. / SUCCESS
 *          \li \c Step3. Move the pool to other device. / SUCCESS
 *          \li \c Step4. Open the pool. / FAIL
 *          \li \c Step5. Force open pool / SUCCESS
 *          \li \c Step6. Verify written pattern. / SUCCESS
 */
TEST_P(MoveDirtyPoolWithoutUS, TC08_MOVE_POOL_DIRTY_WITHOUT_US_before_us) {
  /* Step1 */
  PMEMobjpool* pop =
      pmemobj_create(src_pool_path_.c_str(), nullptr, PMEMOBJ_MIN_POOL, 0644);
  /* Step2 */
  ObjData<int> pd{pop};
  pd.WriteData();
}

TEST_P(MoveDirtyPoolWithoutUS, TC08_MOVE_POOL_DIRTY_WITHOUT_US_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step3 */
  auto out =
      shell_.ExecuteCommand("mv " + src_pool_path_ + " " + dest_pool_path_);
  ASSERT_EQ(out.GetExitCode(), 0) << "Moving operation failed" << std::endl
                                  << out.GetContent() << std::endl;

  /* Step4. */
  PMEMobjpool* pop = pmemobj_open(dest_pool_path_.c_str(), nullptr);
  ASSERT_EQ(pop, nullptr) << "TODO: move between dimms";
}

INSTANTIATE_TEST_CASE_P(UnsafeShutdown, MoveDirtyPoolWithoutUS,
                        ::testing::ValuesIn(GetParamsNoUS()));
