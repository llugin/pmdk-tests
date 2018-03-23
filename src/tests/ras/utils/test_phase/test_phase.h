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
#include "gtest/gtest.h"
#include "inject_mananger/inject_manager.h"
#include "non_copyable/non_copyable.h"

template <class T>
class TestPhase : public NonCopyable {
 public:
  static T& GetInstance() {
    static T test_phase;
    return test_phase;
  }
  int RunPreTestAction();
  int RunPostTestAction();

  void HandleCmdArgs(int argc, char** argv);
  bool HasInjectAtEnd() {
    return post_test_action_ == ExecutionAction::inject;
  }
  const std::string GetPhaseName() const {
    return this->phase_name_;
  }

 protected:
  int SetUp() {
    return static_cast<T*>(this)->SetUp();
  }
  int Inject() {
    return static_cast<T*>(this)->Inject();
  }
  int CheckUSC() {
    return static_cast<T*>(this)->CheckUSC();
  }
  int CleanUp() {
    return static_cast<T*>(this)->CleanUp();
  }

 private:
  enum class ExecutionAction { setup, check_usc, inject, cleanup };
  ExecutionAction pre_test_action_;
  ExecutionAction post_test_action_;
  std::string phase_name_;
};

template <class T>
int TestPhase<T>::RunPreTestAction() {
  int ret;
  switch (pre_test_action_) {
    case ExecutionAction::setup:
      ret = SetUp();
      break;
    case ExecutionAction::check_usc:
      ret = CheckUSC();
      break;
    default:
      ret = 0;
  }
  if (ret != 0) {
    std::cerr << "Action before test execution failed, running CleanUp"
              << std::endl;
    CleanUp();
  }
  return ret;
}

template <class T>
int TestPhase<T>::RunPostTestAction() {
  switch (post_test_action_) {
    case ExecutionAction::inject:
      return Inject();
    case ExecutionAction::cleanup:
      return CleanUp();
    default:
      throw std::invalid_argument("Invalid post execution action");
  }
}

template <class T>
void TestPhase<T>::HandleCmdArgs(int argc, char** argv) {
  const std::string usage =
      "./" + std::string{argv[0]} + " <phase_number> <inject|cleanup>]";
  if (argc < 3) {
    throw std::invalid_argument(usage);
  }

  if (std::string{argv[2]}.compare("cleanup") == 0) {
    post_test_action_ = ExecutionAction::cleanup;
  } else if (std::string{argv[2]}.compare("inject") == 0) {
    post_test_action_ = ExecutionAction::inject;
  } else {
    throw std::invalid_argument(usage);
  }

  phase_name_ = std::string{"phase_"} + argv[1];
  /* Modify --gtest_filter flag to run only tests from specific phase" */
  ::testing::GTEST_FLAG(filter) =
      ::testing::GTEST_FLAG(filter) + "*" + phase_name_ + "*";

  if (phase_name_.compare("phase_1") == 0) {
    pre_test_action_ = ExecutionAction::setup;
  } else {
    pre_test_action_ = ExecutionAction::check_usc;
  }
}
