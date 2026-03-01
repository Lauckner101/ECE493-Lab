#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace cms::services {

class SessionService {
 public:
  std::string CreateSession(int user_id);
  bool IsSessionActive(const std::string& token) const;
  std::optional<int> GetUserIdForSession(const std::string& token) const;
  bool HasActiveSessionForUser(int user_id) const;
  int SessionCount() const;

 private:
  int next_token_ = 1;
  std::unordered_map<std::string, int> sessions_;
};

}  // namespace cms::services
