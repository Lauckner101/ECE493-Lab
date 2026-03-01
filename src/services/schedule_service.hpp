#pragma once

#include <optional>
#include <string>
#include <vector>

#include "models/schedule.hpp"
#include "services/paper_service.hpp"

namespace cms::services {

struct ScheduleGenerateRequest {
  int room_count = 2;
  int time_slot_count = 2;
  bool simulate_save_failure = false;
};

enum class ScheduleGenerateStatus {
  kGenerated,
  kNoAcceptedPapers,
  kUnsatisfiableConstraints,
  kSystemError,
};

struct ScheduleGenerateResult {
  ScheduleGenerateStatus status;
  std::string message;
  int schedule_id;
  std::string html;
};

struct ScheduleEditRequest {
  std::vector<cms::models::ScheduleItem> items;
  bool simulate_save_failure = false;
};

enum class ScheduleEditStatus {
  kUpdated,
  kNoSchedule,
  kNoChanges,
  kConflict,
  kSystemError,
};

struct ScheduleEditResult {
  ScheduleEditStatus status;
  std::string message;
  int schedule_id;
  std::string html;
};

class ScheduleService {
 public:
  explicit ScheduleService(PaperService* paper_service) : paper_service_(paper_service) {}

  ScheduleGenerateResult GenerateSchedule(const ScheduleGenerateRequest& request);
  ScheduleEditResult EditCurrentSchedule(const ScheduleEditRequest& request);
  std::optional<cms::models::Schedule> GetCurrentSchedule() const;
  std::string GetCurrentScheduleHtml() const;

 private:
  static std::vector<std::string> BuildRoomNames(int room_count);
  static std::vector<std::string> BuildTimeSlots(int time_slot_count);
  static std::string RenderHtml(const std::vector<cms::models::ScheduleItem>& items);

  static bool InputsMatch(const std::vector<int>& accepted_ids, const ScheduleGenerateRequest& request,
                          const std::vector<int>& previous_ids, int previous_room_count,
                          int previous_time_slot_count);
  static std::vector<cms::models::ScheduleItem> NormalizeItems(
      const std::vector<cms::models::ScheduleItem>& items);
  static bool ItemsConflict(const std::vector<cms::models::ScheduleItem>& items);

  PaperService* paper_service_;
  int next_schedule_id_ = 1;
  std::optional<cms::models::Schedule> current_schedule_;
  std::vector<int> previous_accepted_ids_;
  int previous_room_count_ = 0;
  int previous_time_slot_count_ = 0;
};

}  // namespace cms::services
