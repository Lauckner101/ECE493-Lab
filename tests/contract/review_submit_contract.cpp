#include <iostream>

#include "api/referee_controller.hpp"
#include "api/review_controller.hpp"
#include "services/paper_service.hpp"
#include "services/referee_service.hpp"
#include "services/review_service.hpp"
#include "test_assert.hpp"

int main() {
  try {
    cms::services::PaperService papers;
    cms::services::RefereeService referee_service(&papers);
    cms::services::ReviewService review_service(&referee_service);

    cms::api::RefereeController referee_controller(&referee_service);
    cms::api::ReviewController review_controller(&review_service);

    auto paper = papers.SubmitPaper(1, {"Contract Review", "Abstract", "k", "cr.pdf", 2048});
    auto invite = referee_service.AssignReferee(paper.paper_id, "contract-review@example.com");
    referee_controller.AcceptInvitation(invite.assignment_id);

    // 201 created
    auto created = review_controller.SubmitReview(
        {paper.paper_id, "contract-review@example.com", "Good", "accept"});
    Expect(created.status == 201, "expected contract 201 on valid review submission");

    // 400 bad request
    auto bad = review_controller.SubmitReview(
        {paper.paper_id, "contract-review@example.com", "", "accept"});
    Expect(bad.status == 400, "expected contract 400 on invalid review payload");

    // 409 duplicate
    auto dup = review_controller.SubmitReview(
        {paper.paper_id, "contract-review@example.com", "Again", "reject"});
    Expect(dup.status == 409, "expected contract 409 on duplicate review");

    std::cout << "review submit contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
