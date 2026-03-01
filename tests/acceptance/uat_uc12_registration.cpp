#include <iostream>
#include <string>

#include "api/auth_controller.hpp"
#include "api/registration_controller.hpp"
#include "services/auth_service.hpp"
#include "services/payment_service.hpp"
#include "services/registration_service.hpp"
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
}  // namespace

int main() {
  try {
    cms::services::UserRepository users;
    cms::services::AuthService auth(&users);
    cms::services::SessionService sessions;
    cms::services::RegistrationService registrations;
    cms::services::PaymentService payments(&registrations);

    cms::api::AuthController auth_controller(&auth, &sessions);
    cms::api::RegistrationController registration_controller(&registrations, &payments, &sessions);

    // Seed attendees.
    Expect(auth.Register({"Attendee One", "att1", "att1@example.com", "Attendee#123"}).status ==
               cms::services::RegisterStatus::kCreated,
           "setup failed: attendee 1 register");
    Expect(auth.Register({"Attendee Two", "att2", "att2@example.com", "Attendee#123"}).status ==
               cms::services::RegisterStatus::kCreated,
           "setup failed: attendee 2 register");
    Expect(auth.Register({"Attendee Three", "att3", "att3@example.com", "Attendee#123"}).status ==
               cms::services::RegisterStatus::kCreated,
           "setup failed: attendee 3 register");
    Expect(auth.Register({"Attendee Four", "att4", "att4@example.com", "Attendee#123"}).status ==
               cms::services::RegisterStatus::kCreated,
           "setup failed: attendee 4 register");

    auto token1 = ExtractSessionToken(auth_controller.Login({"att1", "Attendee#123"}).body);
    auto token2 = ExtractSessionToken(auth_controller.Login({"att2", "Attendee#123"}).body);
    auto token3 = ExtractSessionToken(auth_controller.Login({"att3", "Attendee#123"}).body);
    auto token4 = ExtractSessionToken(auth_controller.Login({"att4", "Attendee#123"}).body);

    // AT-UC12-001: successful registration and payment.
    auto reg1 = registration_controller.CreateRegistration(token1, {"regular", true});
    Expect(reg1.status == 201, "AT-UC12-001: registration should be created");
    int reg1_id = registrations.FindByAttendee(1)->id;
    auto pay1 = registration_controller.PayForRegistration(
        token1, reg1_id, {"4242424242424242", "12/30", "123"});
    Expect(pay1.status == 200, "AT-UC12-001: payment should succeed");
    Expect(payments.HasTicketForRegistration(reg1_id),
           "AT-UC12-001: ticket should be generated");

    // AT-UC12-002: payment declined.
    auto reg2 = registration_controller.CreateRegistration(token2, {"student", true});
    Expect(reg2.status == 201, "AT-UC12-002: registration precondition failed");
    int reg2_id = registrations.FindByAttendee(2)->id;
    auto declined = registration_controller.PayForRegistration(
        token2, reg2_id, {"5555555555555555", "01/31", "456", true});
    Expect(declined.status == 409, "AT-UC12-002: expected payment decline");
    Expect(registrations.FindById(reg2_id)->status == cms::models::RegistrationStatus::kFailed,
           "AT-UC12-002: registration should remain incomplete");

    // AT-UC12-003: invalid payment information.
    auto reg3 = registration_controller.CreateRegistration(token3, {"vip", true});
    Expect(reg3.status == 201, "AT-UC12-003: registration precondition failed");
    int reg3_id = registrations.FindByAttendee(3)->id;
    auto invalid = registration_controller.PayForRegistration(
        token3, reg3_id, {"badcard", "1/2", "x"});
    Expect(invalid.status == 400, "AT-UC12-003: expected validation error");

    // AT-UC12-004: system error while recording registration.
    auto reg4 = registration_controller.CreateRegistration(token4, {"regular", true});
    Expect(reg4.status == 201, "AT-UC12-004: registration precondition failed");
    int reg4_id = registrations.FindByAttendee(4)->id;
    auto save_err = registration_controller.PayForRegistration(
        token4, reg4_id, {"4012888888881881", "11/29", "999", false, true});
    Expect(save_err.status == 500, "AT-UC12-004: expected system error");
    Expect(registrations.FindById(reg4_id)->status != cms::models::RegistrationStatus::kPaid,
           "AT-UC12-004: registration should not be marked paid");

    // AT-UC12-005: confirmation delivery.
    Expect(payments.ConfirmationCount() > 0,
           "AT-UC12-005: confirmation should be delivered");
    Expect(payments.LastConfirmationMessage().find("id=") != std::string::npos,
           "AT-UC12-005: confirmation message should include registration id");

    // AT-UC12-006: registration status reflected in account.
    auto status_view = registration_controller.ViewMyRegistrationStatus(token1);
    Expect(status_view.status == 200 && status_view.body.find("paid") != std::string::npos,
           "AT-UC12-006: expected paid status");

    // AT-UC12-007: prevent duplicate payment.
    auto duplicate = registration_controller.PayForRegistration(
        token1, reg1_id, {"4242424242424242", "12/30", "123"});
    Expect(duplicate.status == 409, "AT-UC12-007: duplicate payment should be blocked");

    // AT-UC12-008: unauthorized user cannot register.
    auto unauthorized = registration_controller.CreateRegistration("", {"regular", true});
    Expect(unauthorized.status == 401, "AT-UC12-008: expected unauthorized response");

    std::cout << "UC-12 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
