#pragma once

#include <string>

#include "models/user_account.hpp"
#include "services/user_repository.hpp"

namespace cms::services {

struct RegisterRequest {
  std::string name;
  std::string username;
  std::string email;
  std::string password;
};

enum class RegisterStatus {
  kCreated,
  kMissingRequiredFields,
  kInvalidEmailFormat,
  kWeakPassword,
  kDuplicateEmail,
};

struct RegisterResult {
  RegisterStatus status;
  std::string message;
  cms::models::UserAccount account;
};

struct LoginRequest {
  std::string username;
  std::string password;
};

enum class LoginStatus {
  kAuthenticated,
  kMissingRequiredFields,
  kInvalidCredentials,
};

struct LoginResult {
  LoginStatus status;
  std::string message;
  cms::models::UserAccount account;
};

struct ChangePasswordRequest {
  std::string current_password;
  std::string new_password;
  std::string confirm_password;
};

enum class ChangePasswordStatus {
  kChanged,
  kMissingRequiredFields,
  kInvalidCurrentPassword,
  kWeakNewPassword,
  kConfirmationMismatch,
  kReuseNotAllowed,
  kUserNotFound,
};

struct ChangePasswordResult {
  ChangePasswordStatus status;
  std::string message;
};

class AuthService {
 public:
  explicit AuthService(UserRepository* repository) : repository_(repository) {}

  RegisterResult Register(const RegisterRequest& request);
  LoginResult LoginByUsername(const LoginRequest& request) const;
  ChangePasswordResult ChangePassword(int user_id, const ChangePasswordRequest& request);
  bool Login(const std::string& identifier, const std::string& password) const;

 private:
  UserRepository* repository_;
};

}  // namespace cms::services
