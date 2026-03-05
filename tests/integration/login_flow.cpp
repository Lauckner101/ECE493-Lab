// GCOVR_EXCL_START
#include <iostream>
#include <string>

#include "api/auth_controller.hpp"
#include "services/auth_service.hpp"
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
    cms::services::UserRepository repository;
    cms::services::AuthService auth(&repository);
    cms::services::SessionService sessions;
    cms::api::AuthController controller(&auth, &sessions);

    auto reg = auth.Register({"Integration User", "integrated", "integration@example.com", "Flow#2026"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated,
           "expected user to be created before login flow");

    auto login = controller.Login({"integrated", "Flow#2026"});
    Expect(login.status == 200, "expected login to succeed in integration flow");

    std::string token = ExtractSessionToken(login.body);
    Expect(!token.empty(), "expected session token after successful login");
    Expect(sessions.IsSessionActive(token), "expected token to be active in session store");

    auto protected_page = controller.ProtectedPage(token);
    Expect(protected_page.status == 200, "expected protected page access with active session");

    auto anonymous_protected_page = controller.ProtectedPage("invalid-session");
    Expect(anonymous_protected_page.status == 401,
           "expected protected page access denied without session");

    std::cout << "login flow integration test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
// GCOVR_EXCL_STOP
