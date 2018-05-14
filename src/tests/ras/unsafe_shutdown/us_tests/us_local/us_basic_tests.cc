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

#include "us_basic_tests.h"

void UnsafeShutdownBasic::SetUp() {
  ASSERT_LE(1, local_us_dimm_colls.size())
      << "Test needs more dimms to run than was specified.";
  us_dimm_pool_path_ = local_us_dimm_colls.front().GetMountpoint() + SEPARATOR +
                       GetNormalizedTestName() + "_pool";
}

/**
 * TRY_OPEN_OBJ
 * Create obj pool on DIMM, trigger unsafe shutdown, try opening the pool
 * \test
 *          \li \c Step1. Create a pool on DIMM / SUCCESS
 *          \li \c Step2. Write pattern to pool / SUCCESS
 *          \li \c Step3. Trigger US, power cycle, /SUCCESS
 *          \li \c Step4. Confirm USC incremented /SUCCESS
 *          \li \c Step5. Open the pool / FAIL
 *          \li \c Step6. Force open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TRY_OPEN_OBJ_before_us) {
  /* Step1. */
  PMEMobjpool* pop = pmemobj_create(us_dimm_pool_path_.c_str(), nullptr,
                                    PMEMOBJ_MIN_POOL, 0644 & PERMISSION_MASK);
  ASSERT_NE(nullptr, pop) << "Pool creating failed. Errno: " << errno
                          << std::endl
                          << pmemobj_errormsg();

  /* Step2. */
  ObjData<int> pd{pop};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

TEST_F(UnsafeShutdownBasic, TRY_OPEN_OBJ_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step4. */
  ASSERT_TRUE(inject_mgmt_.IsUSCIncreasedBy(1, {local_us_dimm_colls.front()}));

  /* Step5. */
  ASSERT_EQ(nullptr, pmemobj_open(us_dimm_pool_path_.c_str(), nullptr))
      << "Pool was opened after unsafe shutdown, but should not.";
  ASSERT_EQ(EINVAL, errno);

  /* Step6. */
  ASSERT_NO_FATAL_FAILURE(Repair(us_dimm_pool_path_));
  PMEMobjpool* pop = pmemobj_open(us_dimm_pool_path_.c_str(), nullptr);
  ASSERT_NE(nullptr, pop) << "Pool opening failed. Errno: " << errno
                          << std::endl
                          << pmemobj_errormsg();

  /* Step7 */
  ObjData<int> pd{pop};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";
  pmemobj_close(pop);
}

/**
 * TRY_OPEN_BLK
 * Create blk pool on DIMM, trigger unsafe shutdown, try opening the pool
 * \test
 *          \li \c Step1. Create a pool on DIMM / SUCCESS
 *          \li \c Step2. Write pattern to pool / SUCCESS
 *          \li \c Step3. Trigger US, power cycle, /SUCCESS
 *          \li \c Step4. Confirm USC incremented /SUCCESS
 *          \li \c Step5. Open the pool / FAIL
 *          \li \c Step6. Force open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TRY_OPEN_BLK_before_us) {
  /* Step1. */
  PMEMblkpool* pbp = pmemblk_create(us_dimm_pool_path_.c_str(), blk_size_,
                                    PMEMBLK_MIN_POOL, 0644 & PERMISSION_MASK);
  ASSERT_NE(nullptr, pbp) << "Pool creating failed. Errno: " << errno
                          << std::endl
                          << pmemblk_errormsg();

  /* Step2. */
  BlkData<int> pd{pbp};
  ASSERT_EQ(0, pd.Write(blk_data_)) << "Writing to pool failed";
}

TEST_F(UnsafeShutdownBasic, TRY_OPEN_BLK_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step4. */
  ASSERT_TRUE(inject_mgmt_.IsUSCIncreasedBy(1, {local_us_dimm_colls.front()}));

  /* Step5. */
  ASSERT_EQ(nullptr, pmemblk_open(us_dimm_pool_path_.c_str(), blk_size_))
      << "Pool was opened after unsafe shutdown, but should not.";
  ASSERT_EQ(EINVAL, errno);

  /* Step6. */
  ASSERT_NO_FATAL_FAILURE(Repair(us_dimm_pool_path_));
  PMEMblkpool* pbp = pmemblk_open(us_dimm_pool_path_.c_str(), blk_size_);
  ASSERT_NE(nullptr, pbp) << "Pool opening failed. Errno: " << errno
                          << std::endl
                          << pmemobj_errormsg();

  /* Step7 */
  BlkData<int> pd{pbp};
  ASSERT_EQ(blk_data_, pd.Read(blk_data_.size()))
      << "Data read from pool differs from written";
  pmemblk_close(pbp);
}

/**
 * TRY_OPEN_LOG
 * Create log pool on DIMM, trigger unsafe shutdown, try opening the pool
 * \test
 *          \li \c Step1. Create a pool on DIMM / SUCCESS
 *          \li \c Step2. Write pattern to pool / SUCCESS
 *          \li \c Step3. Trigger US, power cycle, /SUCCESS
 *          \li \c Step4. Confirm USC incremented /SUCCESS
 *          \li \c Step5. Open the pool / FAIL
 *          \li \c Step6. Force open the pool / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TRY_OPEN_LOG_before_us) {
  /* Step1. */
  PMEMlogpool* plp = pmemlog_create(us_dimm_pool_path_.c_str(),
                                    PMEMLOG_MIN_POOL, 0644 & PERMISSION_MASK);
  ASSERT_NE(nullptr, plp) << "Pool creating failed. Errno: " << errno
                          << std::endl
                          << pmemlog_errormsg();

  /* Step2. */
  LogData pd{plp};
  ASSERT_EQ(0, pd.Write(log_data_)) << "Writing to pool failed";
}

TEST_F(UnsafeShutdownBasic, TRY_OPEN_LOG_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step4. */
  ASSERT_TRUE(inject_mgmt_.IsUSCIncreasedBy(1, {local_us_dimm_colls.front()}));

  /* Step5. */
  ASSERT_EQ(nullptr, pmemlog_open(us_dimm_pool_path_.c_str()))
      << "Pool was opened after unsafe shutdown, but should not.";
  ASSERT_EQ(EINVAL, errno);

  /* Step6. */
  ASSERT_NO_FATAL_FAILURE(Repair(us_dimm_pool_path_));
  PMEMlogpool* plp = pmemlog_open(us_dimm_pool_path_.c_str());
  ASSERT_NE(nullptr, plp) << "Pool opening failed. Errno: " << errno
                          << std::endl
                          << pmemobj_errormsg();

  /* Step7 */
  LogData pd{plp};
  ASSERT_EQ(log_data_, pd.Read()) << "Data read from pool differs from written";
  pmemlog_close(plp);
}

/**
 * TC_TRY_OPEN_AFTER_DOUBLE_US
 * Create pool on DIMM, trigger unsafe shutdown twice, try opening the
 * pool
 * \test
 *          \li \c Step1. Create a pool on DIMM / SUCCESS
 *          \li \c Step2. Write pattern to pool / SUCCESS
 *          \li \c Step3. Trigger US, power cycle, confirm USC incremented -
 * repeat twice / SUCCESS
 *          \li \c Step4. Open the pool / FAIL
 *          \li \c Step5. Force open the pool / SUCCESS
 *          \li \c Step6. Verify pattern / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TC_TRY_OPEN_AFTER_DOUBLE_US_before_us) {
  /* Step1. */
  PMEMobjpool* pop = pmemobj_create(us_dimm_pool_path_.c_str(), nullptr,
                                    PMEMOBJ_MIN_POOL, 0644 & PERMISSION_MASK);
  ASSERT_NE(nullptr, pop) << "Opening pool after shutdown failed. Errno: "
                          << errno << std::endl
                          << pmemobj_errormsg();
  /* Step2. */
  ObjData<int> pd{pop};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

TEST_F(UnsafeShutdownBasic, TC_TRY_OPEN_AFTER_DOUBLE_US_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";
  ASSERT_TRUE(inject_mgmt_.IsUSCIncreasedBy(1, {local_us_dimm_colls.front()}));
}

TEST_F(UnsafeShutdownBasic, TC_TRY_OPEN_AFTER_DOUBLE_US_after_second_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";
  ASSERT_TRUE(inject_mgmt_.IsUSCIncreasedBy(1, {local_us_dimm_colls.front()}));

  /* step4 */
  ASSERT_EQ(nullptr, pmemobj_open(us_dimm_pool_path_.c_str(), nullptr))
      << "Pool was opened after unsafe shutdown, but should not.";
  ASSERT_EQ(EINVAL, errno);

  /* Step5 */
  Repair(us_dimm_pool_path_.c_str());
  PMEMobjpool* pop = pmemobj_open(us_dimm_pool_path_.c_str(), nullptr);
  ASSERT_NE(nullptr, pop) << "Opening pool after shutdown failed. Errno: "
                          << errno << std::endl
                          << pmemobj_errormsg();

  /* Step6. */
  ObjData<int> pd{pop};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";

  pmemobj_close(pop);
}

/**
 * TC_OPEN_CLEAN
 * Create pool on DIMM, close, trigger unsafe shutdown, open the pool
 * \test
 *          \li \c Step1. Create a pool on DIMM. \ SUCCESS
 *          \li \c Step2. Write data to pool. \ SUCCESS
 *          \li \c Step3. Close pool. \ SUCCESS
 *          \li \c Step4. Increment USC, power cycle,
 *          \li \c Step5. Confirm USC incremented /
 * SUCCESS
 *          \li \c Step6. Open the pool. / SUCCESS
 *          \li \c Step7. Verify written pattern / SUCCESS
 */
TEST_F(UnsafeShutdownBasic, TC_OPEN_CLEAN_before_us) {
  /* Step1 */
  PMEMobjpool* pop = pmemobj_create(us_dimm_pool_path_.c_str(), nullptr,
                                    PMEMOBJ_MIN_POOL, 0644 & PERMISSION_MASK);
  ASSERT_NE(nullptr, pop) << "Pool creating failed. Errno: " << errno
                          << std::endl
                          << pmemobj_errormsg();
  /* Step2. */
  ObjData<int> pd{pop};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";

  /* Step3 */
  pmemobj_close(pop);

  /* Step4. */
}

TEST_F(UnsafeShutdownBasic, TC_OPEN_CLEAN_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  /* Step5. */
  ASSERT_TRUE(inject_mgmt_.IsUSCIncreasedBy(1, {local_us_dimm_colls.front()}));

  /* Step6. */
  PMEMobjpool* pop = pmemobj_open(us_dimm_pool_path_.c_str(), nullptr);
  ASSERT_NE(nullptr, pop) << pmemobj_errormsg() << "errno:" << errno;

  /* Step7 */
  ObjData<int> pd{pop};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";

  pmemobj_close(pop);
}

void UnsafeShutdownBasicWithoutUS::SetUp() {
  ASSERT_LE(1, local_non_us_dimm_colls.size())
      << "Test needs more dimms to run than was specified.";
  non_us_dimm_pool_path_ = local_non_us_dimm_colls.front().GetMountpoint() +
                           SEPARATOR + GetNormalizedTestName() + "_pool";
}

/*
* TC_OPEN_DIRTY_NO_US
* Create pool on DIMM, write data, end process without closing the pool, open
* the pool, confirm data.
* \test
*          \li \c Step1. Create a pool on DIMM / SUCCESS
*          \li \c Step2. Write pattern to pool / SUCCESS
*          \li \c Step3. Open the pool / SUCCESS
*          \li \c Step4. Verify pattern / SUCCESS
*/
TEST_F(UnsafeShutdownBasicWithoutUS, TC_OPEN_DIRTY_NO_US_before_us) {
  /* Step1. */
  PMEMobjpool* pop = pmemobj_create(non_us_dimm_pool_path_.c_str(), nullptr,
                                    PMEMOBJ_MIN_POOL, 0644 & PERMISSION_MASK);
  ASSERT_NE(nullptr, pop) << "Pool creating failed. Errno: " << errno
                          << std::endl
                          << pmemobj_errormsg();
  /* Step2. */
  ObjData<int> pd{pop};
  ASSERT_EQ(0, pd.Write(obj_data_)) << "Writing to pool failed";
}

TEST_F(UnsafeShutdownBasicWithoutUS, TC_OPEN_DIRTY_NO_US_after_first_us) {
  ASSERT_TRUE(PassedOnPreviousPhase())
      << "Part of test before shutdown failed.";

  ASSERT_TRUE(inject_mgmt_.IsUSCIncreasedBy(0, {local_non_us_dimm_colls.front()}));

  /* Step3. */
  PMEMobjpool* pop = pmemobj_open(non_us_dimm_pool_path_.c_str(), nullptr);
  ASSERT_NE(nullptr, pop) << "Opening pool after shutdown failed. Errno: "
                          << errno << std::endl
                          << pmemobj_errormsg();

  /* Step4. */
  ObjData<int> pd{pop};
  ASSERT_EQ(obj_data_, pd.Read()) << "Data read from pool differs from written";
  pmemobj_close(pop);
}
