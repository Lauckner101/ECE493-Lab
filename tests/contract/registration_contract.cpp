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

    Expect(auth.Register({"Contract Attendee", "catt", "catt@example.com", "Attendee#123"}).status ==
               cms::services::RegisterStatus::kCreated,
           "seed register failed");
    auto token = ExtractSessionToken(auth_controller.Login({"catt", "Attendee#123"}).body);

    // 201 created
    auto created = controller.CreateRegistration(token, {"regular", true});
    Expect(created.status == 201, "expected contract 201 for valid registration");

    // 400 invalid payload
    auto invalid = controller.CreateRegistration(token, {"", true});
    Expect(invalid.status == 400, "expected contract 400 for invalid attendance type");

    // 409 duplicate
    auto duplicate = controller.CreateRegistration(token, {"regular", true});
    Expect(duplicate.status == 409, "expected contract 409 for duplicate registration");

    std::cout << "registration contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
