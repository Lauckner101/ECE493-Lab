#include <iostream>
#include <string>

#include "api/auth_controller.hpp"
#include "services/auth_service.hpp"
#include "services/session_service.hpp"
#include "services/user_repository.hpp"
#include "test_assert.hpp"

int main() {
  cms::services::UserRepository repository;
  cms::services::AuthService auth(&repository);
  cms::services::SessionService sessions;
  cms::api::AuthController controller(&auth, &sessions);
  cms::api::AuthController no_session_controller(&auth);

  auto reg = auth.Register({"Integration User", "integrated", "integration@example.com", "Flow#2026"});
  Expect(reg.status == cms::services::RegisterStatus::kCreated,
         "expected user to be created before login flow");

  auto missing = controller.Login({"", ""});
  Expect(missing.status == 400, "expected bad request for missing login fields");

  auto invalid = controller.Login({"integrated", "Wrong#2026"});
  Expect(invalid.status == 401, "expected unauthorized for invalid credentials");

  auto login = controller.Login({"integrated", "Flow#2026"});
  Expect(login.status == 200, "expected login to succeed in integration flow");

  const std::string marker = "session=";
  const auto marker_pos = login.body.find(marker);
  Expect(marker_pos != std::string::npos, "expected session token after successful login");
  const std::string token = login.body.substr(marker_pos + marker.size());

  Expect(sessions.IsSessionActive(token), "expected token to be active in session store");

  auto protected_page = controller.ProtectedPage(token);
  Expect(protected_page.status == 200, "expected protected page access with active session");

  auto anonymous_protected_page = controller.ProtectedPage("invalid-session");
  Expect(anonymous_protected_page.status == 401,
         "expected protected page access denied without session");

  auto login_page_auth = controller.LoginPage(token);
  Expect(login_page_auth.status == 200 && login_page_auth.redirect_to == "/home",
         "expected authenticated user redirect from login page");

  auto login_page_guest = controller.LoginPage("invalid-session");
  Expect(login_page_guest.status == 200 && login_page_guest.body.find("login page") != std::string::npos,
         "expected guest login page content");

  auto stateless_login = no_session_controller.Login({"integrated", "Flow#2026"});
  Expect(stateless_login.status == 200 && stateless_login.body.find("session=") == std::string::npos,
         "expected successful login without session token when no session service provided");

  std::cout << "login flow integration test passed\n";
  return 0;
}
