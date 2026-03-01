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

    auto reg = auth.Register({"Author", "author1", "author1@example.com", "Author#123"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated, "precondition: registration failed");
    auto login = auth_controller.Login({"author1", "Author#123"});
    std::string token = ExtractSessionToken(login.body);
    Expect(!token.empty(), "precondition: login failed");

    // AT-UC04-001: successful submission.
    auto ok = paper_controller.SubmitPaper(token, {"Paper Title", "A short abstract", "ai,systems",
                                                   "paper.pdf", 1024 * 1024});
    Expect(ok.status == 201, "AT-UC04-001: expected 201");
    Expect(papers.SubmittedCount() == 1, "AT-UC04-001: expected one submitted paper");

    // AT-UC04-002: missing metadata.
    auto missing = paper_controller.SubmitPaper(token, {"", "A short abstract", "ai,systems",
                                                        "paper2.pdf", 1024 * 1024});
    Expect(missing.status == 400, "AT-UC04-002: expected 400");

    // AT-UC04-003: invalid format.
    auto bad_format = paper_controller.SubmitPaper(token, {"Paper Two", "A short abstract", "ai,systems",
                                                           "paper.docx", 1024 * 1024});
    Expect(bad_format.status == 400, "AT-UC04-003: expected 400");

    // AT-UC04-004: file too large.
    auto too_large = paper_controller.SubmitPaper(token, {"Paper Three", "A short abstract", "ai,systems",
                                                          "paper3.pdf", 11 * 1024 * 1024});
    Expect(too_large.status == 400, "AT-UC04-004: expected 400");

    // AT-UC04-005: save draft.
    auto saved = paper_controller.SaveDraft(token, {"Draft Title", "", ""});
    Expect(saved.status == 201, "AT-UC04-005: expected draft save success");

    // AT-UC04-006: submit saved draft after completion.
    auto draft_res = papers.SaveDraft(reg.account.id, {"Draft Complete", "draft abstract", "cms"});
    Expect(draft_res.status == cms::services::SaveDraftStatus::kSaved,
           "AT-UC04-006: precondition draft save failed");
    auto submit_draft = paper_controller.SubmitDraft(token, draft_res.draft_id, "draft.pdf", 2048);
    Expect(submit_draft.status == 200, "AT-UC04-006: expected draft submission success");

    // AT-UC04-007: duplicate submit only creates one submission.
    auto first = paper_controller.SubmitPaper(token, {"No Dup", "same", "k", "nodup.pdf", 4096});
    auto second = paper_controller.SubmitPaper(token, {"No Dup", "same", "k", "nodup.pdf", 4096});
    Expect(first.status == 201, "AT-UC04-007: first submit should succeed");
    Expect(second.status == 200, "AT-UC04-007: second duplicate submit should be ignored");
    Expect(papers.SubmittedCount() == 3,
           "AT-UC04-007: total submitted count should not increase on duplicate");

    std::cout << "UC-04 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
