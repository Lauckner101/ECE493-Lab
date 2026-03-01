#include "services/auth_service.hpp"

#include "lib/validation.hpp"

namespace cms::services {

RegisterResult AuthService::Register(const RegisterRequest& request) {
  if (cms::lib::IsBlank(request.name) || cms::lib::IsBlank(request.username) ||
      cms::lib::IsBlank(request.email) || cms::lib::IsBlank(request.password)) {
    return {RegisterStatus::kMissingRequiredFields, "missing required fields", {0, "", "", "", ""}};
  }

  if (!cms::lib::IsValidEmailFormat(request.email)) {
    return {RegisterStatus::kInvalidEmailFormat, "invalid email format", {0, "", "", "", ""}};
  }

  if (!cms::lib::IsPasswordComplex(request.password)) {
    return {RegisterStatus::kWeakPassword, "password does not meet complexity rules",
            {0, "", "", "", ""}};
  }

  if (repository_->EmailExists(request.email)) {
    return {RegisterStatus::kDuplicateEmail, "email already registered", {0, "", "", "", ""}};
  }

  auto account = repository_->Create(request.name, request.username, request.email, request.password);
  return {RegisterStatus::kCreated, "account created", account};
}

LoginResult AuthService::LoginByUsername(const LoginRequest& request) const {
  if (cms::lib::IsBlank(request.username) || cms::lib::IsBlank(request.password)) {
    return {LoginStatus::kMissingRequiredFields, "missing required fields", {0, "", "", "", ""}};
  }

  auto account = repository_->FindByUsername(request.username);
  if (!account.has_value() || account->password_hash != request.password) {
    return {LoginStatus::kInvalidCredentials, "invalid credentials", {0, "", "", "", ""}};
  }

  return {LoginStatus::kAuthenticated, "authenticated", account.value()};
}

ChangePasswordResult AuthService::ChangePassword(int user_id, const ChangePasswordRequest& request) {
  if (cms::lib::IsBlank(request.current_password) || cms::lib::IsBlank(request.new_password) ||
      cms::lib::IsBlank(request.confirm_password)) {
    return {ChangePasswordStatus::kMissingRequiredFields, "missing required fields"};
  }

  if (request.new_password != request.confirm_password) {
    return {ChangePasswordStatus::kConfirmationMismatch, "password confirmation mismatch"};
  }

  auto account = repository_->FindById(user_id);
  if (!account.has_value()) {
    return {ChangePasswordStatus::kUserNotFound, "user not found"};
  }

  if (account->password_hash != request.current_password) {
    return {ChangePasswordStatus::kInvalidCurrentPassword, "invalid current password"};
  }

  if (request.new_password == request.current_password) {
    return {ChangePasswordStatus::kReuseNotAllowed, "new password cannot match current password"};
  }

  if (!cms::lib::IsPasswordComplex(request.new_password)) {
    return {ChangePasswordStatus::kWeakNewPassword, "password does not meet complexity rules"};
  }

  repository_->UpdatePasswordByUserId(user_id, request.new_password);
  return {ChangePasswordStatus::kChanged, "password updated"};
}

bool AuthService::Login(const std::string& identifier, const std::string& password) const {
  auto by_username = repository_->FindByUsername(identifier);
  if (by_username.has_value()) {
    return by_username->password_hash == password;
  }

  auto by_email = repository_->FindByEmail(identifier);
  if (!by_email.has_value()) {
    return false;
  }
  return by_email->password_hash == password;
}

}  // namespace cms::services
