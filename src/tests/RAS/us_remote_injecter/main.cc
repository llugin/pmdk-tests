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

#include <iostream>
#include <vector>
#include "dimm_device.h"
#include "injecter.h"

void Reboot() {
  std::cout << "rebooting" << std::endl;
}

int main(int argc, char **argv) {
  std::string usage =
      "./US_INJECTER inject|confirm <MOUNTPOINT1> <MOUNTPOINT2> ...";

  int ret = 0;
  try {
    if (argc < 3) {
      std::cerr << usage << std::endl;
      return 1;
    }

    std::vector<DimmDevice> dimms;

    for (int i = 2; i < argc; ++i) {
      dimms.push_back(DimmDevice{argv[i]});
    }

    Injecter injecter{dimms};

    if (std::string{"inject"}.compare(argv[1]) == 0) {
      injecter.InjectUS();
      Reboot();
      return 255;  // Connection error mock TODO remove
    } else if (std::string{"confirm"}.compare(argv[1]) == 0) {
      bool rebooted_with_us = injecter.ConfirmRebootedWithUS();
      ret = rebooted_with_us ? 0 : 1;
    } else {
      std::cerr << usage << std::endl;
      ret = 1;
    }

  } catch (const std::exception &e) {
    std::cerr << "Exception was caught: " << e.what() << std::endl;
    ret = 1;
  }

  return ret;
}
