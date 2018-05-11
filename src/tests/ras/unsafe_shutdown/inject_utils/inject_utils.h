
#ifndef US_INJECT_MANAGER_H
#define US_INJECT_MANAGER_H

#include "dimm/dimm.h"

class InjectManager {
 public:
  InjectManager(std::string test_dir) : test_dir_(test_dir) {
  }

  bool IsUSCIncreasedBy(int increment,
                        const std::vector<DimmCollection> &dimm_colls);
  int RecordUSCAll(const std::vector<DimmCollection> &dimm_colls);
  int InjectAll(const std::vector<DimmCollection> &us_dimm_colls);

 private:
  std::string test_dir_;
  int RecordDimmUSC(Dimm dimm);
  int ReadRecordedUSC(std::string usc_file_path);
};

#endif  // US_INJECT_MANAGER_H
