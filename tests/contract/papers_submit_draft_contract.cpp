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

    auto reg = auth.Register({"Submit Draft Contract", "sdc", "sdc@example.com", "Author#123"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated, "precondition registration failed");
    auto login = auth_controller.Login({"sdc", "Author#123"});
    auto token = ExtractSessionToken(login.body);

    auto draft = papers.SaveDraft(reg.account.id, {"Ready Draft", "Good abstract", "cms"});
    Expect(draft.status == cms::services::SaveDraftStatus::kSaved,
           "precondition draft save failed for submit-draft contract test");

    auto ok = paper_controller.SubmitDraft(token, draft.draft_id, "ready.pdf", 2048);
    Expect(ok.status == 200, "expected contract 200 for PUT /papers/{paperId}/submit");

    auto invalid = paper_controller.SubmitDraft(token, 99999, "ready.pdf", 2048);
    Expect(invalid.status == 401, "expected contract 401 when draft id is invalid");

    std::cout << "papers submit draft contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
