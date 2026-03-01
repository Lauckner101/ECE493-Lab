#include <iostream>

#include "api/decision_controller.hpp"
#include "api/referee_controller.hpp"
#include "api/review_controller.hpp"
#include "api/schedule_controller.hpp"
#include "services/decision_service.hpp"
#include "services/paper_service.hpp"
#include "services/referee_service.hpp"
#include "services/review_service.hpp"
#include "services/schedule_service.hpp"
#include "test_assert.hpp"

namespace {
void SeedAcceptedPaper(cms::services::PaperService* papers, cms::services::RefereeService* referees,
                       cms::services::ReviewService* reviews, cms::services::DecisionService* decisions,
                       int idx) {
  auto submitted = papers->SubmitPaper(3, {"Contract Seed " + std::to_string(idx), "A", "k",
                                           "contract_seed_" + std::to_string(idx) + ".pdf", 4096});

  cms::api::RefereeController referee_controller(referees);
  cms::api::ReviewController review_controller(reviews);
  cms::api::DecisionController decision_controller(decisions);

  auto i1 = referees->AssignReferee(submitted.paper_id, "cs1_" + std::to_string(idx) + "@example.com");
  auto i2 = referees->AssignReferee(submitted.paper_id, "cs2_" + std::to_string(idx) + "@example.com");
  auto i3 = referees->AssignReferee(submitted.paper_id, "cs3_" + std::to_string(idx) + "@example.com");
  referee_controller.AcceptInvitation(i1.assignment_id);
  referee_controller.AcceptInvitation(i2.assignment_id);
  referee_controller.AcceptInvitation(i3.assignment_id);

  review_controller.SubmitReview(
      {submitted.paper_id, "cs1_" + std::to_string(idx) + "@example.com", "r1", "accept"});
  review_controller.SubmitReview(
      {submitted.paper_id, "cs2_" + std::to_string(idx) + "@example.com", "r2", "accept"});
  review_controller.SubmitReview(
      {submitted.paper_id, "cs3_" + std::to_string(idx) + "@example.com", "r3", "accept"});

  auto decision = decision_controller.RecordDecision({submitted.paper_id, "accept"});
  Expect(decision.status == 201, "seed failed: expected accepted decision");
}
}  // namespace

int main() {
  try {
    // 201 created for valid request
    cms::services::PaperService papers;
    cms::services::RefereeService referees(&papers);
    cms::services::ReviewService reviews(&referees);
    cms::services::DecisionService decisions(&papers, &reviews);
    cms::services::ScheduleService schedule_service(&papers);
    cms::api::ScheduleController schedule_controller(&schedule_service);

    SeedAcceptedPaper(&papers, &referees, &reviews, &decisions, 1);
    auto created = schedule_controller.GenerateSchedule({2, 2});
    Expect(created.status == 201, "expected contract 201 for valid schedule request");

    // 400 when there are no accepted papers
    cms::services::PaperService empty_papers;
    cms::services::ScheduleService empty_schedule_service(&empty_papers);
    cms::api::ScheduleController empty_controller(&empty_schedule_service);
    auto bad = empty_controller.GenerateSchedule({2, 2});
    Expect(bad.status == 400, "expected contract 400 when no accepted papers exist");

    // 409 when constraints cannot be satisfied
    SeedAcceptedPaper(&papers, &referees, &reviews, &decisions, 2);
    SeedAcceptedPaper(&papers, &referees, &reviews, &decisions, 3);
    auto conflict = schedule_controller.GenerateSchedule({1, 2});
    Expect(conflict.status == 409, "expected contract 409 for unsatisfiable constraints");

    // 500 on save failure
    auto failure = schedule_controller.GenerateSchedule({2, 2, true});
    Expect(failure.status == 500, "expected contract 500 for save failure");

    std::cout << "schedule generate contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
