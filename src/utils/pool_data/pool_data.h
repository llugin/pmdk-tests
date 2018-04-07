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

#ifndef POOL_DATA_H
#define POOL_DATA_H

#include <libpmemblk.h>
#include <libpmemlog.h>
#include <libpmemobj.h>
#include <iostream>
#include <vector>

template <typename T>
class ObjData {
 public:
  ObjData(PMEMobjpool *pop) : pop_(pop) {
  }

  int Write(std::vector<T> data) {
    for (auto v : data) {
      PMEMoid oid;
      pmemobj_alloc(pop_, &oid, sizeof(struct elem), type_num_,
                    elem_constructor, &v);
      ++type_num_;

      if (OID_IS_NULL(oid)) {
        std::cerr << "Data allocation failed. Errno: " << errno;
        return 1;
      }
    }
    return 0;
  }

  std::vector<T> Read() {
    std::vector<T> values;
    int tn = 0;

    while (true) {
      PMEMoid oid = POBJ_FIRST_TYPE_NUM(pop_, tn);
      ++tn;

      if (OID_IS_NULL(oid)) {
        break;
      }

      elem *e = static_cast<struct elem *>(pmemobj_direct(oid));
      values.emplace_back(e->value);
    }

    return values;
  }

 private:
  struct elem {
    T value;
  };

  static int elem_constructor(PMEMobjpool *pop, void *ptr, void *arg) {
    struct elem *element = static_cast<struct elem *>(ptr);
    element->value = *static_cast<T *>(arg);
    pmemobj_persist(pop, element, sizeof(struct elem));
    return 0;
  }
  PMEMobjpool *pop_;
  int type_num_ = 0;
};

template <typename T>
class BlkData {
 public:
  BlkData(PMEMblkpool *pbp) : pbp_(pbp) {
  }

  int Write(std::vector<T> data) {
    for (const T c : data) {
      if (pmemblk_write(pbp_, &c, current_block_) != 0) {
        std::cerr << "Writing element on block " << current_block_
                  << "failed. Errno : " << errno;
        return 1;
      }
      ++current_block_;
    }
    return 0;
  }

  std::vector<T> Read() {
    std::vector<T> data;

    size_t bsize = pmemblk_bsize(pbp_);
    for (size_t i = 0; i < current_block_; ++i) {
      std::vector<char> buf;
      buf.reserve(bsize);

      int ret = pmemblk_read(pbp_, buf.data(), i);
      if (ret != 0) {
        std::cerr << "reading element on block " << i
                  << " failed. Errno: " << errno << std::endl;
        break;
      }

      T elem;
      memcpy(&elem, buf.data(), sizeof(elem));
      data.emplace_back(elem);
    }
    return data;
  }

 private:
  PMEMblkpool *pbp_;
  size_t current_block_ = 0;
};

class LogData {
 public:
  LogData(PMEMlogpool *plp) : plp_(plp) {
  }
  int Write(std::string log_text);
  std::string Read();

 private:
  static int ReadLog(const void *buf, size_t len, void *arg);

  const size_t chunk_size_ = 10;
  PMEMlogpool *plp_;
};

#endif  // POOL_DATA_H
