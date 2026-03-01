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
void AcceptPaper(cms::services::PaperService* papers, cms::services::RefereeService* referees,
                 cms::services::ReviewService* reviews, cms::services::DecisionService* decisions,
                 int tag) {
  auto submission = papers->SubmitPaper(
      1, {"UC11 Paper " + std::to_string(tag), "Abstract", "k", "uc11_" + std::to_string(tag) + ".pdf", 4096});
  Expect(submission.status == cms::services::SubmitPaperStatus::kSubmitted, "setup: submit failed");

  cms::api::RefereeController referee_controller(referees);
  cms::api::ReviewController review_controller(reviews);
  cms::api::DecisionController decision_controller(decisions);

  auto i1 = referees->AssignReferee(submission.paper_id, "u11r1_" + std::to_string(tag) + "@example.com");
  auto i2 = referees->AssignReferee(submission.paper_id, "u11r2_" + std::to_string(tag) + "@example.com");
  auto i3 = referees->AssignReferee(submission.paper_id, "u11r3_" + std::to_string(tag) + "@example.com");
  Expect(referee_controller.AcceptInvitation(i1.assignment_id).status == 200, "setup: invitation accept failed");
  Expect(referee_controller.AcceptInvitation(i2.assignment_id).status == 200, "setup: invitation accept failed");
  Expect(referee_controller.AcceptInvitation(i3.assignment_id).status == 200, "setup: invitation accept failed");
  Expect(review_controller
                 .SubmitReview(
                     {submission.paper_id, "u11r1_" + std::to_string(tag) + "@example.com", "r1", "accept"})
                 .status == 201,
         "setup: review failed");
  Expect(review_controller
                 .SubmitReview(
                     {submission.paper_id, "u11r2_" + std::to_string(tag) + "@example.com", "r2", "accept"})
                 .status == 201,
         "setup: review failed");
  Expect(review_controller
                 .SubmitReview(
                     {submission.paper_id, "u11r3_" + std::to_string(tag) + "@example.com", "r3", "accept"})
                 .status == 201,
         "setup: review failed");
  Expect(decision_controller.RecordDecision({submission.paper_id, "accept"}).status == 201,
         "setup: decision failed");
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

    AcceptPaper(&papers, &referees, &reviews, &decisions, 1);
    AcceptPaper(&papers, &referees, &reviews, &decisions, 2);
    auto generated = schedule_controller.GenerateSchedule({2, 2});
    Expect(generated.status == 201, "setup: schedule generation failed");

    auto initial = schedules.GetCurrentSchedule();
    Expect(initial.has_value(), "setup: expected current schedule");

    // AT-UC11-001: edit schedule successfully.
    auto edit_items = initial->items;
    edit_items[0].room = "Room X";
    auto edited = schedule_controller.EditCurrentSchedule({edit_items});
    Expect(edited.status == 200, "AT-UC11-001: expected schedule edit success");

    // AT-UC11-002: conflict detected during edit.
    auto conflict_items = schedules.GetCurrentSchedule()->items;
    conflict_items[0].room = "Conflict Room";
    conflict_items[0].time_slot = "Conflict Slot";
    conflict_items[1].room = "Conflict Room";
    conflict_items[1].time_slot = "Conflict Slot";
    auto conflict = schedule_controller.EditCurrentSchedule({conflict_items});
    Expect(conflict.status == 409, "AT-UC11-002: expected conflict detection");

    // AT-UC11-003: no changes made.
    auto no_changes_items = schedules.GetCurrentSchedule()->items;
    auto no_changes = schedule_controller.EditCurrentSchedule({no_changes_items});
    Expect(no_changes.status == 400, "AT-UC11-003: expected no-change validation");

    // AT-UC11-004: system error while saving.
    auto system_error_items = schedules.GetCurrentSchedule()->items;
    system_error_items[0].time_slot = "Slot Z";
    auto system_error = schedule_controller.EditCurrentSchedule({system_error_items, true});
    Expect(system_error.status == 500, "AT-UC11-004: expected save failure");

    // AT-UC11-005: persistence of edited schedule.
    auto persisted = schedule_controller.ViewCurrentSchedule();
    Expect(persisted.status == 200, "AT-UC11-005: expected persisted schedule view");
    Expect(persisted.body.find("Room X") != std::string::npos,
           "AT-UC11-005: edited schedule should persist");

    // AT-UC11-006: replace previous schedule version.
    auto second_edit = schedules.GetCurrentSchedule()->items;
    second_edit[0].room = "Room Y";
    auto before_second_id = schedules.GetCurrentSchedule()->id;
    auto second_result = schedule_controller.EditCurrentSchedule({second_edit});
    Expect(second_result.status == 200, "AT-UC11-006: expected second edit success");
    auto after_second = schedules.GetCurrentSchedule();
    Expect(after_second->id != before_second_id, "AT-UC11-006: expected new version id");
    Expect(after_second->html.find("Room Y") != std::string::npos,
           "AT-UC11-006: latest schedule version should be current");

    // AT-UC11-007: unauthorized user cannot edit schedule.
    auto unauthorized = schedule_controller.EditCurrentSchedule({after_second->items}, false);
    Expect(unauthorized.status == 401, "AT-UC11-007: expected unauthorized response");

    std::cout << "UC-11 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
