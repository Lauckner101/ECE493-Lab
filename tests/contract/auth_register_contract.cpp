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

    // 201 created
    auto created = controller.Register({"Jane", "jane", "jane@example.com", "Valid#123"});
    Expect(created.status == 201, "expected contract 201 on create");

    // 400 validation error
    auto validation = controller.Register({"Jane", "jane2", "jane2@example.com", "short"});
    Expect(validation.status == 400, "expected contract 400 on validation failure");

    // 409 duplicate
    auto duplicate = controller.Register({"Jane2", "jane3", "JANE@EXAMPLE.COM", "Valid#123"});
    Expect(duplicate.status == 409, "expected contract 409 on duplicate email");

    std::cout << "auth/register contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
