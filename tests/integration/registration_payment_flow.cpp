// GCOVR_EXCL_START
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

    auto reg_user = auth.Register({"Integration Attendee", "intatt", "intatt@example.com", "Attendee#123"});
    Expect(reg_user.status == cms::services::RegisterStatus::kCreated,
           "precondition failed: user register");
    auto login = auth_controller.Login({"intatt", "Attendee#123"});
    auto token = ExtractSessionToken(login.body);
    Expect(!token.empty(), "precondition failed: login session token");

    auto created = registration_controller.CreateRegistration(token, {"regular", true});
    Expect(created.status == 201, "expected successful registration creation");
    auto registration = registrations.FindByAttendee(reg_user.account.id);
    Expect(registration.has_value(), "expected stored registration");

    auto paid = registration_controller.PayForRegistration(
        token, registration->id, {"4242424242424242", "12/30", "123"});
    Expect(paid.status == 200, "expected successful payment");
    Expect(registrations.IsAttendeeRegisteredAndPaid(reg_user.account.id),
           "expected attendee status paid");
    Expect(payments.HasTicketForRegistration(registration->id),
           "expected ticket generated");

    auto status = registration_controller.ViewMyRegistrationStatus(token);
    Expect(status.status == 200 && status.body.find("paid") != std::string::npos,
           "expected paid status on dashboard");

    std::cout << "registration payment integration test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
// GCOVR_EXCL_STOP
