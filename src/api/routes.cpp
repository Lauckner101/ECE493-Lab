#include <string>
#include <vector>

namespace cms::api {

std::vector<std::string> RegisteredRoutes() {
  return {"POST /auth/register", "POST /auth/login", "POST /auth/password/change", "POST /papers",
          "POST /papers/drafts", "PUT /papers/{paperId}/submit", "POST /papers/{paperId}/referees",
          "POST /invitations/{invitationId}/accept", "POST /invitations/{invitationId}/reject",
          "POST /papers/{paperId}/reviews", "POST /papers/{paperId}/decision",
          "POST /schedule/generate", "PUT /schedule/current",
          "POST /conference/registration", "POST /conference/registration/{id}/pay"};
}

}  // namespace cms::api
