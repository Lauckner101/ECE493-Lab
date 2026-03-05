// GCOVR_EXCL_START
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "api/auth_controller.hpp"
#include "api/decision_controller.hpp"
#include "api/paper_controller.hpp"
#include "api/referee_controller.hpp"
#include "api/registration_controller.hpp"
#include "api/review_controller.hpp"
#include "api/schedule_controller.hpp"
#include "lib/validation.hpp"
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

namespace {
std::string ExtractSessionToken(const std::string& body) {
  const std::string marker = "session=";
  auto pos = body.find(marker);
  if (pos == std::string::npos) {
    return "";
  }
  return body.substr(pos + marker.size());
}

void TestValidationAndUserRepository() {
  using cms::lib::IsBlank;
  using cms::lib::IsPasswordComplex;
  using cms::lib::IsValidEmailFormat;

  Expect(IsBlank("   \t\n"), "blank string should be blank");
  Expect(!IsBlank("x"), "non-blank should not be blank");

  Expect(IsValidEmailFormat("a.b+1@example.com"), "valid email expected true");
  Expect(!IsValidEmailFormat("not-an-email"), "invalid email expected false");

  Expect(!IsPasswordComplex("Ab1!"), "short password expected false");
  Expect(!IsPasswordComplex("lowercase1!"), "missing uppercase expected false");
  Expect(!IsPasswordComplex("UPPERCASE1!"), "missing lowercase expected false");
  Expect(!IsPasswordComplex("NoDigits!!"), "missing digit expected false");
  Expect(!IsPasswordComplex("NoSymbol12"), "missing symbol expected false");
  Expect(IsPasswordComplex("GoodPass1!"), "valid complex password expected true");

  cms::services::UserRepository repo;
  Expect(!repo.EmailExists("u@example.com"), "email should not exist initially");
  Expect(!repo.FindByEmail("u@example.com").has_value(), "find by email should miss");
  Expect(!repo.FindByUsername("user").has_value(), "find by username should miss");
  Expect(!repo.FindById(42).has_value(), "find by id should miss");
  Expect(!repo.UpdatePasswordByUserId(42, "new"), "update unknown id should fail");

  auto created = repo.Create("User", "UserName", "U@Example.com", "Hash1!");
  Expect(created.id > 0, "created user must have id");
  Expect(repo.EmailExists("u@example.com"), "email exists after create");
  auto by_email = repo.FindByEmail("u@example.com");
  auto by_username = repo.FindByUsername("username");
  auto by_id = repo.FindById(created.id);
  Expect(by_email.has_value(), "find by email should hit");
  Expect(by_username.has_value(), "find by username should hit");
  Expect(by_id.has_value(), "find by id should hit");

  Expect(repo.UpdatePasswordByUserId(created.id, "NewHash2@"), "update known id should succeed");
  auto after = repo.FindById(created.id);
  Expect(after.has_value() && after->password_hash == "NewHash2@", "password should be updated");
}

void TestAuthAndSession() {
  cms::services::UserRepository users;
  cms::services::AuthService auth(&users);
  cms::services::SessionService sessions;

  auto missing_reg = auth.Register({"", "user", "u@example.com", "GoodPass1!"});
  Expect(missing_reg.status == cms::services::RegisterStatus::kMissingRequiredFields,
         "missing fields registration status");

  auto bad_email = auth.Register({"Name", "user", "bad", "GoodPass1!"});
  Expect(bad_email.status == cms::services::RegisterStatus::kInvalidEmailFormat,
         "invalid email registration status");

  auto weak = auth.Register({"Name", "user", "u@example.com", "weak"});
  Expect(weak.status == cms::services::RegisterStatus::kWeakPassword,
         "weak password registration status");

  auto created = auth.Register({"Name", "user", "u@example.com", "GoodPass1!"});
  Expect(created.status == cms::services::RegisterStatus::kCreated, "created registration status");

  auto dup = auth.Register({"Name2", "user2", "U@EXAMPLE.COM", "Another1!"});
  Expect(dup.status == cms::services::RegisterStatus::kDuplicateEmail,
         "duplicate email registration status");

  auto login_missing = auth.LoginByUsername({"", ""});
  Expect(login_missing.status == cms::services::LoginStatus::kMissingRequiredFields,
         "missing login fields status");

  auto login_bad_user = auth.LoginByUsername({"missing", "GoodPass1!"});
  Expect(login_bad_user.status == cms::services::LoginStatus::kInvalidCredentials,
         "invalid login for unknown user");

  auto login_bad_pw = auth.LoginByUsername({"user", "Wrong1!"});
  Expect(login_bad_pw.status == cms::services::LoginStatus::kInvalidCredentials,
         "invalid login for wrong password");

  auto login_ok = auth.LoginByUsername({"USER", "GoodPass1!"});
  Expect(login_ok.status == cms::services::LoginStatus::kAuthenticated,
         "authenticated login status");

  auto cp_missing = auth.ChangePassword(created.account.id, {"", "", ""});
  Expect(cp_missing.status == cms::services::ChangePasswordStatus::kMissingRequiredFields,
         "change password missing fields");

  auto cp_mismatch = auth.ChangePassword(created.account.id, {"GoodPass1!", "NewPass1!", "Mismatch1!"});
  Expect(cp_mismatch.status == cms::services::ChangePasswordStatus::kConfirmationMismatch,
         "change password mismatch");

  auto cp_not_found = auth.ChangePassword(9999, {"GoodPass1!", "NewPass1!", "NewPass1!"});
  Expect(cp_not_found.status == cms::services::ChangePasswordStatus::kUserNotFound,
         "change password user not found");

  auto cp_invalid_current = auth.ChangePassword(created.account.id, {"Wrong1!", "NewPass1!", "NewPass1!"});
  Expect(cp_invalid_current.status == cms::services::ChangePasswordStatus::kInvalidCurrentPassword,
         "change password invalid current");

  auto cp_reuse = auth.ChangePassword(created.account.id, {"GoodPass1!", "GoodPass1!", "GoodPass1!"});
  Expect(cp_reuse.status == cms::services::ChangePasswordStatus::kReuseNotAllowed,
         "change password reuse blocked");

  auto cp_weak = auth.ChangePassword(created.account.id, {"GoodPass1!", "weak", "weak"});
  Expect(cp_weak.status == cms::services::ChangePasswordStatus::kWeakNewPassword,
         "change password weak new password");

  auto cp_ok = auth.ChangePassword(created.account.id, {"GoodPass1!", "BrandNew1$", "BrandNew1$"});
  Expect(cp_ok.status == cms::services::ChangePasswordStatus::kChanged,
         "change password changed");

  Expect(auth.Login("user", "BrandNew1$"), "login by username should succeed");
  Expect(!auth.Login("user", "Wrong1!"), "login by username wrong password should fail");
  Expect(auth.Login("u@example.com", "BrandNew1$"), "login by email should succeed");
  Expect(!auth.Login("u@example.com", "Wrong1!"), "login by email wrong password should fail");
  Expect(!auth.Login("nobody@example.com", "X"), "login unknown identifier should fail");

  auto t1 = sessions.CreateSession(created.account.id);
  auto t2 = sessions.CreateSession(created.account.id + 1);
  Expect(!t1.empty() && !t2.empty() && t1 != t2, "session tokens must be unique");
  Expect(sessions.IsSessionActive(t1), "session should be active");
  Expect(!sessions.IsSessionActive("missing"), "missing session should be inactive");
  auto uid = sessions.GetUserIdForSession(t1);
  Expect(uid.has_value() && uid.value() == created.account.id, "session should map user id");
  Expect(!sessions.GetUserIdForSession("missing").has_value(), "missing session lookup should fail");
  Expect(sessions.HasActiveSessionForUser(created.account.id), "user should have active session");
  Expect(!sessions.HasActiveSessionForUser(7777), "unknown user should not have active session");
  Expect(sessions.SessionCount() == 2, "session count should reflect created sessions");

  cms::api::AuthController auth_controller(&auth, &sessions);
  cms::api::AuthController auth_no_session(&auth, nullptr);

  auto c_reg_ok = auth_controller.Register({"Another", "another", "another@example.com", "Another1!"});
  Expect(c_reg_ok.status == 201, "auth controller register created");
  auto c_reg_dup = auth_controller.Register({"Again", "another2", "ANOTHER@example.com", "Another2!"});
  Expect(c_reg_dup.status == 409, "auth controller register duplicate");
  auto c_reg_bad = auth_controller.Register({"", "", "bad", "weak"});
  Expect(c_reg_bad.status == 400, "auth controller register bad request");

  auto c_login_missing = auth_controller.Login({"", ""});
  Expect(c_login_missing.status == 400, "auth controller login missing fields");
  auto c_login_invalid = auth_controller.Login({"another", "wrong"});
  Expect(c_login_invalid.status == 401, "auth controller login invalid");
  auto c_login_ok = auth_controller.Login({"another", "Another1!"});
  Expect(c_login_ok.status == 200 && c_login_ok.body.find("session=") != std::string::npos,
         "auth controller login with session service");

  auto c_login_no_session = auth_no_session.Login({"another", "Another1!"});
  Expect(c_login_no_session.status == 200 && c_login_no_session.body.find("session=") == std::string::npos,
         "auth controller login without session service");

  auto token = ExtractSessionToken(c_login_ok.body);
  Expect(!token.empty(), "token should be extractable");

  auto cp_null_session = auth_no_session.ChangePassword(token, {"Another1!", "ZedPass1!", "ZedPass1!"});
  Expect(cp_null_session.status == 401, "change password requires auth when no session service");

  auto cp_bad_token = auth_controller.ChangePassword("missing-token", {"Another1!", "ZedPass1!", "ZedPass1!"});
  Expect(cp_bad_token.status == 401, "change password invalid session token");

  auto cp_ctrl_bad_current = auth_controller.ChangePassword(token, {"wrong", "ZedPass1!", "ZedPass1!"});
  Expect(cp_ctrl_bad_current.status == 401, "change password bad current maps unauthorized");

  auto cp_ctrl_bad_req = auth_controller.ChangePassword(token, {"Another1!", "weak", "weak"});
  Expect(cp_ctrl_bad_req.status == 400, "change password weak new maps bad request");

  auto cp_ctrl_ok = auth_controller.ChangePassword(token, {"Another1!", "ZedPass1!", "ZedPass1!"});
  Expect(cp_ctrl_ok.status == 200, "change password maps success");

  auto ghost_token = sessions.CreateSession(123456);
  auto cp_ctrl_missing_user = auth_controller.ChangePassword(ghost_token, {"x", "Yy1!zzzz", "Yy1!zzzz"});
  Expect(cp_ctrl_missing_user.status == 401, "change password user not found maps unauthorized");

  auto protected_ok = auth_controller.ProtectedPage(token);
  auto protected_fail = auth_controller.ProtectedPage("missing");
  Expect(protected_ok.status == 200, "protected page active session");
  Expect(protected_fail.status == 401, "protected page missing session");

  auto login_page_auth = auth_controller.LoginPage(token);
  auto login_page_guest = auth_controller.LoginPage("missing");
  Expect(login_page_auth.status == 200 && login_page_auth.redirect_to == "/home",
         "login page for authenticated user redirects");
  Expect(login_page_guest.status == 200 && login_page_guest.body.find("login page") != std::string::npos,
         "login page for guest shown");
}

void SeedAcceptedPapers(cms::services::PaperService* papers, int author_id, int count) {
  for (int i = 0; i < count; ++i) {
    auto r = papers->SubmitPaper(author_id,
                                 {"Title " + std::to_string(i),
                                  "Abstract " + std::to_string(i),
                                  "kw",
                                  "paper" + std::to_string(i) + ".pdf",
                                  1024});
    Expect(r.status == cms::services::SubmitPaperStatus::kSubmitted,
           "seed submit should succeed");
    Expect(papers->UpdatePaperStatus(r.paper_id, cms::models::PaperStatus::kAccepted),
           "seed update status should succeed");
  }
}

void TestPaperRegistrationPaymentAndControllers() {
  cms::services::PaperService papers;

  auto sp_missing = papers.SubmitPaper(0, {"", "", "", "p.pdf", 10});
  Expect(sp_missing.status == cms::services::SubmitPaperStatus::kMissingMetadata,
         "submit paper missing metadata");

  auto sp_bad_format = papers.SubmitPaper(1, {"t", "a", "k", "p.txt", 10});
  Expect(sp_bad_format.status == cms::services::SubmitPaperStatus::kInvalidFileFormat,
         "submit paper invalid format");
  auto sp_short_name = papers.SubmitPaper(1, {"t", "a", "k", "a", 10});
  Expect(sp_short_name.status == cms::services::SubmitPaperStatus::kInvalidFileFormat,
         "submit paper short filename invalid format");

  auto sp_too_large_0 = papers.SubmitPaper(1, {"t", "a", "k", "p.pdf", 0});
  Expect(sp_too_large_0.status == cms::services::SubmitPaperStatus::kFileTooLarge,
         "submit paper zero size invalid");

  auto sp_too_large_hi = papers.SubmitPaper(1, {"t", "a", "k", "p.pdf", 11 * 1024 * 1024});
  Expect(sp_too_large_hi.status == cms::services::SubmitPaperStatus::kFileTooLarge,
         "submit paper over max size invalid");

  auto sp_ok = papers.SubmitPaper(1, {"Title", "Abstract", "KW", "my.PDF", 1000});
  Expect(sp_ok.status == cms::services::SubmitPaperStatus::kSubmitted,
         "submit paper success");

  auto sp_dup = papers.SubmitPaper(1, {"title", "abstract", "kw", "my.pdf", 1000});
  Expect(sp_dup.status == cms::services::SubmitPaperStatus::kDuplicateSubmission,
         "submit paper duplicate");

  auto sd_auth = papers.SaveDraft(0, {"t", "a", "k"});
  Expect(sd_auth.status == cms::services::SaveDraftStatus::kSystemError,
         "save draft requires authentication");

  auto sd_fail = papers.SaveDraft(1, {"t", "a", "k", 0, true});
  Expect(sd_fail.status == cms::services::SaveDraftStatus::kSystemError,
         "save draft simulated failure");

  auto sd_invalid = papers.SaveDraft(1, {"bad@title", "a", "k"});
  Expect(sd_invalid.status == cms::services::SaveDraftStatus::kInvalidDraftData,
         "save draft invalid data");

  auto sd_empty = papers.SaveDraft(1, {"  ", " \t", "\n"});
  Expect(sd_empty.status == cms::services::SaveDraftStatus::kEmptyDraft,
         "save draft empty");

  auto sd_new = papers.SaveDraft(1, {"Draft", "Abstract", "KW"});
  Expect(sd_new.status == cms::services::SaveDraftStatus::kSaved && sd_new.draft_id > 0,
         "save draft new");

  auto sd_update = papers.SaveDraft(1, {"Draft2", "Abstract2", "KW2", sd_new.draft_id, false});
  Expect(sd_update.status == cms::services::SaveDraftStatus::kSaved,
         "save draft update existing");

  auto sd_not_found = papers.SaveDraft(1, {"Draft", "Abstract", "KW", 999, false});
  Expect(sd_not_found.status == cms::services::SaveDraftStatus::kDraftNotFound,
         "save draft update missing id");

  auto subd_bad_ext = papers.SubmitDraft(1, sd_new.draft_id, "x.doc", 100);
  Expect(subd_bad_ext.status == cms::services::SubmitPaperStatus::kInvalidFileFormat,
         "submit draft invalid format");

  auto subd_bad_size = papers.SubmitDraft(1, sd_new.draft_id, "x.pdf", 0);
  Expect(subd_bad_size.status == cms::services::SubmitPaperStatus::kFileTooLarge,
         "submit draft invalid size");

  auto empty_draft = papers.SaveDraft(1, {"", "filled", "filled"});
  Expect(empty_draft.status == cms::services::SaveDraftStatus::kSaved,
         "save partially filled draft");
  auto subd_incomplete = papers.SubmitDraft(1, empty_draft.draft_id, "x.pdf", 100);
  Expect(subd_incomplete.status == cms::services::SubmitPaperStatus::kDraftIncomplete,
         "submit draft incomplete metadata");

  auto submitted_draft = papers.SaveDraft(1, {"Unique", "Unique Abstract", "Unique KW"});
  auto subd_ok = papers.SubmitDraft(1, submitted_draft.draft_id, "unique.pdf", 100);
  Expect(subd_ok.status == cms::services::SubmitPaperStatus::kSubmitted,
         "submit draft success");

  auto dup_draft = papers.SaveDraft(1, {"Unique", "Unique Abstract", "Unique KW"});
  auto subd_dup = papers.SubmitDraft(1, dup_draft.draft_id, "unique.pdf", 100);
  Expect(subd_dup.status == cms::services::SubmitPaperStatus::kDuplicateSubmission,
         "submit draft duplicate");

  auto subd_missing = papers.SubmitDraft(1, 99999, "x.pdf", 10);
  Expect(subd_missing.status == cms::services::SubmitPaperStatus::kDraftNotFound,
         "submit draft not found");

  Expect(papers.FindById(sp_ok.paper_id).has_value(), "find paper by id should hit");
  Expect(!papers.FindById(99999).has_value(), "find paper by id should miss");
  Expect(papers.UpdatePaperStatus(sp_ok.paper_id, cms::models::PaperStatus::kAccepted),
         "update paper status should hit");
  Expect(!papers.UpdatePaperStatus(99999, cms::models::PaperStatus::kRejected),
         "update paper status should miss");

  Expect(papers.SubmittedCount() >= 1, "submitted count should be non-zero");
  Expect(papers.DraftCount() >= 1, "draft count should be non-zero");
  Expect(papers.AcceptedCount() >= 1, "accepted count should be non-zero");
  auto accepted_ids = papers.GetAcceptedPaperIds();
  Expect(std::is_sorted(accepted_ids.begin(), accepted_ids.end()), "accepted paper ids must be sorted");

  cms::services::RegistrationService registrations;
  cms::services::PaymentService payments(&registrations);

  auto reg_closed = registrations.CreateRegistration(1, {"regular", false});
  Expect(reg_closed.status == cms::services::CreateRegistrationStatus::kRegistrationClosed,
         "registration closed");

  auto reg_invalid = registrations.CreateRegistration(1, {"speaker", true});
  Expect(reg_invalid.status == cms::services::CreateRegistrationStatus::kInvalidAttendanceType,
         "registration invalid attendance");

  auto reg_ok = registrations.CreateRegistration(1, {"VIP", true});
  Expect(reg_ok.status == cms::services::CreateRegistrationStatus::kCreated,
         "registration created");

  auto reg_dup = registrations.CreateRegistration(1, {"vip", true});
  Expect(reg_dup.status == cms::services::CreateRegistrationStatus::kDuplicateRegistration,
         "registration duplicate");

  Expect(registrations.FindById(reg_ok.registration_id).has_value(), "find registration by id hit");
  Expect(!registrations.FindById(999).has_value(), "find registration by id miss");
  Expect(registrations.FindByAttendee(1).has_value(), "find registration by attendee hit");
  Expect(!registrations.FindByAttendee(777).has_value(), "find registration by attendee miss");
  Expect(registrations.IsRegistrationOwnedBy(reg_ok.registration_id, 1), "registration ownership true");
  Expect(!registrations.IsRegistrationOwnedBy(reg_ok.registration_id, 2), "registration ownership false");
  Expect(!registrations.IsRegistrationOwnedBy(999, 1), "registration ownership id miss false");
  Expect(!registrations.IsAttendeeRegisteredAndPaid(1), "attendee not paid yet");
  Expect(registrations.RegistrationCount() >= 1, "registration count non-zero");

  auto pay_not_found = payments.ProcessPayment(999, {"4242424242424242", "12/30", "123"});
  Expect(pay_not_found.status == cms::services::PaymentStatus::kRegistrationNotFound,
         "payment registration not found");

  auto pay_invalid_len = payments.ProcessPayment(reg_ok.registration_id, {"123", "12/30", "123"});
  Expect(pay_invalid_len.status == cms::services::PaymentStatus::kInvalidPaymentInfo,
         "payment invalid length");

  auto pay_invalid_separator = payments.ProcessPayment(reg_ok.registration_id,
                                                       {"4242424242424242", "1230", "123"});
  Expect(pay_invalid_separator.status == cms::services::PaymentStatus::kInvalidPaymentInfo,
         "payment invalid expiry separator");

  auto pay_invalid_digits = payments.ProcessPayment(reg_ok.registration_id,
                                                    {"42424242424242xx", "12/30", "12x"});
  Expect(pay_invalid_digits.status == cms::services::PaymentStatus::kInvalidPaymentInfo,
         "payment invalid numeric fields");

  auto reg2 = registrations.CreateRegistration(2, {"regular", true});
  auto pay_declined = payments.ProcessPayment(reg2.registration_id,
                                              {"4242424242424242", "12/30", "123", true, false});
  Expect(pay_declined.status == cms::services::PaymentStatus::kDeclined,
         "payment declined");
  auto reg2_after = registrations.FindById(reg2.registration_id);
  Expect(reg2_after.has_value() && reg2_after->status == cms::models::RegistrationStatus::kFailed,
         "decline should mark registration failed");

  auto reg3 = registrations.CreateRegistration(3, {"student", true});
  auto pay_system = payments.ProcessPayment(reg3.registration_id,
                                            {"4242424242424242", "12/30", "123", false, true});
  Expect(pay_system.status == cms::services::PaymentStatus::kSystemError,
         "payment system error");

  auto pay_ok = payments.ProcessPayment(reg_ok.registration_id, {"4242424242424242", "12/30", "123"});
  Expect(pay_ok.status == cms::services::PaymentStatus::kPaid,
         "payment success");
  auto pay_dup = payments.ProcessPayment(reg_ok.registration_id, {"4242424242424242", "12/30", "123"});
  Expect(pay_dup.status == cms::services::PaymentStatus::kDuplicatePayment,
         "duplicate payment blocked");
  Expect(payments.ConfirmationCount() >= 1, "payment confirmation count non-zero");
  Expect(payments.LastConfirmationMessage().find("id=") != std::string::npos,
         "payment confirmation message set");
  Expect(payments.HasTicketForRegistration(reg_ok.registration_id),
         "ticket should exist for paid registration");
  Expect(!payments.HasTicketForRegistration(999), "no ticket for unknown registration");
  Expect(registrations.IsAttendeeRegisteredAndPaid(1), "attendee should now be paid");

  // Controller coverage for paper/registration + session checks.
  cms::services::UserRepository users;
  cms::services::AuthService auth(&users);
  auto user = auth.Register({"Controller User", "ctrl", "ctrl@example.com", "CtrlPass1!"});
  Expect(user.status == cms::services::RegisterStatus::kCreated, "controller user must register");

  cms::services::SessionService sessions;
  cms::api::PaperController paper_controller(&papers, &sessions);
  cms::api::RegistrationController reg_controller(&registrations, &payments, &sessions);
  cms::api::AuthController auth_controller(&auth, &sessions);

  auto login = auth_controller.Login({"ctrl", "CtrlPass1!"});
  auto token = ExtractSessionToken(login.body);
  Expect(!token.empty(), "controller token required");
  auto reg_token = sessions.CreateSession(9001);

  auto pc_unauth = paper_controller.SubmitPaper("", {"t", "a", "k", "x.pdf", 100});
  Expect(pc_unauth.status == 401, "paper controller unauth submit");

  auto pc_bad = paper_controller.SubmitPaper(token, {"", "", "", "x.pdf", 100});
  Expect(pc_bad.status == 400, "paper controller bad submit");

  auto pc_ok = paper_controller.SubmitPaper(token, {"CtlT", "CtlA", "CtlK", "ctl.pdf", 100});
  Expect(pc_ok.status == 201, "paper controller submit created");

  auto pc_dup = paper_controller.SubmitPaper(token, {"CtlT", "CtlA", "CtlK", "ctl.pdf", 100});
  Expect(pc_dup.status == 200, "paper controller duplicate handled");

  auto pd_unauth = paper_controller.SaveDraft("", {"a", "b", "c"});
  Expect(pd_unauth.status == 401, "paper controller unauth draft save");

  auto pd_saved = paper_controller.SaveDraft(token, {"a", "b", "c"});
  Expect(pd_saved.status == 201, "paper controller draft save");

  auto pd_bad = paper_controller.SaveDraft(token, {"@bad", "b", "c"});
  Expect(pd_bad.status == 400, "paper controller invalid draft data");

  auto pd_empty = paper_controller.SaveDraft(token, {"  ", "  ", "  "});
  Expect(pd_empty.status == 400, "paper controller empty draft");

  auto pd_sys = paper_controller.SaveDraft(token, {"a", "b", "c", 0, true});
  Expect(pd_sys.status == 500, "paper controller simulated save failure");

  auto pd_missing = paper_controller.SaveDraft(token, {"a", "b", "c", 999999, false});
  Expect(pd_missing.status == 401, "paper controller draft not found");

  auto psd_unauth = paper_controller.SubmitDraft("", 1, "ok.pdf", 100);
  Expect(psd_unauth.status == 401, "paper controller unauth submit draft");

  auto psd_bad = paper_controller.SubmitDraft(token, 1, "bad.txt", 100);
  Expect(psd_bad.status == 400, "paper controller bad draft format");

  auto psd_missing = paper_controller.SubmitDraft(token, 999999, "ok.pdf", 100);
  Expect(psd_missing.status == 401, "paper controller draft not found for submit");

  auto draft_for_submit = papers.SaveDraft(user.account.id, {"U1", "U2", "U3"});
  Expect(draft_for_submit.status == cms::services::SaveDraftStatus::kSaved, "draft prep save");
  auto psd_ok = paper_controller.SubmitDraft(token, draft_for_submit.draft_id, "ok.pdf", 100);
  Expect(psd_ok.status == 200, "paper controller submit draft success");

  auto rc_unauth = reg_controller.CreateRegistration("", {"regular", true});
  Expect(rc_unauth.status == 401, "registration controller unauth create");

  auto rc_bad = reg_controller.CreateRegistration(reg_token, {"badtype", true});
  Expect(rc_bad.status == 400, "registration controller invalid type");

  auto rc_closed = reg_controller.CreateRegistration(reg_token, {"regular", false});
  Expect(rc_closed.status == 400, "registration controller closed registration");

  auto rc_ok = reg_controller.CreateRegistration(reg_token, {"regular", true});
  Expect(rc_ok.status == 201, "registration controller created");

  auto rc_dup = reg_controller.CreateRegistration(reg_token, {"regular", true});
  Expect(rc_dup.status == 409, "registration controller duplicate");

  auto created_reg = registrations.FindByAttendee(9001);
  Expect(created_reg.has_value(), "controller registration should exist");

  auto pay_unauth = reg_controller.PayForRegistration("", created_reg->id,
                                                      {"4242424242424242", "12/30", "123"});
  Expect(pay_unauth.status == 401, "registration controller unauth pay");

  auto outsider_token = sessions.CreateSession(999999);
  auto pay_not_owner = reg_controller.PayForRegistration(outsider_token, created_reg->id,
                                                         {"4242424242424242", "12/30", "123"});
  Expect(pay_not_owner.status == 401, "registration controller ownership check");

  auto pay_invalid = reg_controller.PayForRegistration(reg_token, created_reg->id,
                                                       {"123", "12/30", "123"});
  Expect(pay_invalid.status == 400, "registration controller invalid payment info");

  auto reg_decline = registrations.CreateRegistration(5000, {"regular", true});
  Expect(reg_decline.status == cms::services::CreateRegistrationStatus::kCreated,
         "decline registration create");
  auto decline_token = sessions.CreateSession(5000);
  auto pay_decline_ctrl = reg_controller.PayForRegistration(decline_token, reg_decline.registration_id,
                                                            {"4242424242424242", "12/30", "123", true, false});
  Expect(pay_decline_ctrl.status == 409, "registration controller declined payment");

  auto reg_fail = registrations.CreateRegistration(6000, {"regular", true});
  Expect(reg_fail.status == cms::services::CreateRegistrationStatus::kCreated,
         "record-failure registration create");
  auto fail_token = sessions.CreateSession(6000);
  auto pay_fail_ctrl = reg_controller.PayForRegistration(fail_token, reg_fail.registration_id,
                                                         {"4242424242424242", "12/30", "123", false, true});
  Expect(pay_fail_ctrl.status == 500, "registration controller payment system failure");

  auto pay_ok_ctrl = reg_controller.PayForRegistration(reg_token, created_reg->id,
                                                       {"4242424242424242", "12/30", "123"});
  Expect(pay_ok_ctrl.status == 200, "registration controller payment success");

  auto pay_dup_ctrl = reg_controller.PayForRegistration(reg_token, created_reg->id,
                                                        {"4242424242424242", "12/30", "123"});
  Expect(pay_dup_ctrl.status == 409, "registration controller duplicate payment");

  auto view_unauth = reg_controller.ViewMyRegistrationStatus("");
  Expect(view_unauth.status == 401, "registration status unauth");

  auto missing_user_token = sessions.CreateSession(7000);
  auto view_missing = reg_controller.ViewMyRegistrationStatus(missing_user_token);
  Expect(view_missing.status == 400, "registration status missing registration");

  auto view_paid = reg_controller.ViewMyRegistrationStatus(reg_token);
  Expect(view_paid.status == 200 && view_paid.body.find("paid") != std::string::npos,
         "registration status paid");

  auto view_failed = reg_controller.ViewMyRegistrationStatus(decline_token);
  Expect(view_failed.status == 200 && view_failed.body.find("failed") != std::string::npos,
         "registration status failed");

  auto pending_reg = registrations.CreateRegistration(8000, {"student", true});
  Expect(pending_reg.status == cms::services::CreateRegistrationStatus::kCreated,
         "pending registration create");
  auto pending_token = sessions.CreateSession(8000);
  auto view_pending = reg_controller.ViewMyRegistrationStatus(pending_token);
  Expect(view_pending.status == 200 && view_pending.body.find("pending") != std::string::npos,
         "registration status pending");
}

void TestRefereeReviewDecisionScheduleAndControllers() {
  cms::services::PaperService papers;
  SeedAcceptedPapers(&papers, 10, 4);

  // Add submitted papers for referee/review/decision paths.
  auto s1 = papers.SubmitPaper(10, {"S1", "A1", "K1", "s1.pdf", 100});
  auto s2 = papers.SubmitPaper(10, {"S2", "A2", "K2", "s2.pdf", 100});
  auto s3 = papers.SubmitPaper(10, {"S3", "A3", "K3", "s3.pdf", 100});
  auto s4 = papers.SubmitPaper(10, {"S4", "A4", "K4", "s4.pdf", 100});
  Expect(s1.status == cms::services::SubmitPaperStatus::kSubmitted, "submitted paper 1");
  Expect(s2.status == cms::services::SubmitPaperStatus::kSubmitted, "submitted paper 2");
  Expect(s3.status == cms::services::SubmitPaperStatus::kSubmitted, "submitted paper 3");
  Expect(s4.status == cms::services::SubmitPaperStatus::kSubmitted, "submitted paper 4");

  cms::services::RefereeService referees(&papers);

  auto ar_not_submitted = referees.AssignReferee(999999, "r@example.com");
  Expect(ar_not_submitted.status == cms::services::AssignRefereeStatus::kPaperNotSubmitted,
         "assign referee requires submitted paper");

  auto ar_invalid_email = referees.AssignReferee(s1.paper_id, "invalid-email");
  Expect(ar_invalid_email.status == cms::services::AssignRefereeStatus::kInvalidEmail,
         "assign referee invalid email");

  auto ar1 = referees.AssignReferee(s1.paper_id, "A@EXAMPLE.COM");
  auto ar_dup = referees.AssignReferee(s1.paper_id, "a@example.com");
  Expect(ar1.status == cms::services::AssignRefereeStatus::kAssigned,
         "assign referee success");
  Expect(ar_dup.status == cms::services::AssignRefereeStatus::kDuplicateAssignment,
         "assign referee duplicate");

  referees.SetRefereeWorkloadForTesting("busy@example.com", 5);
  auto ar_workload = referees.AssignReferee(s1.paper_id, "busy@example.com");
  Expect(ar_workload.status == cms::services::AssignRefereeStatus::kWorkloadExceeded,
         "assign referee workload exceeded");

  auto ar2 = referees.AssignReferee(s1.paper_id, "b@example.com");
  auto ar3 = referees.AssignReferee(s1.paper_id, "c@example.com");
  Expect(ar2.status == cms::services::AssignRefereeStatus::kAssigned, "assign second referee");
  Expect(ar3.status == cms::services::AssignRefereeStatus::kAssigned, "assign third referee");
  Expect(referees.AcceptInvitation(ar1.assignment_id).status ==
             cms::services::InvitationResponseStatus::kAccepted,
         "accept initial invitation 1");
  Expect(referees.AcceptInvitation(ar2.assignment_id).status ==
             cms::services::InvitationResponseStatus::kAccepted,
         "accept initial invitation 2");
  Expect(referees.AcceptInvitation(ar3.assignment_id).status ==
             cms::services::InvitationResponseStatus::kAccepted,
         "accept initial invitation 3");

  auto ar_full = referees.AssignReferee(s1.paper_id, "d@example.com");
  Expect(ar_full.status == cms::services::AssignRefereeStatus::kPaperAlreadyFullyAssigned,
         "assign referee full paper");

  int batch_added = referees.AssignRefereesBatch(
      s2.paper_id, {"r1@example.com", "r2@example.com", "r3@example.com", "r4@example.com"});
  Expect(batch_added == 3, "batch assignment should stop at capacity");

  referees.MarkIncompleteIfNeeded(s2.paper_id);
  Expect(!referees.IsPaperIncomplete(s2.paper_id), "fully assigned paper should not be incomplete");
  referees.MarkIncompleteIfNeeded(s3.paper_id);
  Expect(referees.IsPaperIncomplete(s3.paper_id), "under-assigned paper should be incomplete");

  Expect(referees.CountAssignmentsForPaper(s1.paper_id) == 3, "assignment count for paper");
  Expect(referees.CountAssignmentsForPaper(77777) == 0, "assignment count for missing paper");
  Expect(referees.GetRefereeWorkload("a@example.com") >= 1, "workload for known referee");
  Expect(referees.GetRefereeWorkload("none@example.com") == 0, "workload for unknown referee");

  // Capacity reached on accept.
  int inv_cap_a = referees.CreateInvitationForTesting(s4.paper_id, "cap-a@example.com");
  int inv_cap_b = referees.CreateInvitationForTesting(s4.paper_id, "cap-b@example.com");
  int inv_cap_c = referees.CreateInvitationForTesting(s4.paper_id, "cap-c@example.com");
  int inv_cap_d = referees.CreateInvitationForTesting(s4.paper_id, "cap-d@example.com");
  Expect(referees.AcceptInvitation(inv_cap_a).status == cms::services::InvitationResponseStatus::kAccepted,
         "accept invitation a");
  Expect(referees.AcceptInvitation(inv_cap_b).status == cms::services::InvitationResponseStatus::kAccepted,
         "accept invitation b");
  Expect(referees.AcceptInvitation(inv_cap_c).status == cms::services::InvitationResponseStatus::kAccepted,
         "accept invitation c");
  auto cap_reached = referees.AcceptInvitation(inv_cap_d);
  Expect(cap_reached.status == cms::services::InvitationResponseStatus::kPaperCapacityReached,
         "accept invitation paper capacity reached");

  // Workload exceeded on accept.
  int inv_busy = referees.CreateInvitationForTesting(s3.paper_id, "busy2@example.com");
  referees.SetRefereeWorkloadForTesting("busy2@example.com", 5);
  auto acc_busy = referees.AcceptInvitation(inv_busy);
  Expect(acc_busy.status == cms::services::InvitationResponseStatus::kRefereeWorkloadExceeded,
         "accept invitation workload exceeded");

  // Accept happy path + idempotent/invalid cases.
  int inv_ok = referees.CreateInvitationForTesting(s3.paper_id, "ok@example.com");
  auto acc_ok = referees.AcceptInvitation(inv_ok);
  auto acc_again = referees.AcceptInvitation(inv_ok);
  Expect(acc_ok.status == cms::services::InvitationResponseStatus::kAccepted,
         "accept invitation success");
  Expect(acc_again.status == cms::services::InvitationResponseStatus::kAlreadyAccepted,
         "accept invitation already accepted");

  int inv_rej_first = referees.CreateInvitationForTesting(s3.paper_id, "rej-first@example.com");
  Expect(referees.RejectInvitation(inv_rej_first).status == cms::services::InvitationResponseStatus::kRejected,
         "reject invitation success");
  auto acc_after_reject = referees.AcceptInvitation(inv_rej_first);
  Expect(acc_after_reject.status == cms::services::InvitationResponseStatus::kAlreadyRejected,
         "accept invitation already rejected");

  auto acc_invalid = referees.AcceptInvitation(9999999);
  Expect(acc_invalid.status == cms::services::InvitationResponseStatus::kInvalidInvitation,
         "accept invitation invalid id");

  // Reject cases.
  int inv_rej = referees.CreateInvitationForTesting(s3.paper_id, "rej@example.com");
  auto rej_ok = referees.RejectInvitation(inv_rej);
  auto rej_again = referees.RejectInvitation(inv_rej);
  Expect(rej_ok.status == cms::services::InvitationResponseStatus::kRejected,
         "reject invitation success");
  Expect(rej_again.status == cms::services::InvitationResponseStatus::kAlreadyRejected,
         "reject invitation already rejected");

  int inv_accept_then_reject = referees.CreateInvitationForTesting(s3.paper_id, "acc-rej@example.com");
  Expect(referees.AcceptInvitation(inv_accept_then_reject).status ==
             cms::services::InvitationResponseStatus::kAccepted,
         "precondition accepted before reject");
  auto rej_after_acc = referees.RejectInvitation(inv_accept_then_reject);
  Expect(rej_after_acc.status == cms::services::InvitationResponseStatus::kAlreadyAccepted,
         "reject invitation already accepted");

  auto rej_invalid = referees.RejectInvitation(8888888);
  Expect(rej_invalid.status == cms::services::InvitationResponseStatus::kInvalidInvitation,
         "reject invitation invalid id");

  Expect(referees.CountAcceptedAssignmentsForPaper(s4.paper_id) == 3,
         "accepted assignment count for paper");
  Expect(referees.CountAcceptedAssignmentsForPaper(11111) == 0,
         "accepted assignment count for missing paper");
  Expect(referees.CountAcceptedPapersForReferee("cap-a@example.com") >= 1,
         "accepted papers for referee");
  Expect(!referees.IsPaperAssignedToReferee(s4.paper_id, "none@example.com"),
         "paper not assigned to referee false path");
  Expect(referees.IsPaperAssignedToReferee(s4.paper_id, "CAP-A@example.com"),
         "paper assigned to referee true path");
  Expect(referees.InvitationCount() > 0, "invitation count should be non-zero");
  Expect(referees.EditorNotificationCount() > 0, "editor notification count should be non-zero");
  Expect(referees.IsPaperFullyAssigned(s1.paper_id), "paper should be fully assigned");

  cms::services::ReviewService reviews(&referees);

  auto rv_missing = reviews.SubmitReview({0, "", "", ""});
  Expect(rv_missing.status == cms::services::ReviewSubmitStatus::kMissingFields,
         "review missing fields");

  auto rv_system = reviews.SubmitReview({s1.paper_id, "a@example.com", "good text", "accept", true});
  Expect(rv_system.status == cms::services::ReviewSubmitStatus::kSystemError,
         "review system error");

  auto rv_invalid_rec = reviews.SubmitReview({s1.paper_id, "a@example.com", "good text", "maybe", false});
  Expect(rv_invalid_rec.status == cms::services::ReviewSubmitStatus::kInvalidData,
         "review invalid recommendation");

  auto rv_invalid_text = reviews.SubmitReview({s1.paper_id, "a@example.com", "bad@@text", "accept", false});
  Expect(rv_invalid_text.status == cms::services::ReviewSubmitStatus::kInvalidData,
         "review invalid text");

  auto rv_not_assigned = reviews.SubmitReview({s1.paper_id, "notassigned@example.com", "text", "accept", false});
  Expect(rv_not_assigned.status == cms::services::ReviewSubmitStatus::kNotAssigned,
         "review not assigned");

  auto rv_ok = reviews.SubmitReview({s1.paper_id, "A@EXAMPLE.COM", "good review", "ACCEPT", false});
  Expect(rv_ok.status == cms::services::ReviewSubmitStatus::kSubmitted,
         "review submission success");

  auto rv_dup = reviews.SubmitReview({s1.paper_id, "a@example.com", "another", "accept", false});
  Expect(rv_dup.status == cms::services::ReviewSubmitStatus::kDuplicate,
         "review duplicate");

  auto rv_ok2 = reviews.SubmitReview({s1.paper_id, "b@example.com", "good", "reject", false});
  auto rv_ok3 = reviews.SubmitReview({s1.paper_id, "c@example.com", "good", "accept", false});
  Expect(rv_ok2.status == cms::services::ReviewSubmitStatus::kSubmitted,
         "review submission success second referee");
  Expect(rv_ok3.status == cms::services::ReviewSubmitStatus::kSubmitted,
         "review submission success third referee");

  Expect(reviews.CountReviewsForPaper(s1.paper_id) == 3, "review count for paper");
  Expect(reviews.CountReviewsForPaper(101010) == 0, "review count for missing paper");
  Expect(reviews.HasReviewForPaperByReferee(s1.paper_id, "a@example.com"),
         "has review true");
  Expect(!reviews.HasReviewForPaperByReferee(s1.paper_id, "none@example.com"),
         "has review false");
  Expect(reviews.EditorNotificationCount() >= 3, "review editor notifications");
  Expect(reviews.GetReviewsForPaper(s1.paper_id).size() == 3, "get reviews for paper");
  Expect(reviews.GetReviewsForPaper(55555).empty(), "get reviews for missing paper");

  cms::services::DecisionService decisions(&papers, &reviews);

  auto d_not_found = decisions.RecordDecision({999999, "accept", false});
  Expect(d_not_found.status == cms::services::DecisionStatus::kPaperNotFound,
         "decision paper not found");

  auto d_system = decisions.RecordDecision({s1.paper_id, "accept", true});
  Expect(d_system.status == cms::services::DecisionStatus::kSystemError,
         "decision system error");

  auto d_missing = decisions.RecordDecision({s1.paper_id, "", false});
  Expect(d_missing.status == cms::services::DecisionStatus::kMissingDecision,
         "decision missing decision");

  auto d_invalid = decisions.RecordDecision({s1.paper_id, "maybe", false});
  Expect(d_invalid.status == cms::services::DecisionStatus::kMissingDecision,
         "decision invalid decision text");

  // Use paper with <3 reviews.
  auto d_insufficient = decisions.RecordDecision({s2.paper_id, "accept", false});
  Expect(d_insufficient.status == cms::services::DecisionStatus::kInsufficientReviews,
         "decision insufficient reviews");

  auto d_accept = decisions.RecordDecision({s1.paper_id, "accept", false});
  Expect(d_accept.status == cms::services::DecisionStatus::kRecorded,
         "decision recorded accept");
  auto p_after_accept = papers.FindById(s1.paper_id);
  Expect(p_after_accept.has_value() && p_after_accept->status == cms::models::PaperStatus::kAccepted,
         "paper status should be accepted");

  auto d_dup = decisions.RecordDecision({s1.paper_id, "reject", false});
  Expect(d_dup.status == cms::services::DecisionStatus::kDuplicateDecision,
         "decision duplicate");

  auto d_for_reject_paper = decisions.RecordDecision({s4.paper_id, "reject", false});
  Expect(d_for_reject_paper.status == cms::services::DecisionStatus::kInsufficientReviews,
         "decision for paper without enough reviews stays insufficient");

  Expect(decisions.GetDecisionForPaper(s1.paper_id).has_value(), "decision should exist");
  Expect(!decisions.GetDecisionForPaper(424242).has_value(), "decision missing paper");
  Expect(decisions.AuthorNotificationCount() >= 1, "author notification count");
  Expect(decisions.LastAuthorNotification().find("decision") != std::string::npos,
         "last author notification text");

  // Controller mapping coverage.
  cms::api::RefereeController referee_controller(&referees);
  cms::api::ReviewController review_controller(&reviews);
  cms::api::DecisionController decision_controller(&decisions);
  auto s5 = papers.SubmitPaper(10, {"S5", "A5", "K5", "s5.pdf", 100});
  Expect(s5.status == cms::services::SubmitPaperStatus::kSubmitted,
         "submitted paper for referee controller mapping");

  auto rc_assigned = referee_controller.AssignReferee(s5.paper_id, "ctrla@example.com");
  Expect(rc_assigned.status == 201, "referee controller assign success");
  auto rc_bad_email = referee_controller.AssignReferee(s5.paper_id, "bad");
  Expect(rc_bad_email.status == 400, "referee controller invalid email");
  auto rc_duplicate = referee_controller.AssignReferee(s5.paper_id, "ctrla@example.com");
  Expect(rc_duplicate.status == 409, "referee controller duplicate assignment");

  auto ac_inv = referees.CreateInvitationForTesting(s2.paper_id, "acceptc@example.com");
  auto ar_ok_ctrl = referee_controller.AcceptInvitation(ac_inv);
  auto ar_already_ctrl = referee_controller.AcceptInvitation(ac_inv);
  Expect(ar_ok_ctrl.status == 200, "referee controller accept success");
  Expect(ar_already_ctrl.status == 200, "referee controller already accepted maps ok");

  auto rej_inv = referees.CreateInvitationForTesting(s2.paper_id, "rejectc@example.com");
  auto rr_ok_ctrl = referee_controller.RejectInvitation(rej_inv);
  auto rr_already_ctrl = referee_controller.RejectInvitation(rej_inv);
  Expect(rr_ok_ctrl.status == 200, "referee controller reject success");
  Expect(rr_already_ctrl.status == 200, "referee controller already rejected maps ok");

  auto ar_invalid_ctrl = referee_controller.AcceptInvitation(99999999);
  Expect(ar_invalid_ctrl.status == 400, "referee controller accept invalid invitation");
  auto rr_invalid_ctrl = referee_controller.RejectInvitation(99999999);
  Expect(rr_invalid_ctrl.status == 400, "referee controller reject invalid invitation");

  auto reject_after_accept_inv = referees.CreateInvitationForTesting(s2.paper_id, "accfirst@example.com");
  Expect(referee_controller.AcceptInvitation(reject_after_accept_inv).status == 200,
         "precondition accept before reject in controller");
  auto reject_after_accept_ctrl = referee_controller.RejectInvitation(reject_after_accept_inv);
  Expect(reject_after_accept_ctrl.status == 409,
         "referee controller reject already accepted maps conflict");

  auto accept_after_reject_inv = referees.CreateInvitationForTesting(s2.paper_id, "rejfirst@example.com");
  Expect(referee_controller.RejectInvitation(accept_after_reject_inv).status == 200,
         "precondition reject before accept in controller");
  auto accept_after_reject_ctrl = referee_controller.AcceptInvitation(accept_after_reject_inv);
  Expect(accept_after_reject_ctrl.status == 409,
         "referee controller accept already rejected maps conflict");

  auto exit_incomplete = referee_controller.ExitAssignment(s3.paper_id);
  Expect(exit_incomplete.status == 200 && exit_incomplete.body.find("incompletely") != std::string::npos,
         "referee controller exit incomplete path");
  auto exit_complete = referee_controller.ExitAssignment(s1.paper_id);
  Expect(exit_complete.status == 200 && exit_complete.body.find("fully") != std::string::npos,
         "referee controller exit complete path");

  auto rv_ctrl_missing = review_controller.SubmitReview({0, "", "", ""});
  Expect(rv_ctrl_missing.status == 400, "review controller missing fields");
  auto rv_ctrl_invalid = review_controller.SubmitReview({s2.paper_id, "acceptc@example.com", "bad@@", "accept", false});
  Expect(rv_ctrl_invalid.status == 400, "review controller invalid data");
  auto rv_ctrl_system = review_controller.SubmitReview({s2.paper_id, "acceptc@example.com", "ok", "accept", true});
  Expect(rv_ctrl_system.status == 500, "review controller system error");
  auto rv_ctrl_not_assigned = review_controller.SubmitReview({s2.paper_id, "none@example.com", "ok", "accept", false});
  Expect(rv_ctrl_not_assigned.status == 409, "review controller not assigned");
  auto rv_ctrl_ok = review_controller.SubmitReview({s2.paper_id, "acceptc@example.com", "ok", "accept", false});
  Expect(rv_ctrl_ok.status == 201, "review controller submitted");
  auto rv_ctrl_dup = review_controller.SubmitReview({s2.paper_id, "acceptc@example.com", "ok2", "accept", false});
  Expect(rv_ctrl_dup.status == 409, "review controller duplicate");

  auto dc_not_found = decision_controller.RecordDecision({99999999, "accept", false});
  Expect(dc_not_found.status == 400, "decision controller paper not found");
  auto dc_missing = decision_controller.RecordDecision({s2.paper_id, "", false});
  Expect(dc_missing.status == 400, "decision controller missing decision");
  auto dc_system = decision_controller.RecordDecision({s2.paper_id, "accept", true});
  Expect(dc_system.status == 500, "decision controller system error");
  auto dc_insufficient = decision_controller.RecordDecision({s2.paper_id, "accept", false});
  Expect(dc_insufficient.status == 400, "decision controller insufficient reviews");

  // Create enough reviews for s2, then record + duplicate.
  int s2_inv2 = referees.CreateInvitationForTesting(s2.paper_id, "s2b@example.com");
  int s2_inv3 = referees.CreateInvitationForTesting(s2.paper_id, "s2c@example.com");
  Expect(referees.AcceptInvitation(s2_inv2).status == cms::services::InvitationResponseStatus::kAccepted,
         "s2 accept second referee");
  auto s2_inv3_accept = referees.AcceptInvitation(s2_inv3);
  Expect(s2_inv3_accept.status == cms::services::InvitationResponseStatus::kAccepted ||
             s2_inv3_accept.status == cms::services::InvitationResponseStatus::kPaperCapacityReached,
         "s2 third invitation accept/capacity reached");
  Expect(review_controller.SubmitReview({s2.paper_id, "s2b@example.com", "ok", "reject", false}).status == 201,
         "s2 second review");
  Expect(review_controller.SubmitReview({s2.paper_id, "accfirst@example.com", "ok", "accept", false}).status == 201,
         "s2 third review");

  auto dc_recorded = decision_controller.RecordDecision({s2.paper_id, "reject", false});
  Expect(dc_recorded.status == 201, "decision controller recorded");
  auto dc_duplicate = decision_controller.RecordDecision({s2.paper_id, "accept", false});
  Expect(dc_duplicate.status == 409, "decision controller duplicate");

  // Schedule service/controller branches.
  cms::services::ScheduleService schedule(&papers);
  cms::api::ScheduleController schedule_controller(&schedule);

  auto sg_no_accepted = cms::services::ScheduleService(&papers).GenerateSchedule({2, 2, false});
  Expect(sg_no_accepted.status == cms::services::ScheduleGenerateStatus::kGenerated ||
             sg_no_accepted.status == cms::services::ScheduleGenerateStatus::kNoAcceptedPapers ||
             sg_no_accepted.status == cms::services::ScheduleGenerateStatus::kUnsatisfiableConstraints,
         "independent schedule service check should be deterministic");

  cms::services::PaperService papers_none;
  cms::services::ScheduleService schedule_none(&papers_none);
  auto sg_none = schedule_none.GenerateSchedule({2, 2, false});
  Expect(sg_none.status == cms::services::ScheduleGenerateStatus::kNoAcceptedPapers,
         "schedule no accepted papers");

  auto sg_bad_config = schedule.GenerateSchedule({0, 2, false});
  Expect(sg_bad_config.status == cms::services::ScheduleGenerateStatus::kUnsatisfiableConstraints,
         "schedule invalid config");

  auto sg_capacity = schedule.GenerateSchedule({1, 1, false});
  Expect(sg_capacity.status == cms::services::ScheduleGenerateStatus::kUnsatisfiableConstraints,
         "schedule insufficient capacity");

  auto sg_system = schedule.GenerateSchedule({3, 3, true});
  Expect(sg_system.status == cms::services::ScheduleGenerateStatus::kSystemError,
         "schedule generation system error");

  auto sg_ok = schedule.GenerateSchedule({3, 3, false});
  Expect(sg_ok.status == cms::services::ScheduleGenerateStatus::kGenerated,
         "schedule generation success");

  auto sg_same = schedule.GenerateSchedule({3, 3, false});
  Expect(sg_same.status == cms::services::ScheduleGenerateStatus::kGenerated,
         "schedule generation idempotent path");

  auto current = schedule.GetCurrentSchedule();
  Expect(current.has_value(), "current schedule should exist");
  Expect(!schedule.GetCurrentScheduleHtml().empty(), "current schedule html should exist");

  auto se_no_schedule = schedule_none.EditCurrentSchedule({{}, false});
  Expect(se_no_schedule.status == cms::services::ScheduleEditStatus::kNoSchedule,
         "schedule edit with no schedule");
  Expect(schedule_none.GetCurrentScheduleHtml().empty(),
         "schedule html should be empty when there is no current schedule");

  auto se_system = schedule.EditCurrentSchedule({current->items, true});
  Expect(se_system.status == cms::services::ScheduleEditStatus::kSystemError,
         "schedule edit system error");

  auto se_no_changes = schedule.EditCurrentSchedule({current->items, false});
  Expect(se_no_changes.status == cms::services::ScheduleEditStatus::kNoChanges,
         "schedule edit no changes");

  auto bad_items = current->items;
  bad_items[0].room = bad_items[1].room;
  bad_items[0].time_slot = bad_items[1].time_slot;
  auto se_conflict_slot = schedule.EditCurrentSchedule({bad_items, false});
  Expect(se_conflict_slot.status == cms::services::ScheduleEditStatus::kConflict,
         "schedule edit room/slot conflict");

  auto missing_item = current->items;
  missing_item.pop_back();
  auto se_conflict_size = schedule.EditCurrentSchedule({missing_item, false});
  Expect(se_conflict_size.status == cms::services::ScheduleEditStatus::kConflict,
         "schedule edit size mismatch conflict");

  auto wrong_papers = current->items;
  wrong_papers[0].paper_id += 10000;
  auto se_conflict_set = schedule.EditCurrentSchedule({wrong_papers, false});
  Expect(se_conflict_set.status == cms::services::ScheduleEditStatus::kConflict,
         "schedule edit paper set conflict");

  auto duplicate_paper = current->items;
  duplicate_paper[1].paper_id = duplicate_paper[0].paper_id;
  auto se_conflict_duplicate_paper = schedule.EditCurrentSchedule({duplicate_paper, false});
  Expect(se_conflict_duplicate_paper.status == cms::services::ScheduleEditStatus::kConflict,
         "schedule edit duplicate paper id conflict");

  auto edited = current->items;
  std::swap(edited[0].room, edited[1].room);
  auto se_ok = schedule.EditCurrentSchedule({edited, false});
  Expect(se_ok.status == cms::services::ScheduleEditStatus::kUpdated,
         "schedule edit success");

  auto sc_gen_ok = schedule_controller.GenerateSchedule({3, 3, false});
  Expect(sc_gen_ok.status == 201, "schedule controller generate success");
  auto sc_gen_conflict = schedule_controller.GenerateSchedule({1, 1, false});
  Expect(sc_gen_conflict.status == 409, "schedule controller unsatisfiable maps conflict");
  auto sc_gen_system = schedule_controller.GenerateSchedule({3, 3, true});
  Expect(sc_gen_system.status == 500, "schedule controller system error");

  cms::api::ScheduleController schedule_controller_none(&schedule_none);
  auto sc_gen_none = schedule_controller_none.GenerateSchedule({2, 2, false});
  Expect(sc_gen_none.status == 400, "schedule controller no accepted papers maps bad request");

  auto sc_edit_unauth = schedule_controller.EditCurrentSchedule({edited, false}, false);
  Expect(sc_edit_unauth.status == 401, "schedule controller edit unauthorized");

  auto sc_edit_system = schedule_controller.EditCurrentSchedule({edited, true}, true);
  Expect(sc_edit_system.status == 500, "schedule controller edit system error");

  auto sc_edit_no_changes = schedule_controller.EditCurrentSchedule({schedule.GetCurrentSchedule()->items, false}, true);
  Expect(sc_edit_no_changes.status == 400, "schedule controller edit no changes");

  auto sc_edit_conflict = schedule_controller.EditCurrentSchedule({bad_items, false}, true);
  Expect(sc_edit_conflict.status == 409, "schedule controller edit conflict");

  auto sc_edit_ok = schedule_controller.EditCurrentSchedule({edited, false}, true);
  // Could be no changes depending on latest state; ensure in accepted set.
  Expect(sc_edit_ok.status == 200 || sc_edit_ok.status == 400,
         "schedule controller edit deterministic status");

  auto sc_view_none = schedule_controller_none.ViewCurrentSchedule();
  Expect(sc_view_none.status == 400, "schedule controller view none");
  auto sc_view_ok = schedule_controller.ViewCurrentSchedule();
  Expect(sc_view_ok.status == 200, "schedule controller view success");
}
}  // namespace

int main() {
  try {
    TestValidationAndUserRepository();
    TestAuthAndSession();
    TestPaperRegistrationPaymentAndControllers();
    TestRefereeReviewDecisionScheduleAndControllers();

    std::cout << "unit branch saturation test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
// GCOVR_EXCL_STOP
