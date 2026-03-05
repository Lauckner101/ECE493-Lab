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

int main() {
  cms::services::UserRepository users;
  cms::services::AuthService auth(&users);
  cms::services::SessionService sessions;
  cms::services::RegistrationService registrations;
  cms::services::PaymentService payments(&registrations);

  cms::api::AuthController auth_controller(&auth, &sessions);
  cms::api::RegistrationController registration_controller(&registrations, &payments, &sessions);

  auto reg_user = auth.Register({"Integration Attendee", "intatt", "intatt@example.com", "Attendee#123"});
  Expect(reg_user.status == cms::services::RegisterStatus::kCreated,
         "precondition failed: user register");

  auto second_user = auth.Register({"Integration Attendee 2", "intatt2", "intatt2@example.com", "Attendee#456"});
  Expect(second_user.status == cms::services::RegisterStatus::kCreated,
         "precondition failed: second user register");

  auto login = auth_controller.Login({"intatt", "Attendee#123"});
  const std::string marker = "session=";
  const auto marker_pos = login.body.find(marker);
  Expect(marker_pos != std::string::npos, "precondition failed: login session token");
  const std::string token = login.body.substr(marker_pos + marker.size());

  auto login_2 = auth_controller.Login({"intatt2", "Attendee#456"});
  const auto marker_pos_2 = login_2.body.find(marker);
  Expect(marker_pos_2 != std::string::npos, "precondition failed: second login session token");
  const std::string token_2 = login_2.body.substr(marker_pos_2 + marker.size());

  auto unauth_create = registration_controller.CreateRegistration("bad-token", {"regular", true});
  Expect(unauth_create.status == 401, "expected unauthorized registration creation");

  auto invalid_type = registration_controller.CreateRegistration(token, {"speaker", true});
  Expect(invalid_type.status == 400, "expected invalid attendance type");

  auto closed = registration_controller.CreateRegistration(token, {"regular", false});
  Expect(closed.status == 400, "expected registration closed response");

  auto created = registration_controller.CreateRegistration(token, {"regular", true});
  Expect(created.status == 201, "expected successful registration creation");

  auto duplicate_registration = registration_controller.CreateRegistration(token, {"regular", true});
  Expect(duplicate_registration.status == 409, "expected duplicate registration conflict");

  auto registration = registrations.FindByAttendee(reg_user.account.id);
  Expect(registration.has_value(), "expected stored registration");

  auto wrong_owner = registration_controller.PayForRegistration(
      token_2, registration->id, {"4242424242424242", "12/30", "123"});
  Expect(wrong_owner.status == 401, "expected ownership validation for payment");

  auto invalid_payment = registration_controller.PayForRegistration(
      token, registration->id, {"bad", "12/30", "123"});
  Expect(invalid_payment.status == 400, "expected invalid payment details");

  auto paid = registration_controller.PayForRegistration(
      token, registration->id, {"4242424242424242", "12/30", "123"});
  Expect(paid.status == 200, "expected successful payment");

  auto duplicate_payment = registration_controller.PayForRegistration(
      token, registration->id, {"4242424242424242", "12/30", "123"});
  Expect(duplicate_payment.status == 409, "expected duplicate payment conflict");

  Expect(registrations.IsAttendeeRegisteredAndPaid(reg_user.account.id),
         "expected attendee status paid");
  Expect(payments.HasTicketForRegistration(registration->id),
         "expected ticket generated");

  auto status = registration_controller.ViewMyRegistrationStatus(token);
  Expect(status.status == 200 && status.body.find("paid") != std::string::npos,
         "expected paid status on dashboard");

  auto unauth_status = registration_controller.ViewMyRegistrationStatus("bad-token");
  Expect(unauth_status.status == 401, "expected unauthorized status access");

  std::cout << "registration payment integration test passed\n";
  return 0;
}
