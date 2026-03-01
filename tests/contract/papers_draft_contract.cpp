#include <iostream>
#include <string>

#include "api/auth_controller.hpp"
#include "api/paper_controller.hpp"
#include "services/auth_service.hpp"
#include "services/paper_service.hpp"
#include "services/session_service.hpp"
#include "services/user_repository.hpp"
#include "test_assert.hpp"

namespace {
std::string ExtractSessionToken(const std::string& body) {
  const std::string marker = "session=";
  auto pos = body.find(marker);
  if (pos == std::string::npos) {
    return "";
  }
  return body.substr(pos + marker.size());
}
}  // namespace

int main() {
  try {
    cms::services::UserRepository users;
    cms::services::AuthService auth(&users);
    cms::services::SessionService sessions;
    cms::services::PaperService papers;

    cms::api::AuthController auth_controller(&auth, &sessions);
    cms::api::PaperController paper_controller(&papers, &sessions);

    auto reg = auth.Register({"Draft Contract", "draftc", "draftc@example.com", "Author#123"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated, "precondition registration failed");
    auto login = auth_controller.Login({"draftc", "Author#123"});
    auto token = ExtractSessionToken(login.body);

    auto created = paper_controller.SaveDraft(token, {"Draft Contract Title", "abstract", "k"});
    Expect(created.status == 201, "expected contract 201 for POST /papers/drafts");

    auto bad = paper_controller.SaveDraft(token, {"", "", ""});
    Expect(bad.status == 400, "expected contract 400 for empty draft");

    std::cout << "papers draft contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
