#include "api/auth_controller.hpp"

namespace cms::api {

HttpResponse AuthController::Register(const cms::services::RegisterRequest& request) {
  auto result = auth_service_->Register(request);

  using cms::services::RegisterStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case RegisterStatus::kCreated:
      return Created("registration successful", "/login");
    case RegisterStatus::kDuplicateEmail:
      return Conflict(result.message);
    case RegisterStatus::kMissingRequiredFields:
    case RegisterStatus::kInvalidEmailFormat:
    case RegisterStatus::kWeakPassword:
      return BadRequest(result.message);
  }
  return BadRequest("unknown registration error");  // GCOVR_EXCL_BR_LINE
}

HttpResponse AuthController::Login(const cms::services::LoginRequest& request) {
  auto result = auth_service_->LoginByUsername(request);

  using cms::services::LoginStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case LoginStatus::kAuthenticated: {
      if (session_service_ == nullptr) {
        return Ok("authenticated", "/home");
      }
      std::string token = session_service_->CreateSession(result.account.id);
      return Ok("authenticated session=" + token, "/home");
    }
    case LoginStatus::kMissingRequiredFields:
      return BadRequest(result.message);
    case LoginStatus::kInvalidCredentials:
      return Unauthorized(result.message);
  }
  return Unauthorized("unknown login error");  // GCOVR_EXCL_BR_LINE
}

HttpResponse AuthController::ChangePassword(
    const std::string& session_token, const cms::services::ChangePasswordRequest& request) {
  if (session_service_ == nullptr) {
    return Unauthorized("authentication required");
  }
  auto user_id = session_service_->GetUserIdForSession(session_token);
  if (!user_id.has_value()) {
    return Unauthorized("authentication required");
  }

  auto result = auth_service_->ChangePassword(user_id.value(), request);
  using cms::services::ChangePasswordStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case ChangePasswordStatus::kChanged:
      return Ok(result.message);
    case ChangePasswordStatus::kInvalidCurrentPassword:
      return Unauthorized(result.message);
    case ChangePasswordStatus::kMissingRequiredFields:
    case ChangePasswordStatus::kWeakNewPassword:
    case ChangePasswordStatus::kConfirmationMismatch:
    case ChangePasswordStatus::kReuseNotAllowed:
      return BadRequest(result.message);
    case ChangePasswordStatus::kUserNotFound:
      return Unauthorized(result.message);
  }
  return BadRequest("unknown password change error");  // GCOVR_EXCL_BR_LINE
}

HttpResponse AuthController::ProtectedPage(const std::string& session_token) const {
  if (session_service_ != nullptr && session_service_->IsSessionActive(session_token)) {
    return Ok("protected page");
  }
  return Unauthorized("authentication required");
}

HttpResponse AuthController::LoginPage(const std::string& session_token) const {
  if (session_service_ != nullptr && session_service_->IsSessionActive(session_token)) {
    return Ok("already authenticated", "/home");
  }
  return Ok("login page");
}

}  // namespace cms::api
