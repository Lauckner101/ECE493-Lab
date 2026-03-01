#include "api/schedule_controller.hpp"

namespace cms::api {

HttpResponse ScheduleController::GenerateSchedule(
    const cms::services::ScheduleGenerateRequest& request) {
  auto result = schedule_service_->GenerateSchedule(request);

  using cms::services::ScheduleGenerateStatus;
  switch (result.status) {
    case ScheduleGenerateStatus::kGenerated:
      return Created(result.html);
    case ScheduleGenerateStatus::kNoAcceptedPapers:
      return BadRequest(result.message);
    case ScheduleGenerateStatus::kUnsatisfiableConstraints:
      return Conflict(result.message);
    case ScheduleGenerateStatus::kSystemError:
      return InternalServerError(result.message);
  }
  return BadRequest("unknown scheduling error");
}

HttpResponse ScheduleController::EditCurrentSchedule(
    const cms::services::ScheduleEditRequest& request, bool is_editor) {
  if (!is_editor) {
    return Unauthorized("editor role required");
  }

  auto result = schedule_service_->EditCurrentSchedule(request);
  using cms::services::ScheduleEditStatus;
  switch (result.status) {
    case ScheduleEditStatus::kUpdated:
      return Ok(result.message);
    case ScheduleEditStatus::kNoSchedule:
    case ScheduleEditStatus::kNoChanges:
      return BadRequest(result.message);
    case ScheduleEditStatus::kConflict:
      return Conflict(result.message);
    case ScheduleEditStatus::kSystemError:
      return InternalServerError(result.message);
  }
  return BadRequest("unknown schedule edit error");
}

HttpResponse ScheduleController::ViewCurrentSchedule() const {
  auto schedule = schedule_service_->GetCurrentSchedule();
  if (!schedule.has_value()) {
    return BadRequest("no schedule available");
  }
  return Ok(schedule->html);
}

}  // namespace cms::api
