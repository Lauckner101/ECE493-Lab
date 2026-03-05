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

  auto reg = auth.Register({"Draft Flow", "draftflow", "draftflow@example.com", "Author#123"});
  Expect(reg.status == cms::services::RegisterStatus::kCreated,
         "expected registration before draft integration flow");

  auto login = auth_controller.Login({"draftflow", "Author#123"});
  const std::string marker = "session=";
  const auto marker_pos = login.body.find(marker);
  Expect(marker_pos != std::string::npos, "expected session token");
  const std::string token = login.body.substr(marker_pos + marker.size());

  auto empty = paper_controller.SaveDraft(token, {"", "", ""});
  Expect(empty.status == 400, "expected empty draft rejection");

  auto invalid_data = paper_controller.SaveDraft(token, {"bad@title", "Partial", "cms"});
  Expect(invalid_data.status == 400, "expected invalid draft data rejection");

  auto saved = paper_controller.SaveDraft(token, {"Draft Integration", "Partial", "cms"});
  Expect(saved.status == 201, "expected draft save success");
  Expect(papers.DraftCount() == 1, "expected one draft after save");

  auto missing_draft = paper_controller.SubmitDraft(token, 99, "draft-flow.pdf", 10240);
  Expect(missing_draft.status == 401, "expected missing draft failure");

  auto bad_format = paper_controller.SubmitDraft(token, 1, "draft-flow.txt", 10240);
  Expect(bad_format.status == 400, "expected file format validation for draft submit");

  auto submit = paper_controller.SubmitDraft(token, 1, "draft-flow.pdf", 10240);
  Expect(submit.status == 200, "expected draft submit success");

  auto paper = papers.FindById(1);
  Expect(paper.has_value(), "expected persisted draft/submission");
  Expect(paper->status == cms::models::PaperStatus::kSubmitted,
         "expected draft to become submitted after PUT /papers/{paperId}/submit");

  std::cout << "draft submit integration test passed\n";
  return 0;
}
