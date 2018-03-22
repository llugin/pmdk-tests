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

#include <algorithm>
#include <exception>
#include <iostream>
#include <memory>
#include "api_c/api_c.h"
#include "configXML/local_configuration.h"
#include "gtest/gtest.h"

std::unique_ptr<LocalConfiguration> local_config{new LocalConfiguration()};

std::string filter;
std::string address;
std::string rpmem_env;

int main(int argc, char **argv) {
  try {
    if (local_config->ReadConfigFile() != 0) {
      return -1;
    }
    ::testing::InitGoogleTest(&argc, argv);

    if (argc <= 1) {
      std::cerr << "$ US_REMOTE_TESTER <address> [filter]" << std::endl;
      return -1;
    }
    address = argv[1];
    if (address.compare("localhost") == 0) {
      rpmem_env = "RPMEM_ENABLE_SOCKETS=1 RPMEM_ENABLE_VERBS=0 ";
    }

    if (argc > 2) {
      filter = argv[2];
    }

    return RUN_ALL_TESTS();

  } catch (const std::exception &e) {
    std::cerr << "Exception was caught: " << e.what() << std::endl;
    return -1;
  }
}
