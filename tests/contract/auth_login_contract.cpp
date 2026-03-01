#include <iostream>

#include "api/auth_controller.hpp"
#include "services/auth_service.hpp"
#include "services/session_service.hpp"
#include "services/user_repository.hpp"
#include "test_assert.hpp"

int main() {
  try {
    cms::services::UserRepository repository;
    cms::services::AuthService auth(&repository);
    cms::services::SessionService sessions;
    cms::api::AuthController controller(&auth, &sessions);

    auto reg = auth.Register({"Contract User", "contractuser", "contract@example.com", "Valid#123"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated,
           "precondition: registration failed for login contract test");

    // 200 authenticated
    auto ok = controller.Login({"contractuser", "Valid#123"});
    Expect(ok.status == 200, "expected contract 200 for valid credentials");

    // 400 validation error
    auto bad = controller.Login({"", "Valid#123"});
    Expect(bad.status == 400, "expected contract 400 for missing required fields");

    // 401 invalid credentials
    auto invalid = controller.Login({"contractuser", "Wrong#123"});
    Expect(invalid.status == 401, "expected contract 401 for invalid credentials");

    std::cout << "auth/login contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
