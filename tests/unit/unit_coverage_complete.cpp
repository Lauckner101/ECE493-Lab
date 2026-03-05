#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "api/auth_controller.hpp"
#include "api/decision_controller.hpp"
#include "api/http_errors.hpp"
#include "api/paper_controller.hpp"
#include "api/referee_controller.hpp"
#include "api/registration_controller.hpp"
#include "api/review_controller.hpp"
#include "api/schedule_controller.hpp"
#include "lib/validation.hpp"
#include "models/schedule.hpp"
#include "services/auth_service.hpp"
#include "services/decision_service.hpp"
#include "services/paper_service.hpp"
#include "services/payment_service.hpp"
#include "services/referee_service.hpp"
#include "services/registration_service.hpp"
#include "services/review_service.hpp"
#include "services/schedule_service.hpp"
#include "services/session_service.hpp"
#include "services/user_repository.hpp"
#include "test_assert.hpp"

namespace cms::api {
std::vector<std::string> RegisteredRoutes();
}

namespace {
std::string ExtractToken(const std::string& body) {
  const std::string marker = "session=";
  const auto pos = body.find(marker);
  Expect(pos != std::string::npos, "session token marker missing");
  return body.substr(pos + marker.size());
}

void TestHttpAndRoutes() {
  auto ok = cms::api::Ok("ok", "/next");
  auto bad = cms::api::BadRequest("bad");
  auto unauth = cms::api::Unauthorized("u");
  auto conflict = cms::api::Conflict("c");
  auto err = cms::api::InternalServerError("e");
  auto created = cms::api::Created("created", "/login");

  Expect(ok.status == 200 && ok.redirect_to == "/next", "Ok mapping");
  Expect(bad.status == 400 && bad.redirect_to.empty(), "BadRequest mapping");
  Expect(unauth.status == 401 && unauth.redirect_to.empty(), "Unauthorized mapping");
  Expect(conflict.status == 409 && conflict.redirect_to.empty(), "Conflict mapping");
  Expect(err.status == 500 && err.redirect_to.empty(), "InternalServerError mapping");
  Expect(created.status == 201 && created.redirect_to == "/login", "Created mapping");

  auto routes = cms::api::RegisteredRoutes();
  Expect(!routes.empty(), "registered routes should not be empty");
  Expect(std::find(routes.begin(), routes.end(), "POST /auth/register") != routes.end(),
         "register route should exist");
}

void TestValidationExtra() {
  Expect(cms::lib::IsValidEmailFormat("a@example.com"), "valid email should pass");
  Expect(!cms::lib::IsValidEmailFormat("bad-email"), "invalid email should fail");

  cms::models::ScheduleItem a{1, "R1", "S1"};
  cms::models::ScheduleItem b{1, "R1", "S1"};
  cms::models::ScheduleItem c{2, "R1", "S1"};
  Expect(a == b, "schedule item equality true");
  Expect(!(a == c), "schedule item equality false");
}

void TestAuthApiAndService() {
  cms::services::UserRepository users;
  cms::services::AuthService auth(&users);
  cms::services::SessionService sessions;
  cms::api::AuthController auth_controller(&auth, &sessions);
  cms::api::AuthController auth_no_session(&auth);

  Expect(auth_controller.Register({"", "u", "u@example.com", "GoodPass1!"}).status == 400,
         "register missing fields");
  Expect(auth_controller.Register({"N", "u", "bad", "GoodPass1!"}).status == 400,
         "register invalid email");
  Expect(auth_controller.Register({"N", "u", "u@example.com", "weak"}).status == 400,
         "register weak password");
  Expect(auth_controller.Register({"N", "u", "u@example.com", "GoodPass1!"}).status == 201,
         "register success");
  Expect(auth_controller.Register({"N2", "u2", "U@EXAMPLE.COM", "GoodPass2!"}).status == 409,
         "register duplicate");

  Expect(auth_controller.Login({"", ""}).status == 400, "login missing fields");
  Expect(auth_controller.Login({"u", "Wrong1!"}).status == 401, "login invalid creds");

  auto login = auth_controller.Login({"u", "GoodPass1!"});
  Expect(login.status == 200, "login success");
  const auto token = ExtractToken(login.body);

  auto stateless_login = auth_no_session.Login({"u", "GoodPass1!"});
  Expect(stateless_login.status == 200 && stateless_login.body.find("session=") == std::string::npos,
         "login without session service");

  Expect(auth_no_session.ChangePassword(token, {"GoodPass1!", "GoodPass2!", "GoodPass2!"}).status == 401,
         "change password without session service");
  Expect(auth_controller.ChangePassword("bad", {"GoodPass1!", "GoodPass2!", "GoodPass2!"}).status == 401,
         "change password bad token");
  Expect(auth_controller.ChangePassword(token, {"", "", ""}).status == 400,
         "change password missing fields");
  Expect(auth_controller.ChangePassword(token, {"GoodPass1!", "GoodPass2!", "Mismatch2!"}).status == 400,
         "change password mismatch");

  auto ghost = sessions.CreateSession(9999);
  Expect(auth_controller.ChangePassword(ghost, {"x", "GoodPass2!", "GoodPass2!"}).status == 401,
         "change password user not found");

  Expect(auth_controller.ChangePassword(token, {"Wrong1!", "GoodPass2!", "GoodPass2!"}).status == 401,
         "change password wrong current");
  Expect(auth_controller.ChangePassword(token, {"GoodPass1!", "GoodPass1!", "GoodPass1!"}).status == 400,
         "change password reuse");
  Expect(auth_controller.ChangePassword(token, {"GoodPass1!", "weak", "weak"}).status == 400,
         "change password weak new");
  Expect(auth_controller.ChangePassword(token, {"GoodPass1!", "GoodPass2!", "GoodPass2!"}).status == 200,
         "change password success");

  Expect(auth_controller.ProtectedPage(token).status == 200, "protected page success");
  Expect(auth_controller.ProtectedPage("bad").status == 401, "protected page fail");
  Expect(auth_no_session.ProtectedPage("anything").status == 401, "protected page no session service");
  Expect(auth_controller.LoginPage(token).status == 200, "login page redirect for auth");
  Expect(auth_controller.LoginPage("bad").status == 200, "login page for guest");
  Expect(auth_no_session.LoginPage("any").status == 200, "login page no session service");

  Expect(auth.Login("u", "GoodPass2!"), "login helper by username");
  Expect(auth.Login("u@example.com", "GoodPass2!"), "login helper by email");
  Expect(!auth.Login("missing", "x"), "login helper missing");
}

void TestPaperControllerAndService() {
  cms::services::UserRepository users;
  cms::services::AuthService auth(&users);
  cms::services::SessionService sessions;
  cms::services::PaperService papers;
  cms::api::AuthController auth_controller(&auth, &sessions);
  cms::api::PaperController paper_controller(&papers, &sessions);

  auto reg = auth_controller.Register({"Author", "author", "author@example.com", "Author#123"});
  Expect(reg.status == 201, "author register");
  auto token = ExtractToken(auth_controller.Login({"author", "Author#123"}).body);

  Expect(paper_controller.SubmitPaper("bad", {"t", "a", "k", "m.pdf", 100}).status == 401,
         "submit paper unauthorized");
  Expect(paper_controller.SubmitPaper(token, {"", "a", "k", "m.pdf", 100}).status == 400,
         "submit paper missing metadata");
  Expect(paper_controller.SubmitPaper(token, {"t", "a", "k", "m.txt", 100}).status == 400,
         "submit paper invalid format");
  Expect(paper_controller.SubmitPaper(token, {"t", "a", "k", "m.pdf", 0}).status == 400,
         "submit paper bad size");
  Expect(paper_controller.SubmitPaper(token, {"T", "A", "K", "m.pdf", 100}).status == 201,
         "submit paper success");
  Expect(paper_controller.SubmitPaper(token, {"T", "A", "K", "m.pdf", 100}).status == 200,
         "submit paper duplicate");

  Expect(paper_controller.SaveDraft("bad", {"T", "A", "K"}).status == 401,
         "save draft unauthorized");
  Expect(paper_controller.SaveDraft(token, {"T", "A", "K", 0, true}).status == 500,
         "save draft system error");
  Expect(paper_controller.SaveDraft(token, {"bad@title", "A", "K"}).status == 400,
         "save draft invalid data");
  Expect(paper_controller.SaveDraft(token, {"", "", ""}).status == 400,
         "save draft empty");

  auto saved = paper_controller.SaveDraft(token, {"Draft", "", ""});
  Expect(saved.status == 201, "save draft success");
  Expect(paper_controller.SaveDraft(token, {"Updated", "Abs", "Key", 2}).status == 201,
         "save draft update");
  Expect(paper_controller.SaveDraft(token, {"x", "y", "z", 99}).status == 401,
         "save draft missing id");

  Expect(paper_controller.SubmitDraft("bad", 1, "x.pdf", 100).status == 401,
         "submit draft unauthorized");
  Expect(paper_controller.SubmitDraft(token, 1, "x.txt", 100).status == 400,
         "submit draft invalid format");
  Expect(paper_controller.SubmitDraft(token, 1, "x.pdf", 0).status == 400,
         "submit draft bad size");
  Expect(paper_controller.SubmitDraft(token, 999, "x.pdf", 100).status == 401,
         "submit draft not found");

  auto incomplete = paper_controller.SaveDraft(token, {"OnlyTitle", "", ""});
  Expect(incomplete.status == 201, "incomplete draft created");
  Expect(paper_controller.SubmitDraft(token, 3, "inc.pdf", 100).status == 400,
         "submit incomplete draft");

  auto full_draft = paper_controller.SaveDraft(token, {"Unique", "Abstract", "Key"});
  Expect(full_draft.status == 201, "full draft created");
  Expect(paper_controller.SubmitDraft(token, 4, "u.pdf", 100).status == 200, "submit draft success");

  auto dup_draft = paper_controller.SaveDraft(token, {"Unique", "Abstract", "Key"});
  Expect(dup_draft.status == 201, "duplicate-key draft created");
  Expect(paper_controller.SubmitDraft(token, 5, "u.pdf", 100).status == 200,
         "submit draft duplicate submission path");

  Expect(papers.FindById(123456).has_value() == false, "paper find miss");
  Expect(!papers.UpdatePaperStatus(123456, cms::models::PaperStatus::kAccepted), "update missing paper");
  Expect(papers.AcceptedCount() >= 0, "accepted count executed");
}

void TestRefereeReviewDecisionAndControllers() {
  cms::services::PaperService papers;
  cms::services::RefereeService referees(&papers);
  cms::services::ReviewService reviews(&referees);
  cms::services::DecisionService decisions(&papers, &reviews);
  cms::api::RefereeController referee_controller(&referees);
  cms::api::ReviewController review_controller(&reviews);
  cms::api::DecisionController decision_controller(&decisions);

  auto p1 = papers.SubmitPaper(1, {"P1", "A1", "k", "p1.pdf", 100});
  auto p2 = papers.SubmitPaper(1, {"P2", "A2", "k", "p2.pdf", 100});
  auto p3 = papers.SubmitPaper(1, {"P3", "A3", "k", "p3.pdf", 100});
  Expect(p1.status == cms::services::SubmitPaperStatus::kSubmitted, "p1 submit");

  Expect(referee_controller.AssignReferee(9999, "a@x.com").status == 400,
         "assign non-submitted paper");
  Expect(referee_controller.AssignReferee(p1.paper_id, "bad").status == 400, "assign bad email");

  auto a1 = referees.AssignReferee(p1.paper_id, "r1@example.com");
  auto a2 = referees.AssignReferee(p1.paper_id, "r2@example.com");
  auto a3 = referees.AssignReferee(p1.paper_id, "r3@example.com");
  Expect(a1.status == cms::services::AssignRefereeStatus::kAssigned, "assign success");
  Expect(referee_controller.AssignReferee(p1.paper_id, "R1@example.com").status == 409,
         "assign duplicate");
  Expect(referee_controller.AssignReferee(p1.paper_id, "r4@example.com").status == 409,
         "assign over capacity");

  referees.SetRefereeWorkloadForTesting("busy@example.com", 5);
  Expect(referee_controller.AssignReferee(p2.paper_id, "busy@example.com").status == 409,
         "assign workload exceeded");

  int batch_assigned = referees.AssignRefereesBatch(
      p2.paper_id, {"b1@example.com", "b2@example.com", "b3@example.com", "b4@example.com"});
  Expect(batch_assigned == 3, "batch assignment should stop at full capacity");

  Expect(referee_controller.AcceptInvitation(999999).status == 400, "accept invalid invitation");
  Expect(referee_controller.AcceptInvitation(a1.assignment_id).status == 200, "accept invitation");
  Expect(referee_controller.AcceptInvitation(a1.assignment_id).status == 200, "accept already accepted");
  Expect(referee_controller.AcceptInvitation(a2.assignment_id).status == 200, "accept a2");
  Expect(referee_controller.AcceptInvitation(a3.assignment_id).status == 200, "accept a3");

  auto rej_inv = referees.CreateInvitationForTesting(p3.paper_id, "rej@example.com");
  Expect(referee_controller.RejectInvitation(rej_inv).status == 200, "reject invitation");
  Expect(referee_controller.AcceptInvitation(rej_inv).status == 409, "accept already rejected conflict");

  auto cap_inv = referees.CreateInvitationForTesting(p1.paper_id, "overflow@example.com");
  Expect(referee_controller.AcceptInvitation(cap_inv).status == 409, "accept paper capacity reached");

  auto wl_inv = referees.CreateInvitationForTesting(p3.paper_id, "wl@example.com");
  referees.SetRefereeWorkloadForTesting("wl@example.com", 5);
  Expect(referee_controller.AcceptInvitation(wl_inv).status == 409, "accept workload reached");

  Expect(referee_controller.RejectInvitation(a1.assignment_id).status == 409,
         "reject already accepted conflict");
  Expect(referee_controller.RejectInvitation(999998).status == 400, "reject invalid invitation");
  Expect(referee_controller.RejectInvitation(rej_inv).status == 200, "reject already rejected");

  Expect(referee_controller.ExitAssignment(p1.paper_id).status == 200, "exit assignment full");
  Expect(referee_controller.ExitAssignment(p3.paper_id).status == 200, "exit assignment incomplete");

  Expect(referees.CountAcceptedPapersForReferee("r1@example.com") >= 1,
         "accepted papers for referee");
  Expect(referees.CountAcceptedPapersForReferee("none@example.com") == 0,
         "accepted papers empty branch");
  Expect(!referees.IsPaperAssignedToReferee(999, "nobody@example.com"),
         "assigned-to-referee missing paper");

  Expect(review_controller.SubmitReview({0, "", "", ""}).status == 400, "review missing fields");
  Expect(review_controller.SubmitReview({p1.paper_id, "r2@example.com", "ok", "accept", true}).status == 500,
         "review system error");
  Expect(review_controller.SubmitReview({p1.paper_id, "r2@example.com", "bad@@", "accept"}).status == 400,
         "review invalid data");
  Expect(review_controller.SubmitReview({p1.paper_id, "r4@example.com", "ok", "accept"}).status == 409,
         "review not assigned until accepted");

  Expect(review_controller.SubmitReview({p1.paper_id, "r1@example.com", "good", "accept"}).status == 201,
         "review submit 1");
  Expect(review_controller.SubmitReview({p1.paper_id, "r2@example.com", "good", "accept"}).status == 201,
         "review submit 2");
  Expect(review_controller.SubmitReview({p1.paper_id, "r3@example.com", "good", "reject"}).status == 201,
         "review submit 3");
  Expect(review_controller.SubmitReview({p1.paper_id, "R1@example.com", "dup", "accept"}).status == 409,
         "review duplicate");

  Expect(reviews.CountReviewsForPaper(9999) == 0, "count reviews empty");
  auto paper_reviews = reviews.GetReviewsForPaper(p1.paper_id);
  Expect(!paper_reviews.empty(), "get reviews non-empty");

  Expect(decision_controller.RecordDecision({9999, "accept"}).status == 400, "decision paper missing");
  Expect(decision_controller.RecordDecision({p2.paper_id, "accept", true}).status == 500,
         "decision system error");
  Expect(decision_controller.RecordDecision({p2.paper_id, ""}).status == 400,
         "decision missing decision value");
  Expect(decision_controller.RecordDecision({p2.paper_id, "accept"}).status == 400,
         "decision insufficient reviews");

  Expect(decision_controller.RecordDecision({p1.paper_id, "accept"}).status == 201, "decision recorded accept");
  Expect(decision_controller.RecordDecision({p1.paper_id, "reject"}).status == 409,
         "decision duplicate conflict");

  auto p4 = papers.SubmitPaper(1, {"P4", "A4", "k", "p4.pdf", 100});
  auto d1 = referees.AssignReferee(p4.paper_id, "d1@example.com");
  auto d2 = referees.AssignReferee(p4.paper_id, "d2@example.com");
  auto d3 = referees.AssignReferee(p4.paper_id, "d3@example.com");
  referee_controller.AcceptInvitation(d1.assignment_id);
  referee_controller.AcceptInvitation(d2.assignment_id);
  referee_controller.AcceptInvitation(d3.assignment_id);
  review_controller.SubmitReview({p4.paper_id, "d1@example.com", "r", "reject"});
  review_controller.SubmitReview({p4.paper_id, "d2@example.com", "r", "reject"});
  review_controller.SubmitReview({p4.paper_id, "d3@example.com", "r", "reject"});
  Expect(decision_controller.RecordDecision({p4.paper_id, "reject"}).status == 201,
         "decision recorded reject path");
}

void TestRegistrationPaymentScheduleAndControllers() {
  cms::services::RegistrationService registrations;
  cms::services::PaymentService payments(&registrations);
  cms::services::SessionService sessions;
  cms::services::PaperService papers;
  cms::services::ScheduleService schedules(&papers);

  cms::services::UserRepository users;
  cms::services::AuthService auth(&users);
  cms::api::AuthController auth_controller(&auth, &sessions);
  cms::api::RegistrationController reg_controller(&registrations, &payments, &sessions);
  cms::api::ScheduleController schedule_controller(&schedules);

  auto user1 = auth.Register({"A", "a", "a@example.com", "GoodPass1!"});
  auto user2 = auth.Register({"B", "b", "b@example.com", "GoodPass1!"});
  const auto t1 = ExtractToken(auth_controller.Login({"a", "GoodPass1!"}).body);
  const auto t2 = ExtractToken(auth_controller.Login({"b", "GoodPass1!"}).body);

  Expect(reg_controller.CreateRegistration("bad", {"regular", true}).status == 401,
         "create registration unauthorized");
  Expect(reg_controller.CreateRegistration(t1, {"", true}).status == 400,
         "create registration invalid type blank");
  Expect(reg_controller.CreateRegistration(t1, {"student", false}).status == 400,
         "create registration closed");
  Expect(reg_controller.CreateRegistration(t1, {"regular", true}).status == 201,
         "create registration success");
  Expect(reg_controller.CreateRegistration(t1, {"vip", true}).status == 409,
         "create registration duplicate");

  auto reg = registrations.FindByAttendee(user1.account.id);
  Expect(reg.has_value(), "registration exists");

  Expect(!registrations.MarkRegistrationPaid(9999), "mark paid missing");
  Expect(registrations.MarkRegistrationFailed(reg->id), "mark failed existing");
  Expect(!registrations.MarkRegistrationFailed(9999), "mark failed missing");
  Expect(registrations.IsRegistrationOwnedBy(reg->id, user1.account.id), "owned true");
  Expect(!registrations.IsRegistrationOwnedBy(9999, user1.account.id), "owned missing reg");
  Expect(!registrations.IsRegistrationOwnedBy(reg->id, user2.account.id), "owned false other user");
  Expect(!registrations.IsAttendeeRegisteredAndPaid(user1.account.id), "failed is not paid");

  Expect(reg_controller.PayForRegistration("bad", reg->id, {"4242424242424242", "12/30", "123"}).status == 401,
         "pay unauthorized");
  Expect(reg_controller.PayForRegistration(t2, reg->id, {"4242424242424242", "12/30", "123"}).status == 401,
         "pay wrong owner");

  Expect(reg_controller.PayForRegistration(t1, reg->id, {"bad", "12/30", "123"}).status == 400,
         "pay invalid info length");
  Expect(reg_controller.PayForRegistration(t1, reg->id, {"424242424242424A", "12/30", "123"}).status == 400,
         "pay invalid non-digit card");
  Expect(reg_controller.PayForRegistration(t1, reg->id, {"4242424242424242", "12330", "123"}).status == 400,
         "pay invalid expiry slash");
  Expect(reg_controller.PayForRegistration(t1, reg->id,
                                           {"4242424242424242", "12/30", "12A"}).status == 400,
         "pay invalid cvv digits");

  Expect(reg_controller.PayForRegistration(t1, reg->id,
                                           {"4242424242424242", "12/30", "123", true}).status == 409,
         "pay declined");
  Expect(reg_controller.ViewMyRegistrationStatus(t1).body.find("failed") != std::string::npos,
         "status failed branch");

  auto created2 = reg_controller.CreateRegistration(t2, {"vip", true});
  Expect(created2.status == 201, "second registration success");
  auto reg2 = registrations.FindByAttendee(user2.account.id);
  Expect(reg2.has_value(), "second registration exists");
  Expect(reg_controller.PayForRegistration(t2, reg2->id,
                                           {"4242424242424242", "12/30", "123", false, true}).status == 500,
         "pay system error branch");
  Expect(reg_controller.PayForRegistration(t2, reg2->id,
                                           {"4242424242424242", "12/30", "123"}).status == 200,
         "pay success");
  Expect(reg_controller.PayForRegistration(t2, reg2->id,
                                           {"4242424242424242", "12/30", "123"}).status == 409,
         "pay duplicate");

  Expect(reg_controller.ViewMyRegistrationStatus("bad").status == 401, "view status unauthorized");
  Expect(reg_controller.ViewMyRegistrationStatus("missing-token").status == 401,
         "view status unauthorized missing");
  Expect(reg_controller.ViewMyRegistrationStatus(t2).body.find("paid") != std::string::npos,
         "view status paid");

  auto user3 = auth.Register({"C", "c", "c@example.com", "GoodPass1!"});
  const auto t3 = ExtractToken(auth_controller.Login({"c", "GoodPass1!"}).body);
  Expect(reg_controller.ViewMyRegistrationStatus(t3).status == 400, "view status no registration");

  Expect(!payments.HasTicketForRegistration(9999), "ticket false branch");
  Expect(payments.HasTicketForRegistration(reg2->id), "ticket true branch");

  Expect(schedule_controller.GenerateSchedule({1, 1, false}).status == 400, "schedule no accepted papers");

  auto s1 = papers.SubmitPaper(1, {"S1", "A", "k", "s1.pdf", 100});
  auto s2 = papers.SubmitPaper(1, {"S2", "A", "k", "s2.pdf", 100});
  papers.UpdatePaperStatus(s1.paper_id, cms::models::PaperStatus::kAccepted);
  papers.UpdatePaperStatus(s2.paper_id, cms::models::PaperStatus::kAccepted);

  Expect(schedule_controller.GenerateSchedule({0, 1, false}).status == 409, "schedule invalid rooms");
  Expect(schedule_controller.GenerateSchedule({1, 1, false}).status == 409, "schedule unsatisfiable capacity");
  Expect(schedule_controller.GenerateSchedule({2, 1, true}).status == 500, "schedule system error");

  auto gen = schedule_controller.GenerateSchedule({2, 1, false});
  Expect(gen.status == 201, "schedule generated");
  Expect(schedule_controller.GenerateSchedule({2, 1, false}).status == 201,
         "schedule up-to-date path");

  Expect(schedules.GetCurrentScheduleHtml().empty() == false, "schedule html exists");

  Expect(schedule_controller.EditCurrentSchedule({}, false).status == 401, "schedule edit unauthorized");
  Expect(schedule_controller.ViewCurrentSchedule().status == 200, "view schedule success");

  auto current = schedules.GetCurrentSchedule();
  Expect(current.has_value(), "schedule current exists");

  Expect(schedule_controller.EditCurrentSchedule({current->items, true}, true).status == 500,
         "edit schedule system error");
  Expect(schedule_controller.EditCurrentSchedule({current->items, false}, true).status == 400,
         "edit schedule no changes");

  auto bad_items = current->items;
  bad_items[0].paper_id = -1;
  Expect(schedule_controller.EditCurrentSchedule({bad_items, false}, true).status == 409,
         "edit schedule invalid item conflict");

  auto wrong_size = current->items;
  wrong_size.pop_back();
  Expect(schedule_controller.EditCurrentSchedule({wrong_size, false}, true).status == 409,
         "edit schedule size mismatch conflict");

  auto wrong_set = current->items;
  wrong_set[0].paper_id += 1000;
  Expect(schedule_controller.EditCurrentSchedule({wrong_set, false}, true).status == 409,
         "edit schedule different paper set conflict");

  auto edited = current->items;
  std::swap(edited[0].room, edited[1].room);
  Expect(schedule_controller.EditCurrentSchedule({edited, false}, true).status == 200,
         "edit schedule updated");

  cms::services::ScheduleService empty_schedule(&papers);
  cms::api::ScheduleController empty_schedule_controller(&empty_schedule);
  Expect(empty_schedule_controller.EditCurrentSchedule({{}, false}, true).status == 400,
         "edit with no schedule");
  Expect(empty_schedule.GetCurrentScheduleHtml().empty(), "empty schedule html branch");
  Expect(empty_schedule_controller.ViewCurrentSchedule().status == 400, "view schedule empty");
}

void TestServiceShortCircuitEdges() {
  cms::services::UserRepository users;
  cms::services::AuthService auth(&users);
  auto created = auth.Register({"N", "u1", "u1@example.com", "GoodPass1!"});
  Expect(created.status == cms::services::RegisterStatus::kCreated, "seed user");

  Expect(auth.Register({"", "u2", "u2@example.com", "GoodPass1!"}).status ==
             cms::services::RegisterStatus::kMissingRequiredFields,
         "register blank name branch");
  Expect(auth.Register({"N", "", "u2@example.com", "GoodPass1!"}).status ==
             cms::services::RegisterStatus::kMissingRequiredFields,
         "register blank username branch");
  Expect(auth.Register({"N", "u2", "", "GoodPass1!"}).status ==
             cms::services::RegisterStatus::kMissingRequiredFields,
         "register blank email branch");
  Expect(auth.Register({"N", "u2", "u2@example.com", ""}).status ==
             cms::services::RegisterStatus::kMissingRequiredFields,
         "register blank password branch");

  Expect(auth.LoginByUsername({"", "GoodPass1!"}).status ==
             cms::services::LoginStatus::kMissingRequiredFields,
         "login blank username");
  Expect(auth.LoginByUsername({"u1", ""}).status ==
             cms::services::LoginStatus::kMissingRequiredFields,
         "login blank password");

  Expect(auth.ChangePassword(created.account.id, {"", "GoodPass2!", "GoodPass2!"}).status ==
             cms::services::ChangePasswordStatus::kMissingRequiredFields,
         "change password blank current");
  Expect(auth.ChangePassword(created.account.id, {"GoodPass1!", "", "GoodPass2!"}).status ==
             cms::services::ChangePasswordStatus::kMissingRequiredFields,
         "change password blank new");
  Expect(auth.ChangePassword(created.account.id, {"GoodPass1!", "GoodPass2!", ""}).status ==
             cms::services::ChangePasswordStatus::kMissingRequiredFields,
         "change password blank confirm");

  cms::services::PaperService papers;
  Expect(papers.SubmitPaper(0, {"T", "A", "K", "m.pdf", 10}).status ==
             cms::services::SubmitPaperStatus::kMissingMetadata,
         "submit paper author<=0");
  Expect(papers.SubmitPaper(1, {"", "A", "K", "m.pdf", 10}).status ==
             cms::services::SubmitPaperStatus::kMissingMetadata,
         "submit paper blank title");
  Expect(papers.SubmitPaper(1, {"T", "", "K", "m.pdf", 10}).status ==
             cms::services::SubmitPaperStatus::kMissingMetadata,
         "submit paper blank abstract");
  Expect(papers.SubmitPaper(1, {"T", "A", "", "m.pdf", 10}).status ==
             cms::services::SubmitPaperStatus::kMissingMetadata,
         "submit paper blank keywords");

  auto d1 = papers.SaveDraft(1, {"title", "abs", "k"});
  Expect(d1.status == cms::services::SaveDraftStatus::kSaved, "save draft normal");
  Expect(papers.SaveDraft(1, {"", "abs", ""}).status == cms::services::SaveDraftStatus::kSaved,
         "save draft blank title but non-blank abstract");
  Expect(papers.SaveDraft(1, {"", "", "k"}).status == cms::services::SaveDraftStatus::kSaved,
         "save draft blank title/abstract but non-blank keywords");
  auto d2 = papers.SaveDraft(2, {"title", "abs", "k"});
  Expect(d2.status == cms::services::SaveDraftStatus::kSaved, "save second draft");
  Expect(papers.SubmitDraft(999, d1.draft_id, "m.pdf", 10).status ==
             cms::services::SubmitPaperStatus::kDraftNotFound,
         "submit draft author mismatch");
  Expect(papers.SubmitDraft(1, d2.draft_id, "m.pdf", 10).status ==
             cms::services::SubmitPaperStatus::kDraftNotFound,
         "submit draft wrong owner mismatch");
  Expect(papers.SubmitDraft(1, d1.draft_id, "m.pdf", 10).status ==
             cms::services::SubmitPaperStatus::kSubmitted,
         "submit draft happy path");
  Expect(papers.SubmitDraft(1, d1.draft_id, "m.pdf", 10).status ==
             cms::services::SubmitPaperStatus::kDraftNotFound,
         "submit draft status mismatch after submit");
  Expect(papers.SubmitDraft(1, d2.draft_id, "m.pdf", 11 * 1024 * 1024).status ==
             cms::services::SubmitPaperStatus::kFileTooLarge,
         "submit draft too large upper bound");
  auto blank_abs = papers.SaveDraft(1, {"T", "", "K"});
  Expect(blank_abs.status == cms::services::SaveDraftStatus::kSaved, "blank abstract draft");
  Expect(papers.SubmitDraft(1, blank_abs.draft_id, "ba.pdf", 10).status ==
             cms::services::SubmitPaperStatus::kDraftIncomplete,
         "submit draft incomplete abstract");
  auto blank_kw = papers.SaveDraft(1, {"T", "A", ""});
  Expect(blank_kw.status == cms::services::SaveDraftStatus::kSaved, "blank keyword draft");
  Expect(papers.SubmitDraft(1, blank_kw.draft_id, "bk.pdf", 10).status ==
             cms::services::SubmitPaperStatus::kDraftIncomplete,
         "submit draft incomplete keywords");

  cms::services::RegistrationService regs;
  auto reg = regs.CreateRegistration(1, {"regular", true});
  Expect(reg.status == cms::services::CreateRegistrationStatus::kCreated, "create registration");
  cms::services::PaymentService payments(&regs);
  Expect(payments.ProcessPayment(reg.registration_id, {"1234567890123456", "1A/30", "123"}).status ==
             cms::services::PaymentStatus::kInvalidPaymentInfo,
         "payment invalid month digits");
  Expect(payments.ProcessPayment(reg.registration_id, {"1234567890123456", "12/3A", "123"}).status ==
             cms::services::PaymentStatus::kInvalidPaymentInfo,
         "payment invalid year digits");
  Expect(payments.ProcessPayment(reg.registration_id, {"123456789012345/", "12/30", "123"}).status ==
             cms::services::PaymentStatus::kInvalidPaymentInfo,
         "payment invalid card low non-digit");
  Expect(payments.ProcessPayment(reg.registration_id, {"1234567890123456", "12/30", "12/"}).status ==
             cms::services::PaymentStatus::kInvalidPaymentInfo,
         "payment invalid cvv low non-digit");
  Expect(payments.ProcessPayment(reg.registration_id, {"123", "12/30", "123"}).status ==
             cms::services::PaymentStatus::kInvalidPaymentInfo,
         "payment invalid card length");
  Expect(payments.ProcessPayment(reg.registration_id, {"1234567890123456", "12/30", "12"}).status ==
             cms::services::PaymentStatus::kInvalidPaymentInfo,
         "payment invalid cvv length");
  Expect(payments.ProcessPayment(reg.registration_id, {"1234567890123456", "12/3", "123"}).status ==
             cms::services::PaymentStatus::kInvalidPaymentInfo,
         "payment invalid expiry length");

  cms::services::RefereeService referees(&papers);
  auto p = papers.SubmitPaper(1, {"RP", "A", "k", "rp.pdf", 100});
  auto r = referees.AssignReferee(p.paper_id, "sr@example.com");
  Expect(r.status == cms::services::AssignRefereeStatus::kAssigned, "assign referee");
  Expect(referees.AssignRefereesBatch(p.paper_id, {}) == 0, "batch empty list");
  papers.UpdatePaperStatus(p.paper_id, cms::models::PaperStatus::kAccepted);
  Expect(referees.AssignReferee(p.paper_id, "late@example.com").status ==
             cms::services::AssignRefereeStatus::kPaperNotSubmitted,
         "assign on non-submitted status");
  auto rej_id = referees.CreateInvitationForTesting(p.paper_id, "sr@example.com");
  Expect(referees.RejectInvitation(rej_id).status == cms::services::InvitationResponseStatus::kRejected,
         "reject invited assignment");
  Expect(referees.CountAcceptedPapersForReferee("sr@example.com") == 0,
         "accepted papers false branch for rejected status");

  cms::services::ReviewService reviews(&referees);
  Expect(reviews.SubmitReview({-1, "r@example.com", "txt", "accept"}).status ==
             cms::services::ReviewSubmitStatus::kMissingFields,
         "review invalid paper id");
  Expect(reviews.SubmitReview({p.paper_id, " ", "txt", "accept"}).status ==
             cms::services::ReviewSubmitStatus::kMissingFields,
         "review blank email");
  Expect(reviews.SubmitReview({p.paper_id, "r@example.com", " ", "accept"}).status ==
             cms::services::ReviewSubmitStatus::kMissingFields,
         "review blank text");
  Expect(reviews.SubmitReview({p.paper_id, "r@example.com", "txt", " "}).status ==
             cms::services::ReviewSubmitStatus::kMissingFields,
         "review blank recommendation");

  cms::services::ScheduleService schedules(&papers);
  auto g1 = schedules.GenerateSchedule({2, 1, false});
  Expect(g1.status == cms::services::ScheduleGenerateStatus::kGenerated, "schedule generated base");
  auto p_extra = papers.SubmitPaper(1, {"Extra", "A", "k", "extra.pdf", 100});
  papers.UpdatePaperStatus(p_extra.paper_id, cms::models::PaperStatus::kAccepted);
  auto g_ids = schedules.GenerateSchedule({2, 1, false});
  Expect(g_ids.status == cms::services::ScheduleGenerateStatus::kGenerated,
         "input mismatch accepted ids");
  auto g2 = schedules.GenerateSchedule({3, 1, false});
  Expect(g2.status == cms::services::ScheduleGenerateStatus::kGenerated, "input mismatch room count");
  auto g3 = schedules.GenerateSchedule({3, 2, false});
  Expect(g3.status == cms::services::ScheduleGenerateStatus::kGenerated, "input mismatch slot count");
  Expect(schedules.GenerateSchedule({1, 0, false}).status ==
             cms::services::ScheduleGenerateStatus::kUnsatisfiableConstraints,
         "schedule unsat zero slots");
}

}  // namespace

int main() {
  TestHttpAndRoutes();
  TestValidationExtra();
  TestAuthApiAndService();
  TestPaperControllerAndService();
  TestRefereeReviewDecisionAndControllers();
  TestRegistrationPaymentScheduleAndControllers();
  TestServiceShortCircuitEdges();

  std::cout << "unit coverage complete test passed\n";
  return 0;
}
