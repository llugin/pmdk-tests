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

#ifndef INJECT_MANAGER_H
#define INJECT_MANAGER_H

#include "dimm/dimm.h"

enum class InjectStrategy { all, first, last };

class InjectManager {
 public:
  InjectManager(std::string test_dir, InjectStrategy strategy)
      : test_dir_{test_dir}, strategy_{strategy} {
  }

  bool UnsafelyShutdown(const std::vector<DimmCollection> &dimm_colls);
  bool SafelyShutdown(const std::vector<DimmCollection> &dimm_colls);
  int RecordUSC(const std::vector<DimmCollection> &dimm_colls);
  int Inject(const std::vector<DimmCollection> &us_dimm_colls);

 private:
  std::string test_dir_;
  InjectStrategy strategy_;
  int RecordDimmUSC(Dimm dimm);
  int ReadRecordedUSC(std::string usc_file_path);
  std::vector<Dimm> DimmsToInject(const DimmCollection &us_dimm_coll);
};

#endif  // INJECT_MANAGER_H
