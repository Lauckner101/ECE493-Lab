#include <iostream>

#include "api/referee_controller.hpp"
#include "services/paper_service.hpp"
#include "services/referee_service.hpp"
#include "test_assert.hpp"

int main() {
  try {
    cms::services::PaperService papers;
    cms::services::RefereeService referee_service(&papers);
    cms::api::RefereeController referee_controller(&referee_service);

    auto paper = papers.SubmitPaper(1, {"Integration Paper", "Integration Abstract", "k", "int.pdf", 2048});
    Expect(paper.status == cms::services::SubmitPaperStatus::kSubmitted,
           "precondition failed: paper submission failed in integration flow");

    auto r1 = referee_controller.AssignReferee(paper.paper_id, "r1@integration.com");
    auto r2 = referee_controller.AssignReferee(paper.paper_id, "r2@integration.com");
    auto r3 = referee_controller.AssignReferee(paper.paper_id, "r3@integration.com");

    Expect(r1.status == 201 && r2.status == 201 && r3.status == 201,
           "expected three referee assignments in integration flow");
    Expect(referee_service.IsPaperFullyAssigned(paper.paper_id),
           "expected paper to be fully assigned in integration flow");
    Expect(referee_service.InvitationCount() == 3,
           "expected one invitation per successful assignment");

    std::cout << "referee assignment integration test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
