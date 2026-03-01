#include <iostream>
#include <string>

#include "api/auth_controller.hpp"
#include "services/auth_service.hpp"
#include "services/session_service.hpp"
#include "services/user_repository.hpp"
#include "test_assert.hpp"

using cms::api::AuthController;
using cms::services::AuthService;
using cms::services::LoginRequest;
using cms::services::RegisterRequest;
using cms::services::SessionService;
using cms::services::UserRepository;

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
    UserRepository repository;
    AuthService auth(&repository);
    SessionService sessions;
    AuthController controller(&auth, &sessions);

    auto reg = auth.Register({"Login User", "loginuser", "login@example.com", "Strong#123"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated,
           "precondition: account registration failed");

    // AT-UC02-001: successful login.
    auto ok = controller.Login({"loginuser", "Strong#123"});
    Expect(ok.status == 200, "AT-UC02-001: expected 200");
    Expect(ok.redirect_to == "/home", "AT-UC02-001: expected redirect to home");
    std::string active_token = ExtractSessionToken(ok.body);
    Expect(!active_token.empty(), "AT-UC02-001: expected active session token");

    // AT-UC02-002: username does not exist.
    auto missing_user = controller.Login({"ghost", "Strong#123"});
    Expect(missing_user.status == 401, "AT-UC02-002: expected 401");

    // AT-UC02-003: incorrect password.
    auto wrong_password = controller.Login({"loginuser", "Wrong#999"});
    Expect(wrong_password.status == 401, "AT-UC02-003: expected 401");

    // AT-UC02-004: missing required fields.
    auto missing_field = controller.Login({"", "Strong#123"});
    Expect(missing_field.status == 400, "AT-UC02-004: expected 400");

    // AT-UC02-005: session persists and protected page remains accessible.
    auto protected_page = controller.ProtectedPage(active_token);
    Expect(protected_page.status == 200, "AT-UC02-005: expected protected access with session");

    // AT-UC02-006: visiting login page while already logged in.
    auto login_page = controller.LoginPage(active_token);
    Expect(login_page.status == 200, "AT-UC02-006: expected login page request handled");
    Expect(login_page.redirect_to == "/home", "AT-UC02-006: expected redirect to home");

    // AT-UC02-007: case-insensitive username matching.
    auto case_match = controller.Login({"LOGINUSER", "Strong#123"});
    Expect(case_match.status == 200, "AT-UC02-007: expected case-insensitive login success");

    std::cout << "UC-02 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
