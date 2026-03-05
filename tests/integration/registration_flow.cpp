#include <iostream>

#include "api/auth_controller.hpp"
#include "services/auth_service.hpp"
#include "services/user_repository.hpp"
#include "test_assert.hpp"

int main() {
  cms::services::UserRepository repository;
  cms::services::AuthService auth(&repository);
  cms::api::AuthController controller(&auth);

  auto bad_email = controller.Register({"Flow User", "flow", "flow.example.com", "Flow#2026"});
  Expect(bad_email.status == 400, "expected invalid email registration failure");

  auto weak = controller.Register({"Flow User", "flow", "flow@example.com", "weak"});
  Expect(weak.status == 400, "expected weak password registration failure");

  auto created = controller.Register({"Flow User", "flow", "flow@example.com", "Flow#2026"});
  Expect(created.status == 201, "expected successful registration in integration flow");

  auto duplicate = controller.Register({"Flow User 2", "flow2", "FLOW@EXAMPLE.COM", "Flow#3030"});
  Expect(duplicate.status == 409, "expected duplicate email conflict");

  auto account = repository.FindByEmail("flow@example.com");
  Expect(account.has_value(), "expected account persisted in repository");
  Expect(account->username == "flow", "expected persisted username");
  Expect(auth.Login("flow@example.com", "Flow#2026"), "expected credentials to authenticate");
  Expect(!auth.Login("flow@example.com", "Wrong#2026"), "expected wrong password rejection");

  std::cout << "registration flow integration test passed\n";
  return 0;
}
