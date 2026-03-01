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

    auto reg = auth.Register({"Flow Author", "flowauthor", "flowauthor@example.com", "Author#123"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated,
           "expected author registration before submission flow");

    auto login = auth_controller.Login({"flowauthor", "Author#123"});
    std::string token = ExtractSessionToken(login.body);
    Expect(!token.empty(), "expected active session token");

    auto submit = paper_controller.SubmitPaper(token, {"Flow Paper", "Flow abstract", "systems",
                                                       "flow.pdf", 200 * 1024});
    Expect(submit.status == 201, "expected paper submit success");

    Expect(papers.SubmittedCount() == 1, "expected one submitted paper in integration flow");
    auto paper = papers.FindById(1);
    Expect(paper.has_value(), "expected stored paper after submit");
    Expect(paper->status == cms::models::PaperStatus::kSubmitted,
           "expected stored paper status to be submitted");

    std::cout << "paper submit integration test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
