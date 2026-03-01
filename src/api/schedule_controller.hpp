#pragma once

#include "api/http_errors.hpp"
#include "services/schedule_service.hpp"

namespace cms::api {

class ScheduleController {
 public:
  explicit ScheduleController(cms::services::ScheduleService* schedule_service)
      : schedule_service_(schedule_service) {}

  HttpResponse GenerateSchedule(const cms::services::ScheduleGenerateRequest& request);
  HttpResponse EditCurrentSchedule(const cms::services::ScheduleEditRequest& request,
                                   bool is_editor = true);
  HttpResponse ViewCurrentSchedule() const;

 private:
  cms::services::ScheduleService* schedule_service_;
};

}  // namespace cms::api
