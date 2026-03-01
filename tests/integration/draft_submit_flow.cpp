#include <iostream>
#include <string>

#include "api/auth_controller.hpp"
#include "api/paper_controller.hpp"
#include "models/paper_submission.hpp"
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

    auto reg = auth.Register({"Draft Flow", "draftflow", "draftflow@example.com", "Author#123"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated,
           "expected registration before draft integration flow");

    auto login = auth_controller.Login({"draftflow", "Author#123"});
    auto token = ExtractSessionToken(login.body);
    Expect(!token.empty(), "expected session token");

    auto saved = paper_controller.SaveDraft(token, {"Draft Integration", "Partial", "cms"});
    Expect(saved.status == 201, "expected draft save success");
    Expect(papers.DraftCount() == 1, "expected one draft after save");

    auto submit = paper_controller.SubmitDraft(token, 1, "draft-flow.pdf", 10240);
    Expect(submit.status == 200, "expected draft submit success");

    auto paper = papers.FindById(1);
    Expect(paper.has_value(), "expected persisted draft/submission");
    Expect(paper->status == cms::models::PaperStatus::kSubmitted,
           "expected draft to become submitted after PUT /papers/{paperId}/submit");

    std::cout << "draft submit integration test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
