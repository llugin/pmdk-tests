
#include "gtest_utils.h"

bool gtest_utils::ThisTestPassed() {
  return ::testing::UnitTest::GetInstance()
      ->current_test_info()
      ->result()
      ->Passed();
}

std::string gtest_utils::GetFullTestName() {
  auto *test_info = ::testing::UnitTest::GetInstance()->current_test_info();
  return std::string{test_info->test_case_name()} + "/" +
         std::string{test_info->name()};
}

bool gtest_utils::PartiallyPassed() {
  ::testing::UnitTest *ut = ::testing::UnitTest::GetInstance();
  return ut->successful_test_count() > 0 && ut->failed_test_count() > 0;
}

bool gtest_utils::NoTestsPassed() {
  ::testing::UnitTest *ut = ::testing::UnitTest::GetInstance();
  return ut->successful_test_count() == 0;
}
