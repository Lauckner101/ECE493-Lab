// GCOVR_EXCL_START
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

int main() {
  try {
    cms::services::PaperService papers;
    cms::services::RefereeService referee_service(&papers);
    cms::services::ReviewService review_service(&referee_service);
    cms::services::DecisionService decision_service(&papers, &review_service);

    cms::api::RefereeController referee_controller(&referee_service);
    cms::api::ReviewController review_controller(&review_service);
    cms::api::DecisionController decision_controller(&decision_service);

    auto paper = papers.SubmitPaper(1, {"Decision Integration", "A", "k", "di.pdf", 2048});
    Expect(paper.status == cms::services::SubmitPaperStatus::kSubmitted,
           "precondition failed: paper submit");

    auto a1 = referee_service.AssignReferee(paper.paper_id, "i1@example.com");
    auto a2 = referee_service.AssignReferee(paper.paper_id, "i2@example.com");
    auto a3 = referee_service.AssignReferee(paper.paper_id, "i3@example.com");
    referee_controller.AcceptInvitation(a1.assignment_id);
    referee_controller.AcceptInvitation(a2.assignment_id);
    referee_controller.AcceptInvitation(a3.assignment_id);

    review_controller.SubmitReview({paper.paper_id, "i1@example.com", "good", "accept"});
    review_controller.SubmitReview({paper.paper_id, "i2@example.com", "good", "accept"});
    review_controller.SubmitReview({paper.paper_id, "i3@example.com", "good", "accept"});

    auto decision = decision_controller.RecordDecision({paper.paper_id, "accept"});
    Expect(decision.status == 201, "expected decision flow success");

    auto stored = decision_service.GetDecisionForPaper(paper.paper_id);
    Expect(stored.has_value() && stored->decision == "accept",
           "expected stored decision in integration flow");

    auto paper_after = papers.FindById(paper.paper_id);
    Expect(paper_after.has_value() && paper_after->status == cms::models::PaperStatus::kAccepted,
           "expected paper status transition after decision");

    std::cout << "decision flow integration test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
// GCOVR_EXCL_STOP
