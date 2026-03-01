#pragma once

#include <string>
#include <vector>

namespace cms::models {

struct ScheduleItem {
  int paper_id;
  std::string room;
  std::string time_slot;

  bool operator==(const ScheduleItem& other) const = default;
};

struct Schedule {
  int id;
  std::vector<ScheduleItem> items;
  std::string html;
};

}  // namespace cms::models
