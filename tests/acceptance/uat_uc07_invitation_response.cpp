#include <iostream>

#include "api/referee_controller.hpp"
#include "services/paper_service.hpp"
#include "services/referee_service.hpp"
#include "test_assert.hpp"

namespace {
int CreateSubmittedPaper(cms::services::PaperService* papers, int idx) {
  auto r = papers->SubmitPaper(1, {"Paper " + std::to_string(idx), "Abstract", "k",
                                   "p" + std::to_string(idx) + ".pdf", 2048});
  return r.paper_id;
}
}  // namespace

int main() {
  try {
    cms::services::PaperService papers;
    cms::services::RefereeService service(&papers);
    cms::api::RefereeController controller(&service);

    // AT-UC07-001: accept invitation successfully.
    int paper1 = CreateSubmittedPaper(&papers, 1);
    auto invite1 = service.AssignReferee(paper1, "acc@example.com");
    auto acc = controller.AcceptInvitation(invite1.assignment_id);
    Expect(acc.status == 200, "AT-UC07-001: expected accept success");
    Expect(service.IsPaperAssignedToReferee(paper1, "acc@example.com"),
           "AT-UC07-001: paper should be assigned to referee");

    // AT-UC07-002: reject invitation.
    int paper2 = CreateSubmittedPaper(&papers, 2);
    auto invite2 = service.AssignReferee(paper2, "rej@example.com");
    auto rej = controller.RejectInvitation(invite2.assignment_id);
    Expect(rej.status == 200, "AT-UC07-002: expected reject success");
    Expect(!service.IsPaperAssignedToReferee(paper2, "rej@example.com"),
           "AT-UC07-002: rejected referee must not be assigned");

    // AT-UC07-003: paper already has three referees.
    int paper3 = CreateSubmittedPaper(&papers, 3);
    auto i31 = service.AssignReferee(paper3, "c1@example.com");
    auto i32 = service.AssignReferee(paper3, "c2@example.com");
    auto i33 = service.AssignReferee(paper3, "c3@example.com");
    controller.AcceptInvitation(i31.assignment_id);
    controller.AcceptInvitation(i32.assignment_id);
    controller.AcceptInvitation(i33.assignment_id);
    int overflow_inv = service.CreateInvitationForTesting(paper3, "c4@example.com");
    auto cap = controller.AcceptInvitation(overflow_inv);
    Expect(cap.status == 409, "AT-UC07-003: expected capacity block");

    // AT-UC07-004: referee workload limit exceeded.
    int paper4 = CreateSubmittedPaper(&papers, 4);
    auto i4 = service.AssignReferee(paper4, "busy@example.com");
    service.SetRefereeWorkloadForTesting("busy@example.com", 5);
    auto busy = controller.AcceptInvitation(i4.assignment_id);
    Expect(busy.status == 409, "AT-UC07-004: expected workload limit block");

    // AT-UC07-005: invalid invitation.
    auto invalid = controller.AcceptInvitation(999999);
    Expect(invalid.status == 400, "AT-UC07-005: expected invalid invitation error");

    // AT-UC07-006: prevent duplicate acceptance.
    int paper6 = CreateSubmittedPaper(&papers, 6);
    auto i6 = service.AssignReferee(paper6, "dupacc@example.com");
    auto first_accept = controller.AcceptInvitation(i6.assignment_id);
    auto dup_accept = controller.AcceptInvitation(i6.assignment_id);
    Expect(first_accept.status == 200, "AT-UC07-006: first accept should succeed");
    Expect(dup_accept.status == 200, "AT-UC07-006: duplicate accept should be informational");
    Expect(service.CountAcceptedPapersForReferee("dupacc@example.com") == 1,
           "AT-UC07-006: duplicate acceptance must not duplicate assignment");

    // AT-UC07-007: paper visible in referee account after acceptance.
    int paper7 = CreateSubmittedPaper(&papers, 7);
    auto i7 = service.AssignReferee(paper7, "visible@example.com");
    controller.AcceptInvitation(i7.assignment_id);
    Expect(service.CountAcceptedPapersForReferee("visible@example.com") >= 1,
           "AT-UC07-007: accepted paper should be visible in referee account list");

    std::cout << "UC-07 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
