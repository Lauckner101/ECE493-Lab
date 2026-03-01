#include <iostream>

#include "api/auth_controller.hpp"
#include "services/auth_service.hpp"
#include "services/user_repository.hpp"
#include "test_assert.hpp"

int main() {
  try {
    cms::services::UserRepository repository;
    cms::services::AuthService auth(&repository);
    cms::api::AuthController controller(&auth);

    auto result = controller.Register({"Flow User", "flow", "flow@example.com", "Flow#2026"});
    Expect(result.status == 201, "expected successful registration in integration flow");

    auto account = repository.FindByEmail("flow@example.com");
    Expect(account.has_value(), "expected account persisted in repository");
    Expect(account->username == "flow", "expected persisted username");
    Expect(auth.Login("flow@example.com", "Flow#2026"), "expected credentials to authenticate");

    std::cout << "registration flow integration test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
