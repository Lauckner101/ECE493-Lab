#pragma once

#include <string>

#include "api/http_errors.hpp"
#include "services/payment_service.hpp"
#include "services/registration_service.hpp"
#include "services/session_service.hpp"

namespace cms::api {

class RegistrationController {
 public:
  explicit RegistrationController(cms::services::RegistrationService* registration_service,
                                  cms::services::PaymentService* payment_service,
                                  cms::services::SessionService* session_service)
      : registration_service_(registration_service),
        payment_service_(payment_service),
        session_service_(session_service) {}

  HttpResponse CreateRegistration(const std::string& session_token,
                                  const cms::services::CreateRegistrationRequest& request);
  HttpResponse PayForRegistration(const std::string& session_token, int registration_id,
                                  const cms::services::PaymentRequest& request);
  HttpResponse ViewMyRegistrationStatus(const std::string& session_token) const;

 private:
  cms::services::RegistrationService* registration_service_;
  cms::services::PaymentService* payment_service_;
  cms::services::SessionService* session_service_;
};

}  // namespace cms::api
