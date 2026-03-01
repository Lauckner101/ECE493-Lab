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

    auto paper = papers.SubmitPaper(1, {"Review Paper", "Abstract", "k", "review.pdf", 2048});
    auto invite = referee_service.AssignReferee(paper.paper_id, "reviewer@example.com");
    auto accept = referee_controller.AcceptInvitation(invite.assignment_id);
    Expect(accept.status == 200, "precondition failed: invitation acceptance");

    // AT-UC08-001: successful review.
    auto ok = review_controller.SubmitReview(
        {paper.paper_id, "reviewer@example.com", "Solid contribution.", "accept"});
    Expect(ok.status == 201, "AT-UC08-001: expected successful submit");
    Expect(review_service.EditorNotificationCount() == 1,
           "AT-UC08-001: editor notification expected");

    // AT-UC08-002: missing fields.
    auto missing = review_controller.SubmitReview(
        {paper.paper_id, "reviewer@example.com", "", "accept"});
    Expect(missing.status == 400, "AT-UC08-002: expected missing-fields block");

    // AT-UC08-003: invalid data.
    auto invalid = review_controller.SubmitReview(
        {paper.paper_id, "reviewer@example.com", "Bad @@ chars", "maybe"});
    Expect(invalid.status == 400, "AT-UC08-003: expected invalid-data block");

    // AT-UC08-004: system error during submit.
    auto sys = review_controller.SubmitReview(
        {paper.paper_id, "reviewer@example.com", "Retry later", "reject", true});
    Expect(sys.status == 500, "AT-UC08-004: expected system-error response");

    // AT-UC08-005: prevent duplicate review.
    auto dup = review_controller.SubmitReview(
        {paper.paper_id, "reviewer@example.com", "Second review attempt", "reject"});
    Expect(dup.status == 409, "AT-UC08-005: expected duplicate block");

    // AT-UC08-006: visible to editor.
    auto reviews = review_service.GetReviewsForPaper(paper.paper_id);
    Expect(!reviews.empty(), "AT-UC08-006: expected editor-visible review list");

    // AT-UC08-007: retry after failure.
    auto paper2 = papers.SubmitPaper(1, {"Retry Paper", "Abstract", "k", "retry.pdf", 2048});
    auto invite2 = referee_service.AssignReferee(paper2.paper_id, "reviewer2@example.com");
    referee_controller.AcceptInvitation(invite2.assignment_id);
    auto failed = review_controller.SubmitReview(
        {paper2.paper_id, "reviewer2@example.com", "Transient fail", "accept", true});
    auto retried = review_controller.SubmitReview(
        {paper2.paper_id, "reviewer2@example.com", "Recovered submit", "accept"});
    Expect(failed.status == 500 && retried.status == 201,
           "AT-UC08-007: retry should succeed after failure");

    std::cout << "UC-08 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
