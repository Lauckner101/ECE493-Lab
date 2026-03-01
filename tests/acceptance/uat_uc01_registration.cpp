#include <iostream>

#include "api/auth_controller.hpp"
#include "services/auth_service.hpp"
#include "services/user_repository.hpp"
#include "test_assert.hpp"

using cms::api::AuthController;
using cms::services::AuthService;
using cms::services::RegisterRequest;
using cms::services::UserRepository;

int main() {
  try {
    UserRepository repository;
    AuthService auth(&repository);
    AuthController controller(&auth);

    // AT-UC01-001: successful registration.
    auto ok = controller.Register({"Alice", "alice", "alice@example.com", "Strong#123"});
    Expect(ok.status == 201, "AT-UC01-001: expected status 201");
    Expect(ok.redirect_to == "/login", "AT-UC01-001: expected redirect to login");

    // AT-UC01-002: duplicate email.
    auto dup = controller.Register({"Alice2", "alice2", "alice@example.com", "Strong#123"});
    Expect(dup.status == 409, "AT-UC01-002: expected duplicate to return 409");

    // AT-UC01-003: invalid password.
    auto weak = controller.Register({"Bob", "bob", "bob@example.com", "weak"});
    Expect(weak.status == 400, "AT-UC01-003: expected weak password to return 400");

    // AT-UC01-004: missing fields.
    auto missing = controller.Register({"", "charlie", "charlie@example.com", "Strong#123"});
    Expect(missing.status == 400, "AT-UC01-004: expected missing fields to return 400");

    // AT-UC01-005: invalid email format.
    auto bad_email = controller.Register({"Dan", "dan", "not-an-email", "Strong#123"});
    Expect(bad_email.status == 400, "AT-UC01-005: expected invalid email to return 400");

    // AT-UC01-006: case-insensitive duplicate email detection.
    auto case_dup = controller.Register({"Alice3", "alice3", "ALICE@EXAMPLE.COM", "Strong#123"});
    Expect(case_dup.status == 409, "AT-UC01-006: expected case-insensitive duplicate check");

    // AT-UC01-007: login after registration.
    Expect(auth.Login("alice@example.com", "Strong#123"), "AT-UC01-007: expected login to succeed");

    std::cout << "UC-01 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
