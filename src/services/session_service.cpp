#include "services/session_service.hpp"

namespace cms::services {

std::string SessionService::CreateSession(int user_id) {
  std::string token = "session-" + std::to_string(user_id) + "-" + std::to_string(next_token_++);
  sessions_[token] = user_id;
  return token;
}

bool SessionService::IsSessionActive(const std::string& token) const { return sessions_.contains(token); }

std::optional<int> SessionService::GetUserIdForSession(const std::string& token) const {
  auto it = sessions_.find(token);
  if (it == sessions_.end()) {
    return std::nullopt;
  }
  return it->second;
}

bool SessionService::HasActiveSessionForUser(int user_id) const {
  for (const auto& [token, uid] : sessions_) {
    (void)token;
    if (uid == user_id) {
      return true;
    }
  }
  return false;
}

int SessionService::SessionCount() const { return static_cast<int>(sessions_.size()); }

}  // namespace cms::services
