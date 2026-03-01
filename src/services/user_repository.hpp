#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "models/user_account.hpp"

namespace cms::services {

class UserRepository {
 public:
  bool EmailExists(const std::string& email) const;
  std::optional<cms::models::UserAccount> FindByEmail(const std::string& email) const;
  std::optional<cms::models::UserAccount> FindByUsername(const std::string& username) const;
  std::optional<cms::models::UserAccount> FindById(int id) const;
  bool UpdatePasswordByUserId(int user_id, const std::string& new_password_hash);
  cms::models::UserAccount Create(const std::string& name, const std::string& username,
                                  const std::string& email, const std::string& password_hash);

 private:
  static std::string Normalize(const std::string& value);

  int next_id_ = 1;
  std::unordered_map<std::string, cms::models::UserAccount> by_email_;
  std::unordered_map<std::string, cms::models::UserAccount> by_username_;
};

}  // namespace cms::services
