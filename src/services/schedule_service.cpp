#include "services/schedule_service.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace cms::services {

std::vector<std::string> ScheduleService::BuildRoomNames(int room_count) {
  std::vector<std::string> rooms;
  for (int i = 1; i <= room_count; ++i) {
    rooms.push_back("Room " + std::to_string(i));
  }
  return rooms;
}

std::vector<std::string> ScheduleService::BuildTimeSlots(int time_slot_count) {
  std::vector<std::string> slots;
  for (int i = 1; i <= time_slot_count; ++i) {
    slots.push_back("Slot " + std::to_string(i));
  }
  return slots;
}

std::string ScheduleService::RenderHtml(const std::vector<cms::models::ScheduleItem>& items) {
  std::ostringstream html;
  html << "<table><thead><tr><th>Paper</th><th>Room</th><th>Time Slot</th></tr></thead><tbody>";
  for (const auto& item : items) {
    html << "<tr><td>" << item.paper_id << "</td><td>" << item.room << "</td><td>" << item.time_slot
         << "</td></tr>";
  }
  html << "</tbody></table>";
  return html.str();
}

bool ScheduleService::InputsMatch(const std::vector<int>& accepted_ids,
                                  const ScheduleGenerateRequest& request,
                                  const std::vector<int>& previous_ids, int previous_room_count,
                                  int previous_time_slot_count) {
  return accepted_ids == previous_ids && request.room_count == previous_room_count &&
         request.time_slot_count == previous_time_slot_count;
}

std::vector<cms::models::ScheduleItem> ScheduleService::NormalizeItems(
    const std::vector<cms::models::ScheduleItem>& items) {
  std::vector<cms::models::ScheduleItem> normalized = items;
  std::sort(normalized.begin(), normalized.end(),
            [](const cms::models::ScheduleItem& a, const cms::models::ScheduleItem& b) {
              return a.paper_id < b.paper_id;
            });
  return normalized;
}

bool ScheduleService::ItemsConflict(const std::vector<cms::models::ScheduleItem>& items) {
  std::unordered_set<int> paper_ids;
  std::unordered_set<std::string> room_slot_keys;

  for (const auto& item : items) {
    if (item.paper_id <= 0 || item.room.empty() || item.time_slot.empty()) {
      return true;
    }
    if (!paper_ids.insert(item.paper_id).second) {
      return true;
    }
    std::string key = item.room + "|" + item.time_slot;
    if (!room_slot_keys.insert(key).second) {
      return true;
    }
  }

  return false;
}

ScheduleGenerateResult ScheduleService::GenerateSchedule(const ScheduleGenerateRequest& request) {
  auto accepted_ids = paper_service_->GetAcceptedPaperIds();
  if (accepted_ids.empty()) {
    return {ScheduleGenerateStatus::kNoAcceptedPapers,
            "schedule generation requires at least one accepted paper", 0, ""};
  }

  if (request.room_count <= 0 || request.time_slot_count <= 0) {
    return {ScheduleGenerateStatus::kUnsatisfiableConstraints,
            "rooms and time slots must be configured", 0, ""};
  }

  const int capacity = request.room_count * request.time_slot_count;
  if (static_cast<int>(accepted_ids.size()) > capacity) {
    return {ScheduleGenerateStatus::kUnsatisfiableConstraints,
            "available rooms/time slots cannot satisfy scheduling constraints", 0, ""};
  }

  if (request.simulate_save_failure) {
    return {ScheduleGenerateStatus::kSystemError, "schedule could not be saved", 0, ""};
  }

  if (current_schedule_.has_value() &&
      InputsMatch(accepted_ids, request, previous_accepted_ids_, previous_room_count_,
                  previous_time_slot_count_)) {
    return {ScheduleGenerateStatus::kGenerated, "schedule already up to date",
            current_schedule_->id, current_schedule_->html};
  }

  auto rooms = BuildRoomNames(request.room_count);
  auto slots = BuildTimeSlots(request.time_slot_count);

  std::vector<cms::models::ScheduleItem> items;
  items.reserve(accepted_ids.size());
  for (size_t i = 0; i < accepted_ids.size(); ++i) {
    const int room_index = static_cast<int>(i % rooms.size());
    const int slot_index = static_cast<int>(i / rooms.size());
    items.push_back({accepted_ids[i], rooms[room_index], slots[slot_index]});
  }

  cms::models::Schedule schedule{next_schedule_id_++, items, RenderHtml(items)};
  current_schedule_ = schedule;
  previous_accepted_ids_ = accepted_ids;
  previous_room_count_ = request.room_count;
  previous_time_slot_count_ = request.time_slot_count;

  return {ScheduleGenerateStatus::kGenerated, "schedule generated", schedule.id, schedule.html};
}

ScheduleEditResult ScheduleService::EditCurrentSchedule(const ScheduleEditRequest& request) {
  if (!current_schedule_.has_value()) {
    return {ScheduleEditStatus::kNoSchedule, "no current schedule to edit", 0, ""};
  }
  if (request.simulate_save_failure) {
    return {ScheduleEditStatus::kSystemError, "schedule update could not be saved", 0, ""};
  }

  auto current_items = NormalizeItems(current_schedule_->items);
  auto edited_items = NormalizeItems(request.items);

  if (current_items == edited_items) {
    return {ScheduleEditStatus::kNoChanges, "no schedule updates detected", current_schedule_->id,
            current_schedule_->html};
  }

  if (ItemsConflict(edited_items) || edited_items.size() != current_items.size()) {
    return {ScheduleEditStatus::kConflict,
            "conflicting or invalid schedule assignments detected", 0, ""};
  }

  for (size_t i = 0; i < current_items.size(); ++i) {
    if (current_items[i].paper_id != edited_items[i].paper_id) {
      return {ScheduleEditStatus::kConflict,
              "edited schedule must contain the same set of papers", 0, ""};
    }
  }

  cms::models::Schedule updated{next_schedule_id_++, request.items, RenderHtml(request.items)};
  current_schedule_ = updated;
  return {ScheduleEditStatus::kUpdated, "schedule updated", updated.id, updated.html};
}

std::optional<cms::models::Schedule> ScheduleService::GetCurrentSchedule() const {
  return current_schedule_;
}

std::string ScheduleService::GetCurrentScheduleHtml() const {
  if (!current_schedule_.has_value()) {
    return "";
  }
  return current_schedule_->html;
}

}  // namespace cms::services
