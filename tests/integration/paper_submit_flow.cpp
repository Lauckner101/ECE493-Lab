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

int main() {
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
  const std::string marker = "session=";
  const auto marker_pos = login.body.find(marker);
  Expect(marker_pos != std::string::npos, "expected active session token");
  const std::string token = login.body.substr(marker_pos + marker.size());

  auto unauth = paper_controller.SubmitPaper("bad-token", {"Flow Paper", "Flow abstract", "systems",
                                                            "flow.pdf", 200 * 1024});
  Expect(unauth.status == 401, "expected auth failure for invalid token");

  auto bad_format = paper_controller.SubmitPaper(token, {"Flow Paper", "Flow abstract", "systems",
                                                         "flow.txt", 200 * 1024});
  Expect(bad_format.status == 400, "expected file format validation failure");

  auto success = paper_controller.SubmitPaper(token, {"Flow Paper", "Flow abstract", "systems",
                                                      "flow.pdf", 200 * 1024});
  Expect(success.status == 201, "expected paper submit success");

  auto duplicate = paper_controller.SubmitPaper(token, {"Flow Paper", "Flow abstract", "systems",
                                                        "flow.pdf", 200 * 1024});
  Expect(duplicate.status == 200, "expected duplicate submission handling");

  Expect(papers.SubmittedCount() == 1, "expected one submitted paper in integration flow");
  auto paper = papers.FindById(1);
  Expect(paper.has_value(), "expected stored paper after submit");
  Expect(paper->status == cms::models::PaperStatus::kSubmitted,
         "expected stored paper status to be submitted");

  std::cout << "paper submit integration test passed\n";
  return 0;
}
