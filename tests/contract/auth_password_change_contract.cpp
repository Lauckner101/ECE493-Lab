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

    auto reg = auth.Register({"Contract Pwd", "contractpwd", "contractpwd@example.com", "Valid#123"});
    Expect(reg.status == cms::services::RegisterStatus::kCreated,
           "precondition registration failed");

    auto login = controller.Login({"contractpwd", "Valid#123"});
    Expect(login.status == 200, "precondition login failed");
    auto token = ExtractSessionToken(login.body);

    // 200 changed
    auto ok = controller.ChangePassword(token, {"Valid#123", "NewValid#123", "NewValid#123"});
    Expect(ok.status == 200, "expected contract 200 for successful password change");

    // 400 validation failure
    auto bad = controller.ChangePassword(token, {"NewValid#123", "weak", "weak"});
    Expect(bad.status == 400, "expected contract 400 for weak password");

    // 401 invalid current password
    auto invalid = controller.ChangePassword(token, {"Wrong#123", "Other#123", "Other#123"});
    Expect(invalid.status == 401, "expected contract 401 for invalid current password");

    std::cout << "auth/password/change contract test passed\n";
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
