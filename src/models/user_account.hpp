#pragma once

#include <string>

namespace cms::models {

struct UserAccount {
  int id;
  std::string name;
  std::string username;
  std::string email;
  std::string password_hash;
};

}  // namespace cms::models
