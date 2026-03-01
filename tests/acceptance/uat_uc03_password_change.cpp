#include <iostream>
#include <string>

#include "api/auth_controller.hpp"
#include "services/auth_service.hpp"
#include "services/session_service.hpp"
#include "services/user_repository.hpp"
#include "test_assert.hpp"

using cms::api::AuthController;
using cms::services::AuthService;
using cms::services::ChangePasswordRequest;
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

    auto reg = auth.Register({"Pwd User", "pwduser", "pwd@example.com", "Start#123"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated,
           "precondition: registration failed");

    auto login = controller.Login({"pwduser", "Start#123"});
    Expect(login.status == 200, "precondition: login failed");
    std::string token = ExtractSessionToken(login.body);
    Expect(!token.empty(), "precondition: missing session token");

    // AT-UC03-001: successful password change.
    auto ok = controller.ChangePassword(token, {"Start#123", "NewPass#456", "NewPass#456"});
    Expect(ok.status == 200, "AT-UC03-001: expected 200");

    // AT-UC03-002: incorrect current password.
    auto wrong_current = controller.ChangePassword(token, {"Bad#123", "Other#789", "Other#789"});
    Expect(wrong_current.status == 401, "AT-UC03-002: expected 401");

    // AT-UC03-003: weak new password.
    auto weak = controller.ChangePassword(token, {"NewPass#456", "weak", "weak"});
    Expect(weak.status == 400, "AT-UC03-003: expected 400");

    // AT-UC03-004: confirmation mismatch.
    auto mismatch = controller.ChangePassword(token, {"NewPass#456", "Strong#789", "Strong#000"});
    Expect(mismatch.status == 400, "AT-UC03-004: expected 400");

    // AT-UC03-005: missing required fields.
    auto missing = controller.ChangePassword(token, {"", "Strong#789", "Strong#789"});
    Expect(missing.status == 400, "AT-UC03-005: expected 400");

    // AT-UC03-007: prevent reusing same password.
    auto reused = controller.ChangePassword(token, {"NewPass#456", "NewPass#456", "NewPass#456"});
    Expect(reused.status == 400, "AT-UC03-007: expected 400");

    // AT-UC03-006: old password invalid, new password valid.
    Expect(!auth.Login("pwduser", "Start#123"), "AT-UC03-006: old password should fail");
    Expect(auth.Login("pwduser", "NewPass#456"), "AT-UC03-006: new password should succeed");

    std::cout << "UC-03 acceptance tests passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
