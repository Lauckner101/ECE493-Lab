#include "services/user_repository.hpp"

#include <algorithm>
#include <cctype>

namespace cms::services {

std::string UserRepository::Normalize(const std::string& value) {
  std::string lowered = value;
  std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return lowered;
}

bool UserRepository::EmailExists(const std::string& email) const {
  return by_email_.contains(Normalize(email));
}

std::optional<cms::models::UserAccount> UserRepository::FindByEmail(const std::string& email) const {
  auto key = Normalize(email);
  auto it = by_email_.find(key);
  if (it == by_email_.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::optional<cms::models::UserAccount> UserRepository::FindByUsername(
    const std::string& username) const {
  auto key = Normalize(username);
  auto it = by_username_.find(key);
  if (it == by_username_.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::optional<cms::models::UserAccount> UserRepository::FindById(int id) const {
  for (const auto& [key, account] : by_email_) {
    (void)key;
    if (account.id == id) {
      return account;
    }
  }
  return std::nullopt;
}

bool UserRepository::UpdatePasswordByUserId(int user_id, const std::string& new_password_hash) {
  bool updated = false;

  for (auto& [key, account] : by_email_) {
    (void)key;
    if (account.id == user_id) {
      account.password_hash = new_password_hash;
      updated = true;
    }
  }

  for (auto& [key, account] : by_username_) {
    (void)key;
    if (account.id == user_id) {
      account.password_hash = new_password_hash;
      updated = true;
    }
  }

  return updated;
}

cms::models::UserAccount UserRepository::Create(const std::string& name, const std::string& username,
                                                const std::string& email,
                                                const std::string& password_hash) {
  cms::models::UserAccount account{next_id_++, name, Normalize(username), Normalize(email),  // GCOVR_EXCL_BR_LINE
                                   password_hash};  // GCOVR_EXCL_BR_LINE
  by_email_[account.email] = account;
  by_username_[account.username] = account;
  return account;
}

}  // namespace cms::services
