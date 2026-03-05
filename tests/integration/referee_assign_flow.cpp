#include <iostream>

#include "api/referee_controller.hpp"
#include "services/paper_service.hpp"
#include "services/referee_service.hpp"
#include "test_assert.hpp"

int main() {
  cms::services::PaperService papers;
  cms::services::RefereeService referee_service(&papers);
  cms::api::RefereeController referee_controller(&referee_service);

  auto paper = papers.SubmitPaper(1, {"Integration Paper", "Integration Abstract", "k", "int.pdf", 2048});
  Expect(paper.status == cms::services::SubmitPaperStatus::kSubmitted,
         "precondition failed: paper submission failed in integration flow");

  auto invalid_email = referee_controller.AssignReferee(paper.paper_id, "not-an-email");
  Expect(invalid_email.status == 400, "expected invalid email validation");

  auto first = referee_service.AssignReferee(paper.paper_id, "r1@integration.com");
  auto second = referee_service.AssignReferee(paper.paper_id, "r2@integration.com");
  auto third = referee_service.AssignReferee(paper.paper_id, "r3@integration.com");
  Expect(first.status == cms::services::AssignRefereeStatus::kAssigned &&
             second.status == cms::services::AssignRefereeStatus::kAssigned &&
             third.status == cms::services::AssignRefereeStatus::kAssigned,
         "expected three referee assignments in integration flow");

  auto controller_success = referee_controller.AssignReferee(
      papers.SubmitPaper(2, {"P2", "A2", "k", "p2.pdf", 1024}).paper_id, "p2r1@integration.com");
  Expect(controller_success.status == 201, "expected successful assignment from controller");

  auto duplicate = referee_controller.AssignReferee(paper.paper_id, "R1@integration.com");
  Expect(duplicate.status == 409, "expected duplicate assignment conflict");

  auto over_capacity = referee_controller.AssignReferee(paper.paper_id, "r4@integration.com");
  Expect(over_capacity.status == 409, "expected fully assigned conflict");

  auto bad_invite = referee_controller.AcceptInvitation(9999);
  Expect(bad_invite.status == 400, "expected invalid invitation rejection");

  auto accepted = referee_controller.AcceptInvitation(first.assignment_id);
  Expect(accepted.status == 200, "expected invitation acceptance");

  auto already_accepted = referee_controller.AcceptInvitation(first.assignment_id);
  Expect(already_accepted.status == 200, "expected idempotent accept response");

  auto reject_accepted = referee_controller.RejectInvitation(first.assignment_id);
  Expect(reject_accepted.status == 409, "expected conflict when rejecting accepted invitation");

  auto rejected = referee_controller.RejectInvitation(second.assignment_id);
  Expect(rejected.status == 200, "expected invitation rejection");

  auto already_rejected = referee_controller.RejectInvitation(second.assignment_id);
  Expect(already_rejected.status == 200, "expected idempotent reject response");

  auto paper_fully_assigned = referee_controller.ExitAssignment(paper.paper_id);
  Expect(paper_fully_assigned.status == 200 &&
             paper_fully_assigned.body.find("fully assigned") != std::string::npos,
         "expected fully assigned marker for paper with three assignments");

  auto paper_incomplete = referee_controller.ExitAssignment(2);
  Expect(paper_incomplete.status == 200 &&
             paper_incomplete.body.find("incompletely") != std::string::npos,
         "expected incomplete assignment marker for partially assigned paper");

  Expect(referee_service.InvitationCount() == 4,
         "expected one invitation per successful assignment");

  std::cout << "referee assignment integration test passed\n";
  return 0;
}
