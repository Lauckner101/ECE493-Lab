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
    cms::api::RegistrationController controller(&registrations, &payments, &sessions);

    Expect(auth.Register({"Pay Attendee", "payatt", "payatt@example.com", "Attendee#123"}).status ==
               cms::services::RegisterStatus::kCreated,
           "seed register failed");
    auto token = ExtractSessionToken(auth_controller.Login({"payatt", "Attendee#123"}).body);

    auto reg = controller.CreateRegistration(token, {"regular", true});
    Expect(reg.status == 201, "seed registration failed");
    int reg_id = registrations.FindByAttendee(1)->id;

    // 200 OK
    auto paid = controller.PayForRegistration(token, reg_id, {"4242424242424242", "12/30", "123"});
    Expect(paid.status == 200, "expected contract 200 for successful payment");

    // 409 duplicate payment
    auto duplicate = controller.PayForRegistration(token, reg_id, {"4242424242424242", "12/30", "123"});
    Expect(duplicate.status == 409, "expected contract 409 for duplicate payment");

    // 400 invalid payment info
    Expect(auth.Register({"Pay Attendee 2", "payatt2", "payatt2@example.com", "Attendee#123"}).status ==
               cms::services::RegisterStatus::kCreated,
           "seed register 2 failed");
    auto token2 = ExtractSessionToken(auth_controller.Login({"payatt2", "Attendee#123"}).body);
    auto reg2 = controller.CreateRegistration(token2, {"student", true});
    Expect(reg2.status == 201, "seed registration 2 failed");
    int reg2_id = registrations.FindByAttendee(2)->id;
    auto invalid = controller.PayForRegistration(token2, reg2_id, {"bad", "x", "9"});
    Expect(invalid.status == 400, "expected contract 400 for invalid payment data");

    // 409 declined
    auto declined =
        controller.PayForRegistration(token2, reg2_id, {"5555555555555555", "01/32", "456", true});
    Expect(declined.status == 409, "expected contract 409 for declined payment");

    std::cout << "payment contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
