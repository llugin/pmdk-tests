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

#include "pool_data.h"

/*template <typename T>
ObjData<T>::ObjData(PMEMobjpool *pop) : pop_(pop) {
}

int ObjData::elem_constructor(PMEMobjpool *pop, void *ptr, void *arg) {
  struct elem *element = (struct elem *)ptr;
  element->value = *(int *)arg;
  pmemobj_persist(pop, element, sizeof(struct elem));
  return 0;
}
template <typename T>
void ObjData<T>::WriteData() {
  for (auto v : test_data_) {
    // int val = v;
    PMEMoid oid;
    pmemobj_alloc(pop_, &oid, sizeof(struct elem), type_num_, elem_constructor,
                  &v);
    type_num_++;
    ASSERT_FALSE(OID_IS_NULL(oid)) << "Data allocation failed. Errno: "
                                   << errno;
  }
}
template <typename T>
void ObjData<T>::AssertDataCorrect() {
  std::vector<T> values;
  int tn = 0;

  while (true) {
    PMEMoid oid = POBJ_FIRST_TYPE_NUM(pop_, tn);
    tn++;

    if (OID_IS_NULL(oid)) {
      break;
    }

    elem *e = (struct elem *)pmemobj_direct(oid);
    values.push_back(e->value);
  }
  ASSERT_EQ(values, test_data_)
      << "Data read from obj pool does not match data written to it";
}
*/
BlkData::BlkData(PMEMblkpool *pbp) : pbp_(pbp) {
}

void BlkData::WriteData() {
  char pattern = '0';
  size_t blk_size = pmemblk_bsize(pbp_);

  for (int i = 0; i < writes_number_; ++i) {
    std::vector<char> buf(blk_size);
    memset(buf.data(), pattern, blk_size);
    ASSERT_EQ(pmemblk_write(pbp_, buf.data(), i), 0)
        << "Writing element on index " << i << "failed. Errno : " << errno;
    ++pattern;
  }
}

void BlkData::AssertDataCorrect() {
  char pattern = '0';
  size_t blk_size = pmemblk_bsize(pbp_);

  for (int i = 0; i < writes_number_; ++i) {
    std::vector<char> buf(blk_size);
    ASSERT_EQ(pmemblk_read(pbp_, buf.data(), i), 0)
        << "read on element with index " << i << " failed. Errno: " << errno
        << std::endl;
    for (size_t j = 0; j < blk_size; ++j) {
      ASSERT_EQ(buf[j], pattern) << "Patterns mismatch" << std::endl;
    }
    ++pattern;
  }
}

LogData::LogData(PMEMlogpool *plp) : plp_(plp) {
}

void LogData::WriteData() {
  int chunk_size = log_text_.size() / chunks_;

  int pos = 0;
  for (int i = 0; i < chunks_; ++i) {
    ASSERT_EQ(pmemlog_append(plp_, log_text_.substr(pos, chunk_size).c_str(),
                             chunk_size),
              0)
        << "Appending line to log pool failed. Errno: " << errno;
    pos += chunk_size;
  }

  int last_chunk_size = log_text_.size() % chunks_;
  ASSERT_EQ(
      pmemlog_append(plp_, log_text_.substr(pos).c_str(), last_chunk_size), 0)
      << "Appending line to log pool failed. Errno :" << errno;
}

void LogData::AssertDataCorrect() {
  pmemlog_walk(plp_, 0, ReadLog, nullptr);
}

int LogData::ReadLog(const void *buf, size_t len, void *) {
  std::string read_text{static_cast<const char *>(buf), len};
  EXPECT_EQ(read_text, log_text_)
      << "Log read from pool differs from log written to it";
  return 0;
}

const std::string LogData::log_text_ =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
    "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
    "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
    "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
    "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint "
    "occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    "mollit anim id est laborum.";
