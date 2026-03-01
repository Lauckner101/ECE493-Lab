#include <iostream>
#include <string>
#include <vector>

#include "api/referee_controller.hpp"
#include "services/paper_service.hpp"
#include "services/referee_service.hpp"
#include "test_assert.hpp"

namespace {
int CreateSubmittedPaper(cms::services::PaperService* papers, int author_id, const std::string& suffix) {
  auto res = papers->SubmitPaper(author_id,
                                 {"Paper " + suffix, "Abstract " + suffix, "k", "paper" + suffix + ".pdf", 2048});
  return res.paper_id;
}
}  // namespace

int main() {
  try {
    cms::services::PaperService papers;
    cms::services::RefereeService referee_service(&papers);
    cms::api::RefereeController referee_controller(&referee_service);

    const int author_id = 1;

    // AT-UC06-001: assign three referees successfully.
    int paper1 = CreateSubmittedPaper(&papers, author_id, "1");
    auto r1 = referee_controller.AssignReferee(paper1, "r1@example.com");
    auto r2 = referee_controller.AssignReferee(paper1, "r2@example.com");
    auto r3 = referee_controller.AssignReferee(paper1, "r3@example.com");
    Expect(r1.status == 201 && r2.status == 201 && r3.status == 201,
           "AT-UC06-001: expected three successful assignments");
    Expect(referee_service.IsPaperFullyAssigned(paper1),
           "AT-UC06-001: paper should be fully assigned after three referees");
    Expect(referee_service.InvitationCount() >= 3,
           "AT-UC06-001: invitations should be sent for successful assignments");

    // AT-UC06-002: workload limit exceeded.
    const std::string overloaded = "busy@example.com";
    for (int i = 0; i < 5; ++i) {
      int pid = CreateSubmittedPaper(&papers, author_id, "w" + std::to_string(i));
      auto ok = referee_controller.AssignReferee(pid, overloaded);
      Expect(ok.status == 201, "AT-UC06-002: preload workload should succeed");
    }
    int paper2 = CreateSubmittedPaper(&papers, author_id, "2");
    auto overload_try = referee_controller.AssignReferee(paper2, overloaded);
    Expect(overload_try.status == 409, "AT-UC06-002: expected workload limit block");

    // AT-UC06-003: duplicate assignment.
    int paper3 = CreateSubmittedPaper(&papers, author_id, "3");
    auto first = referee_controller.AssignReferee(paper3, "dup@example.com");
    auto dup = referee_controller.AssignReferee(paper3, "dup@example.com");
    Expect(first.status == 201, "AT-UC06-003: first assignment should succeed");
    Expect(dup.status == 409, "AT-UC06-003: duplicate assignment should be blocked");

    // AT-UC06-004: fewer than three assigned marked incomplete on exit.
    int paper4 = CreateSubmittedPaper(&papers, author_id, "4");
    auto partial = referee_controller.AssignReferee(paper4, "partial@example.com");
    Expect(partial.status == 201, "AT-UC06-004: partial assignment setup failed");
    auto exit = referee_controller.ExitAssignment(paper4);
    Expect(exit.status == 200 && referee_service.IsPaperIncomplete(paper4),
           "AT-UC06-004: paper should be marked incomplete");

    // AT-UC06-005: workload increment.
    int paper5 = CreateSubmittedPaper(&papers, author_id, "5");
    int before = referee_service.GetRefereeWorkload("inc@example.com");
    auto inc = referee_controller.AssignReferee(paper5, "inc@example.com");
    int after = referee_service.GetRefereeWorkload("inc@example.com");
    Expect(inc.status == 201 && after == before + 1,
           "AT-UC06-005: workload should increment by one");

    // AT-UC06-006: invitation sent on assignment.
    int paper6 = CreateSubmittedPaper(&papers, author_id, "6");
    int invites_before = referee_service.InvitationCount();
    auto inv = referee_controller.AssignReferee(paper6, "invite@example.com");
    int invites_after = referee_service.InvitationCount();
    Expect(inv.status == 201 && invites_after == invites_before + 1,
           "AT-UC06-006: invitation count should increase");

    // AT-UC06-007: prevent assignment beyond three.
    int paper7 = CreateSubmittedPaper(&papers, author_id, "7");
    Expect(referee_controller.AssignReferee(paper7, "a1@example.com").status == 201,
           "AT-UC06-007: setup assignment 1 failed");
    Expect(referee_controller.AssignReferee(paper7, "a2@example.com").status == 201,
           "AT-UC06-007: setup assignment 2 failed");
    int batch_assigned = referee_service.AssignRefereesBatch(
        paper7, std::vector<std::string>{"a3@example.com", "a4@example.com"});
    Expect(batch_assigned == 1, "AT-UC06-007: expected only one additional referee assigned");
    Expect(referee_service.CountAssignmentsForPaper(paper7) == 3,
           "AT-UC06-007: total assignments must not exceed three");

    std::cout << "UC-06 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
