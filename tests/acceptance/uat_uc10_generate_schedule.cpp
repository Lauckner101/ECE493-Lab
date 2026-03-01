#include <iostream>

#include "api/decision_controller.hpp"
#include "api/referee_controller.hpp"
#include "api/review_controller.hpp"
#include "api/schedule_controller.hpp"
#include "models/paper_submission.hpp"
#include "services/decision_service.hpp"
#include "services/paper_service.hpp"
#include "services/referee_service.hpp"
#include "services/review_service.hpp"
#include "services/schedule_service.hpp"
#include "test_assert.hpp"

namespace {
void AcceptPaper(cms::services::PaperService* papers, cms::services::RefereeService* referees,
                 cms::services::ReviewService* reviews, cms::services::DecisionService* decisions,
                 int author_id, const std::string& title, int tag) {
  auto paper =
      papers->SubmitPaper(author_id, {title, "Abstract", "k", "paper" + std::to_string(tag) + ".pdf", 2048});

  auto i1 = referees->AssignReferee(paper.paper_id, "r1_" + std::to_string(tag) + "@example.com");
  auto i2 = referees->AssignReferee(paper.paper_id, "r2_" + std::to_string(tag) + "@example.com");
  auto i3 = referees->AssignReferee(paper.paper_id, "r3_" + std::to_string(tag) + "@example.com");

  cms::api::RefereeController referee_controller(referees);
  cms::api::ReviewController review_controller(reviews);
  cms::api::DecisionController decision_controller(decisions);

  Expect(referee_controller.AcceptInvitation(i1.assignment_id).status == 200, "setup: invitation accept failed");
  Expect(referee_controller.AcceptInvitation(i2.assignment_id).status == 200, "setup: invitation accept failed");
  Expect(referee_controller.AcceptInvitation(i3.assignment_id).status == 200, "setup: invitation accept failed");

  Expect(review_controller
                 .SubmitReview({paper.paper_id, "r1_" + std::to_string(tag) + "@example.com", "R1", "accept"})
                 .status == 201,
         "setup: review failed");
  Expect(review_controller
                 .SubmitReview({paper.paper_id, "r2_" + std::to_string(tag) + "@example.com", "R2", "accept"})
                 .status == 201,
         "setup: review failed");
  Expect(review_controller
                 .SubmitReview({paper.paper_id, "r3_" + std::to_string(tag) + "@example.com", "R3", "accept"})
                 .status == 201,
         "setup: review failed");

  auto decision = decision_controller.RecordDecision({paper.paper_id, "accept"});
  Expect(decision.status == 201, "setup: decision failed");
}
}  // namespace

int main() {
  try {
    cms::services::PaperService papers;
    cms::services::RefereeService referees(&papers);
    cms::services::ReviewService reviews(&referees);
    cms::services::DecisionService decisions(&papers, &reviews);
    cms::services::ScheduleService schedules(&papers);
    cms::api::ScheduleController schedule_controller(&schedules);

    // AT-UC10-001: Generate schedule successfully.
    AcceptPaper(&papers, &referees, &reviews, &decisions, 1, "Accepted A", 101);
    AcceptPaper(&papers, &referees, &reviews, &decisions, 1, "Accepted B", 102);
    auto generated = schedule_controller.GenerateSchedule({2, 2});
    Expect(generated.status == 201, "AT-UC10-001: expected schedule generation success");
    Expect(generated.body.find("<table>") != std::string::npos,
           "AT-UC10-001: expected HTML schedule output");

    // AT-UC10-002: View persisted schedule.
    auto persisted = schedule_controller.ViewCurrentSchedule();
    Expect(persisted.status == 200, "AT-UC10-002: expected persisted schedule view");
    Expect(persisted.body == generated.body, "AT-UC10-002: persisted schedule should be unchanged");

    // AT-UC10-003: No accepted papers.
    cms::services::PaperService papers_none;
    cms::services::ScheduleService schedules_none(&papers_none);
    cms::api::ScheduleController schedule_none(&schedules_none);
    auto no_papers = schedule_none.GenerateSchedule({2, 2});
    Expect(no_papers.status == 400, "AT-UC10-003: generation should be blocked");

    // AT-UC10-004: Unsatisfiable constraints.
    cms::services::PaperService papers_conflict;
    cms::services::RefereeService referees_conflict(&papers_conflict);
    cms::services::ReviewService reviews_conflict(&referees_conflict);
    cms::services::DecisionService decisions_conflict(&papers_conflict, &reviews_conflict);
    cms::services::ScheduleService schedules_conflict(&papers_conflict);
    cms::api::ScheduleController schedule_conflict(&schedules_conflict);
    AcceptPaper(&papers_conflict, &referees_conflict, &reviews_conflict, &decisions_conflict, 2, "C1", 201);
    AcceptPaper(&papers_conflict, &referees_conflict, &reviews_conflict, &decisions_conflict, 2, "C2", 202);
    AcceptPaper(&papers_conflict, &referees_conflict, &reviews_conflict, &decisions_conflict, 2, "C3", 203);
    auto conflict = schedule_conflict.GenerateSchedule({1, 2});
    Expect(conflict.status == 409, "AT-UC10-004: expected unsatisfied constraint conflict");

    // AT-UC10-005: System error while saving.
    auto save_error = schedule_controller.GenerateSchedule({2, 2, true});
    Expect(save_error.status == 500, "AT-UC10-005: expected save failure response");

    // AT-UC10-006: Deterministic scheduling behavior.
    auto first = schedule_controller.GenerateSchedule({2, 2});
    auto second = schedule_controller.GenerateSchedule({2, 2});
    Expect(first.status == 201 && second.status == 201, "AT-UC10-006: expected both generations to succeed");
    Expect(first.body == second.body, "AT-UC10-006: expected deterministic output for unchanged inputs");

    // AT-UC10-007: HTML output validation.
    Expect(first.body.find("<tr>") != std::string::npos, "AT-UC10-007: HTML should contain table rows");
    Expect(first.body.find("</table>") != std::string::npos,
           "AT-UC10-007: HTML should contain table closing tag");

    std::cout << "UC-10 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
