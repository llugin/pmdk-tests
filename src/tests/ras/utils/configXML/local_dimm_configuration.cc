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

#include "local_dimm_configuration.h"

#ifdef __linux__
void LocalDimmConfiguration::SetDimmCollections() {
  for (auto mountpoint : dimm_mountpoints_) {
    try {
      DimmCollection temp = DimmCollection(mountpoint);
      dimm_collections_.emplace_back(std::move(temp));
    } catch (const std::invalid_argument &e) {
      std::cerr << e.what() << std::endl;
      throw;
    }
  }
}
#endif

int LocalDimmConfiguration::FillConfigFields(pugi::xml_node &&root) {
  root = root.child("localConfiguration");

  if (root.empty()) {
    std::cerr << "Cannot find 'localConfiguration' node" << std::endl;
    return -1;
  }

  if (SetTestDir(root, test_dir_) != 0 ||
      ParseDimmMountpoints(root.child("dimmConfiguration")) != 0) {
    return -1;
  }

  return 0;
}

#ifdef __linux__
const std::vector<DimmCollection> LocalDimmConfiguration::GetDimmCollections() {
  if (dimm_collections_.empty()) {
    SetDimmCollections();
    return dimm_collections_;
  }
  return dimm_collections_;
}
#endif

int LocalDimmConfiguration::ParseDimmMountpoints(pugi::xml_node &&node) {
  int ret = -1;

  for (auto &&it : node.children("mountPoint")) {
    ret = 0;
    try {
      dimm_mountpoints_.emplace_back(it.text().get());
    } catch (const std::invalid_argument &e) {
      std::cerr << e.what() << std::endl;
      return -1;
    }
  }

  if (ret == -1) {
    std::cerr << "dimmConfiguration node does not exist" << std::endl;
  }

  return ret;
}
