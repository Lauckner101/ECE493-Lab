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

    auto paper = papers.SubmitPaper(1, {"Reject Contract", "A", "k", "r.pdf", 1024});
    auto invite = service.AssignReferee(paper.paper_id, "reject-contract@example.com");

    auto ok = controller.RejectInvitation(invite.assignment_id);
    Expect(ok.status == 200, "expected contract 200 on invitation reject success");

    std::cout << "invitation reject contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
