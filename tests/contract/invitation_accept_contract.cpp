#include <iostream>

#include "api/referee_controller.hpp"
#include "services/paper_service.hpp"
#include "services/referee_service.hpp"
#include "test_assert.hpp"

int main() {
  try {
    cms::services::PaperService papers;
    cms::services::RefereeService service(&papers);
    cms::api::RefereeController controller(&service);

    auto paper = papers.SubmitPaper(1, {"Accept Contract", "A", "k", "a.pdf", 1024});
    auto invite = service.AssignReferee(paper.paper_id, "accept-contract@example.com");

    // 200 success
    auto ok = controller.AcceptInvitation(invite.assignment_id);
    Expect(ok.status == 200, "expected contract 200 on invitation accept success");

    // 409 conflict
    int p2 = papers.SubmitPaper(1, {"Full", "A", "k", "f.pdf", 1024}).paper_id;
    auto i1 = service.AssignReferee(p2, "x1@example.com");
    auto i2 = service.AssignReferee(p2, "x2@example.com");
    auto i3 = service.AssignReferee(p2, "x3@example.com");
    controller.AcceptInvitation(i1.assignment_id);
    controller.AcceptInvitation(i2.assignment_id);
    controller.AcceptInvitation(i3.assignment_id);
    int overflow = service.CreateInvitationForTesting(p2, "x4@example.com");
    auto conflict = controller.AcceptInvitation(overflow);
    Expect(conflict.status == 409, "expected contract 409 on acceptance conflict");

    std::cout << "invitation accept contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
