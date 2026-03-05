#include <iostream>

#include "api/referee_controller.hpp"
#include "api/review_controller.hpp"
#include "services/paper_service.hpp"
#include "services/referee_service.hpp"
#include "services/review_service.hpp"
#include "test_assert.hpp"

int main() {
  cms::services::PaperService papers;
  cms::services::RefereeService referee_service(&papers);
  cms::services::ReviewService review_service(&referee_service);

  cms::api::RefereeController referee_controller(&referee_service);
  cms::api::ReviewController review_controller(&review_service);

  auto paper = papers.SubmitPaper(1, {"Integration Review", "Abstract", "k", "ir.pdf", 2048});
  Expect(paper.status == cms::services::SubmitPaperStatus::kSubmitted,
         "precondition failed: paper submit");

  auto missing_fields = review_controller.SubmitReview({0, "", "", ""});
  Expect(missing_fields.status == 400, "expected missing-field rejection");

  auto invite = referee_service.AssignReferee(paper.paper_id, "int-review@example.com");
  Expect(invite.status == cms::services::AssignRefereeStatus::kAssigned,
         "precondition failed: assign referee");

  auto before_accept = review_controller.SubmitReview(
      {paper.paper_id, "int-review@example.com", "Comprehensive review", "accept"});
  Expect(before_accept.status == 409, "expected not-assigned conflict before invitation acceptance");

  auto accepted = referee_controller.AcceptInvitation(invite.assignment_id);
  Expect(accepted.status == 200, "precondition failed: accept invitation");

  auto invalid_data = review_controller.SubmitReview(
      {paper.paper_id, "int-review@example.com", "Comprehensive @@ review", "accept"});
  Expect(invalid_data.status == 400, "expected invalid review payload rejection");

  auto submitted = review_controller.SubmitReview(
      {paper.paper_id, "int-review@example.com", "Comprehensive review", "accept"});
  Expect(submitted.status == 201, "expected successful review submission in integration flow");

  auto duplicate = review_controller.SubmitReview(
      {paper.paper_id, "INT-REVIEW@example.com", "Another review", "accept"});
  Expect(duplicate.status == 409, "expected duplicate review conflict");

  Expect(review_service.CountReviewsForPaper(paper.paper_id) == 1,
         "expected exactly one review stored");
  Expect(review_service.HasReviewForPaperByReferee(paper.paper_id, "int-review@example.com"),
         "expected review visibility by paper/referee");

  std::cout << "review submit integration test passed\n";
  return 0;
}
