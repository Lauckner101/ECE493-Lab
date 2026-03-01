#pragma once

#include "api/http_errors.hpp"
#include "services/decision_service.hpp"

namespace cms::api {

class DecisionController {
 public:
  explicit DecisionController(cms::services::DecisionService* decision_service)
      : decision_service_(decision_service) {}

  HttpResponse RecordDecision(const cms::services::DecisionRequest& request);

 private:
  cms::services::DecisionService* decision_service_;
};

}  // namespace cms::api
