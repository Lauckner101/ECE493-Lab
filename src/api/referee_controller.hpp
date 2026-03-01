#pragma once

#include <string>

#include "api/http_errors.hpp"
#include "services/referee_service.hpp"

namespace cms::api {

class RefereeController {
 public:
  explicit RefereeController(cms::services::RefereeService* referee_service)
      : referee_service_(referee_service) {}

  HttpResponse AssignReferee(int paper_id, const std::string& referee_email);
  HttpResponse AcceptInvitation(int invitation_id);
  HttpResponse RejectInvitation(int invitation_id);
  HttpResponse ExitAssignment(int paper_id);

 private:
  cms::services::RefereeService* referee_service_;
};

}  // namespace cms::api
