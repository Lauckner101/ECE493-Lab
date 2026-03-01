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

    auto reg = auth.Register({"Draft Author", "draftauthor", "draftauthor@example.com", "Author#123"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated, "precondition: registration failed");
    auto login = auth_controller.Login({"draftauthor", "Author#123"});
    std::string token = ExtractSessionToken(login.body);
    Expect(!token.empty(), "precondition: login failed");

    // AT-UC05-001: save draft with partial info.
    auto save = paper_controller.SaveDraft(token, {"Draft 1", "", ""});
    Expect(save.status == 201, "AT-UC05-001: expected draft save success");
    Expect(papers.DraftCount() == 1, "AT-UC05-001: expected one draft");

    // AT-UC05-002: update existing draft without creating duplicate.
    auto first_draft = papers.SaveDraft(reg.account.id, {"Original", "v1", "k"});
    Expect(first_draft.status == cms::services::SaveDraftStatus::kSaved,
           "AT-UC05-002: precondition save failed");
    int before_update_count = papers.DraftCount();
    auto update = paper_controller.SaveDraft(token, {"Original Updated", "v2", "k2", first_draft.draft_id});
    Expect(update.status == 201, "AT-UC05-002: expected update success");
    Expect(papers.DraftCount() == before_update_count,
           "AT-UC05-002: expected no duplicate draft after update");

    // AT-UC05-003: invalid info blocks save.
    auto invalid = paper_controller.SaveDraft(token, {"bad@title", "v", "k"});
    Expect(invalid.status == 400, "AT-UC05-003: expected invalid draft to be blocked");

    // AT-UC05-004: no information entered -> warning/block per configured rule.
    auto empty = paper_controller.SaveDraft(token, {"", "", ""});
    Expect(empty.status == 400, "AT-UC05-004: expected empty draft warning/block");

    // AT-UC05-005: draft not marked as submitted.
    auto draft_entity = papers.FindById(first_draft.draft_id);
    Expect(draft_entity.has_value(), "AT-UC05-005: expected draft retrievable");
    Expect(draft_entity->status == cms::models::PaperStatus::kDraft,
           "AT-UC05-005: expected draft status");

    // AT-UC05-006: system error during save.
    auto sys_fail = paper_controller.SaveDraft(token, {"retry me", "a", "b", 0, true});
    Expect(sys_fail.status == 500, "AT-UC05-006: expected system error response");

    // AT-UC05-007: retry save after failure succeeds.
    auto retry = paper_controller.SaveDraft(token, {"retry me", "a", "b"});
    Expect(retry.status == 201, "AT-UC05-007: expected retry save success");

    std::cout << "UC-05 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
