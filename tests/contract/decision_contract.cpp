#include <iostream>

#include "api/decision_controller.hpp"
#include "api/referee_controller.hpp"
#include "api/review_controller.hpp"
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

    auto paper = papers.SubmitPaper(1, {"Decision Contract", "A", "k", "dc.pdf", 2048});

    auto i1 = referee_service.AssignReferee(paper.paper_id, "d1@example.com");
    auto i2 = referee_service.AssignReferee(paper.paper_id, "d2@example.com");
    auto i3 = referee_service.AssignReferee(paper.paper_id, "d3@example.com");
    referee_controller.AcceptInvitation(i1.assignment_id);
    referee_controller.AcceptInvitation(i2.assignment_id);
    referee_controller.AcceptInvitation(i3.assignment_id);
    review_controller.SubmitReview({paper.paper_id, "d1@example.com", "r1", "accept"});
    review_controller.SubmitReview({paper.paper_id, "d2@example.com", "r2", "accept"});
    review_controller.SubmitReview({paper.paper_id, "d3@example.com", "r3", "accept"});

    // 201 created
    auto created = decision_controller.RecordDecision({paper.paper_id, "accept"});
    Expect(created.status == 201, "expected contract 201 for valid decision");

    // 400 validation/insufficient
    auto bad = decision_controller.RecordDecision({9999, ""});
    Expect(bad.status == 400, "expected contract 400 for invalid decision request");

    // 409 duplicate
    auto dup = decision_controller.RecordDecision({paper.paper_id, "reject"});
    Expect(dup.status == 409, "expected contract 409 for duplicate decision");

    std::cout << "decision contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
