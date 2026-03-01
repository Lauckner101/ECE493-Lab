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

    auto paper = papers.SubmitPaper(1, {"Contract Paper", "Contract Abstract", "k", "contract.pdf", 4096});
    Expect(paper.status == cms::services::SubmitPaperStatus::kSubmitted,
           "precondition failed: submitted paper missing");

    // 201 assigned
    auto created = referee_controller.AssignReferee(paper.paper_id, "contract-ref@example.com");
    Expect(created.status == 201, "expected contract 201 for valid referee assignment");

    // 400 bad request
    auto invalid = referee_controller.AssignReferee(paper.paper_id, "not-an-email");
    Expect(invalid.status == 400, "expected contract 400 for invalid referee email");

    // 409 conflict (duplicate)
    auto duplicate = referee_controller.AssignReferee(paper.paper_id, "contract-ref@example.com");
    Expect(duplicate.status == 409, "expected contract 409 for duplicate assignment");

    std::cout << "referee assign contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
