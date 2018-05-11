#include "inject_utils.h"

int InjectManager::ReadRecordedUSC(std::string usc_file_path) {
  std::string content;
  if (ApiC::ReadFile(usc_file_path, content) != 0) {
    return -1;
  }

  try {
    return std::stoi(content);
  } catch (const std::invalid_argument &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  } catch (const std::out_of_range &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
}

int InjectManager::RecordDimmUSC(Dimm dimm) {
  int usc = dimm.GetShutdownCount();
  if (usc == -1) {
    std::cerr << "Reading USC from dimm " << dimm.GetUid() << " failed"
              << std::endl;
    return -1;
  }

  if (ApiC::CreateFileT(test_dir_ + SEPARATOR + dimm.GetUid(),
                        std::to_string(usc)) == -1) {
    return -1;
  }
  return 0;
}

int InjectManager::RecordUSCAll(const std::vector<DimmCollection> &dimm_colls) {
  for (const auto &dc : dimm_colls) {
    for (const auto &d : dc) {
      if (RecordDimmUSC(d) != 0) {
        return -1;
      }
    }
  }
  return 0;
}

int InjectManager::InjectAll(const std::vector<DimmCollection> &us_dimm_colls) {
  for (const auto &dc : us_dimm_colls) {
    for (const auto &d : dc) {
      if (d.InjectUnsafeShutdown() != 0) {
        return -1;
      }
    }
  }
  return 0;
}

bool InjectManager::IsUSCIncreasedBy(
    int increment, const std::vector<DimmCollection> &dimm_colls) {
  for (const auto &dc : dimm_colls) {
    for (const auto &d : dc) {
      int recorded_usc = ReadRecordedUSC(test_dir_ + SEPARATOR + d.GetUid());
      if (recorded_usc == -1) {
        std::cerr << "Could not read USC in dimm: " << d.GetUid()
                  << " on mountpoint: " << dc.GetMountpoint() << std::endl;
        return false;
      }
      if (recorded_usc + increment != d.GetShutdownCount()) {
        std::cerr << "Value of USC (" << d.GetShutdownCount() << ") on dimm "
                  << d.GetUid() << " on mounptoint: " << dc.GetMountpoint()
                  << "differs from expected (" << recorded_usc + increment
                  << ")" << std::endl;
        return false;
      }
    }
  }
  return true;
}
