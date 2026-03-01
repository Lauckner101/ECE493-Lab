#include <iostream>

#include "api/decision_controller.hpp"
#include "api/referee_controller.hpp"
#include "api/review_controller.hpp"
#include "models/paper_submission.hpp"
#include "services/decision_service.hpp"
#include "services/paper_service.hpp"
#include "services/referee_service.hpp"
#include "services/review_service.hpp"
#include "test_assert.hpp"

namespace {
void AddAcceptedReview(cms::services::PaperService* papers, cms::services::RefereeService* referees,
                       cms::api::RefereeController* referee_controller,
                       cms::api::ReviewController* review_controller, int paper_id,
                       const std::string& referee_email, const std::string& recommendation) {
  auto invite = referees->AssignReferee(paper_id, referee_email);
  referee_controller->AcceptInvitation(invite.assignment_id);
  auto review = review_controller->SubmitReview(
      {paper_id, referee_email, "Review from " + referee_email, recommendation});
  Expect(review.status == 201, "setup failed: could not submit prerequisite review");
}
}  // namespace

int main() {
  try {
    cms::services::PaperService papers;
    cms::services::RefereeService referee_service(&papers);
    cms::services::ReviewService review_service(&referee_service);
    cms::services::DecisionService decision_service(&papers, &review_service);

    cms::api::RefereeController referee_controller(&referee_service);
    cms::api::ReviewController review_controller(&review_service);
    cms::api::DecisionController decision_controller(&decision_service);

    // AT-UC09-001: accept paper successfully.
    auto p1 = papers.SubmitPaper(1, {"Decision A", "A", "k", "a.pdf", 2048});
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p1.paper_id,
                      "r1a@example.com", "accept");
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p1.paper_id,
                      "r2a@example.com", "accept");
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p1.paper_id,
                      "r3a@example.com", "accept");
    auto accept = decision_controller.RecordDecision({p1.paper_id, "accept"});
    Expect(accept.status == 201, "AT-UC09-001: expected accept success");
    auto p1_after = papers.FindById(p1.paper_id);
    Expect(p1_after.has_value() && p1_after->status == cms::models::PaperStatus::kAccepted,
           "AT-UC09-001: paper status should be Accepted");

    // AT-UC09-002: reject paper successfully.
    auto p2 = papers.SubmitPaper(1, {"Decision R", "A", "k", "r.pdf", 2048});
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p2.paper_id,
                      "r1r@example.com", "reject");
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p2.paper_id,
                      "r2r@example.com", "reject");
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p2.paper_id,
                      "r3r@example.com", "reject");
    auto reject = decision_controller.RecordDecision({p2.paper_id, "reject"});
    Expect(reject.status == 201, "AT-UC09-002: expected reject success");
    auto p2_after = papers.FindById(p2.paper_id);
    Expect(p2_after.has_value() && p2_after->status == cms::models::PaperStatus::kRejected,
           "AT-UC09-002: paper status should be Rejected");

    // AT-UC09-003: insufficient reviews.
    auto p3 = papers.SubmitPaper(1, {"Decision I", "A", "k", "i.pdf", 2048});
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p3.paper_id,
                      "r1i@example.com", "accept");
    auto insufficient = decision_controller.RecordDecision({p3.paper_id, "accept"});
    Expect(insufficient.status == 400, "AT-UC09-003: decision should be blocked if <3 reviews");

    // AT-UC09-004: no decision selected.
    auto p4 = papers.SubmitPaper(1, {"Decision N", "A", "k", "n.pdf", 2048});
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p4.paper_id,
                      "r1n@example.com", "accept");
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p4.paper_id,
                      "r2n@example.com", "accept");
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p4.paper_id,
                      "r3n@example.com", "accept");
    auto none = decision_controller.RecordDecision({p4.paper_id, ""});
    Expect(none.status == 400, "AT-UC09-004: missing decision should be blocked");

    // AT-UC09-005: system error while saving.
    auto p5 = papers.SubmitPaper(1, {"Decision E", "A", "k", "e.pdf", 2048});
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p5.paper_id,
                      "r1e@example.com", "accept");
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p5.paper_id,
                      "r2e@example.com", "accept");
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p5.paper_id,
                      "r3e@example.com", "accept");
    auto err = decision_controller.RecordDecision({p5.paper_id, "accept", true});
    Expect(err.status == 500, "AT-UC09-005: expected save failure response");

    // AT-UC09-006: author notification content.
    auto p6 = papers.SubmitPaper(1, {"Decision C", "A", "k", "c.pdf", 2048});
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p6.paper_id,
                      "r1c@example.com", "accept");
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p6.paper_id,
                      "r2c@example.com", "accept");
    AddAcceptedReview(&papers, &referee_service, &referee_controller, &review_controller, p6.paper_id,
                      "r3c@example.com", "accept");
    auto dec6 = decision_controller.RecordDecision({p6.paper_id, "accept"});
    Expect(dec6.status == 201, "AT-UC09-006: decision save failed");
    auto note = decision_service.LastAuthorNotification();
    Expect(note.find("accept") != std::string::npos,
           "AT-UC09-006: author notification should include decision");

    // AT-UC09-007: prevent duplicate decision submission.
    auto dup = decision_controller.RecordDecision({p6.paper_id, "reject"});
    Expect(dup.status == 409, "AT-UC09-007: duplicate decision should be blocked");

    // AT-UC09-008: decision persistence.
    auto persisted = decision_service.GetDecisionForPaper(p6.paper_id);
    Expect(persisted.has_value() && persisted->decision == "accept",
           "AT-UC09-008: stored decision should persist after reload/access");

    std::cout << "UC-09 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
