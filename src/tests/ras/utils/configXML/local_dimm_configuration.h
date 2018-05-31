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

#ifndef PMDK_TESTS_SRC_UTILS_CONFIGXML_LOCAL_DIMM_CONFIGURATION_H_
#define PMDK_TESTS_SRC_UTILS_CONFIGXML_LOCAL_DIMM_CONFIGURATION_H_

#include "configXML/read_config.h"

#ifdef __linux__
#include "dimm/dimm.h"
#endif

#include "pugixml.hpp"

class LocalDimmConfiguration final : public ReadConfig<LocalDimmConfiguration> {
 private:
  friend class ReadConfig<LocalDimmConfiguration>;
  std::string test_dir_;
  int FillConfigFields(pugi::xml_node &&root);

#ifdef __linux__
  std::vector<DimmCollection> dimm_collections_;
  void SetDimmCollections();
#endif
  std::vector<std::string> dimm_mountpoints_;

  int ParseDimmMountpoints(pugi::xml_node &&node);

 public:
  const std::string &GetTestDir() const {
    return this->test_dir_;
  }
  std::string &operator[](int idx) {
    return dimm_mountpoints_.at(idx);
  }
  int GetSize() const {
    return dimm_mountpoints_.size();
  }

#ifdef __linux__
  const std::vector<DimmCollection> GetDimmCollections();
#endif
  const std::vector<std::string>::const_iterator begin() const noexcept {
    return dimm_mountpoints_.cbegin();
  }

  const std::vector<std::string>::const_iterator end() const noexcept {
    return dimm_mountpoints_.cend();
  }
};

#endif  // !PMDK_TESTS_SRC_UTILS_CONFIGXML_LOCAL_DIMM_CONFIGURATION_H_
