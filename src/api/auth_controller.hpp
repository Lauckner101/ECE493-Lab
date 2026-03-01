#pragma once

#include "api/http_errors.hpp"
#include "services/auth_service.hpp"
#include "services/session_service.hpp"

namespace cms::api {

class AuthController {
 public:
  explicit AuthController(cms::services::AuthService* auth_service,
                          cms::services::SessionService* session_service = nullptr)
      : auth_service_(auth_service), session_service_(session_service) {}

  HttpResponse Register(const cms::services::RegisterRequest& request);
  HttpResponse Login(const cms::services::LoginRequest& request);
  HttpResponse ChangePassword(const std::string& session_token,
                              const cms::services::ChangePasswordRequest& request);
  HttpResponse ProtectedPage(const std::string& session_token) const;
  HttpResponse LoginPage(const std::string& session_token) const;

 private:
  cms::services::AuthService* auth_service_;
  cms::services::SessionService* session_service_;
};

}  // namespace cms::api
