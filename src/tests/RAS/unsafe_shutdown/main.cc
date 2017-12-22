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
#include <string>
#include "api_c/api_c.h"
#include "configXML/local_configuration.h"
#include "dimm_device.h"
#include "gtest/gtest.h"

std::unique_ptr<LocalConfiguration> local_config{new LocalConfiguration()};

std::vector<DimmDevice> us_dimms;
std::vector<DimmDevice> non_us_dimms;
std::vector<DimmDevice> remote_us_dimms;
std::vector<DimmDevice> remote_non_us_dimms;

void Reboot() {
  std::cout << "rebooting" << std::endl;
}

int main(int argc, char **argv) {
  int ret = 0;
  try {
    if (local_config->ReadConfigFile() != 0) {
      return -1;
    }

    non_us_dimms.push_back(DimmDevice{local_config->GetTestDir() + SEPARATOR +
                                      "dimm1" + SEPARATOR});
    us_dimms.push_back(DimmDevice{local_config->GetTestDir() + SEPARATOR +
                                  "dimm2" + SEPARATOR});
    us_dimms.push_back(DimmDevice{local_config->GetTestDir() + SEPARATOR +
                                  "dimm3" + SEPARATOR});

    remote_non_us_dimms.assign(non_us_dimms.begin(), non_us_dimms.end());
    remote_us_dimms.assign(us_dimms.begin(), us_dimms.end());

    ::testing::InitGoogleTest(&argc, argv);

    bool shutdown_at_end =
        (argc > 1 && std::string{"shutdown"}.compare(argv[1]) == 0) ? true
                                                                    : false;

    if (shutdown_at_end) {
      for (auto d : us_dimms) {
        ApiC::CreateDirectoryT(d.GetMountpoint());
      }
      for (auto d : non_us_dimms) {
        ApiC::CreateDirectoryT(d.GetMountpoint());
      }
    }

    ret = RUN_ALL_TESTS();
    std::cout << "Return code of tests execution: " << ret << std::endl;

    if (!shutdown_at_end) {
      std::cout << "after shutdown";

      for (auto d : us_dimms) {
        ApiC::CleanDirectory(d.GetMountpoint());
        ApiC::RemoveDirectoryT(d.GetMountpoint());
      }
      for (auto d : non_us_dimms) {
        ApiC::CleanDirectory(d.GetMountpoint());
        ApiC::RemoveDirectoryT(d.GetMountpoint());
      }

      ApiC::CleanDirectory(local_config->GetTestDir());
      ApiC::RemoveDirectoryT(local_config->GetTestDir());

    } else {
      Reboot();
      return 255;  // Connection error mock TODO remove
    }

  } catch (const std::exception &e) {
    std::cerr << "Exception was caught: " << e.what() << std::endl;
    ret = -1;
  }

  return ret;
}
