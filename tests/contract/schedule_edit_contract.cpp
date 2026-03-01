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
  auto submission =
      papers->SubmitPaper(1, {"UC11 Contract " + std::to_string(idx), "A", "k", "u11c" + std::to_string(idx) + ".pdf", 2048});
  cms::api::RefereeController referee_controller(referees);
  cms::api::ReviewController review_controller(reviews);
  cms::api::DecisionController decision_controller(decisions);
  auto i1 = referees->AssignReferee(submission.paper_id, "c1_" + std::to_string(idx) + "@example.com");
  auto i2 = referees->AssignReferee(submission.paper_id, "c2_" + std::to_string(idx) + "@example.com");
  auto i3 = referees->AssignReferee(submission.paper_id, "c3_" + std::to_string(idx) + "@example.com");
  referee_controller.AcceptInvitation(i1.assignment_id);
  referee_controller.AcceptInvitation(i2.assignment_id);
  referee_controller.AcceptInvitation(i3.assignment_id);
  review_controller.SubmitReview({submission.paper_id, "c1_" + std::to_string(idx) + "@example.com", "r1", "accept"});
  review_controller.SubmitReview({submission.paper_id, "c2_" + std::to_string(idx) + "@example.com", "r2", "accept"});
  review_controller.SubmitReview({submission.paper_id, "c3_" + std::to_string(idx) + "@example.com", "r3", "accept"});
  Expect(decision_controller.RecordDecision({submission.paper_id, "accept"}).status == 201,
         "seed failed");
}
}  // namespace

int main() {
  try {
    cms::services::PaperService papers;
    cms::services::RefereeService referees(&papers);
    cms::services::ReviewService reviews(&referees);
    cms::services::DecisionService decisions(&papers, &reviews);
    cms::services::ScheduleService schedules(&papers);
    cms::api::ScheduleController controller(&schedules);

    SeedAcceptedPaper(&papers, &referees, &reviews, &decisions, 1);
    SeedAcceptedPaper(&papers, &referees, &reviews, &decisions, 2);
    Expect(controller.GenerateSchedule({2, 2}).status == 201, "setup failed");

    // 200 on successful edit
    auto base_items = schedules.GetCurrentSchedule()->items;
    base_items[0].room = "Edited Room";
    auto ok = controller.EditCurrentSchedule({base_items});
    Expect(ok.status == 200, "expected contract 200 for valid edit");

    // 400 on no changes
    auto no_change = controller.EditCurrentSchedule({schedules.GetCurrentSchedule()->items});
    Expect(no_change.status == 400, "expected contract 400 for unchanged schedule");

    // 409 on conflicts
    auto conflicts = schedules.GetCurrentSchedule()->items;
    conflicts[0].room = "R";
    conflicts[0].time_slot = "T";
    conflicts[1].room = "R";
    conflicts[1].time_slot = "T";
    auto conflict = controller.EditCurrentSchedule({conflicts});
    Expect(conflict.status == 409, "expected contract 409 for conflicting schedule");

    // 500 on save failure
    auto valid_change = schedules.GetCurrentSchedule()->items;
    valid_change[0].time_slot = "New Slot";
    auto failure = controller.EditCurrentSchedule({valid_change, true});
    Expect(failure.status == 500, "expected contract 500 for save failure");

    // 401 on unauthorized edit
    auto unauthorized = controller.EditCurrentSchedule({schedules.GetCurrentSchedule()->items}, false);
    Expect(unauthorized.status == 401, "expected contract 401 for unauthorized edit");

    std::cout << "schedule edit contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
