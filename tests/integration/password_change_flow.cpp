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

  auto reg = auth.Register({"Flow Pwd", "flowpwd", "flowpwd@example.com", "Flow#111"});
  Expect(reg.status == cms::services::RegisterStatus::kCreated,
         "expected user creation in integration flow");

  auto login = controller.Login({"flowpwd", "Flow#111"});
  Expect(login.status == 200, "expected login before password change");

  const std::string marker = "session=";
  const auto marker_pos = login.body.find(marker);
  Expect(marker_pos != std::string::npos, "expected active session token");
  const std::string token = login.body.substr(marker_pos + marker.size());

  auto unauthorized = controller.ChangePassword("bad-token", {"Flow#111", "Flow#222", "Flow#222"});
  Expect(unauthorized.status == 401, "expected unauthorized password change for invalid token");

  auto mismatch = controller.ChangePassword(token, {"Flow#111", "Flow#222", "Flow#333"});
  Expect(mismatch.status == 400, "expected confirmation mismatch validation failure");

  auto wrong_current = controller.ChangePassword(token, {"Wrong#111", "Flow#222", "Flow#222"});
  Expect(wrong_current.status == 401, "expected invalid current password rejection");

  auto changed = controller.ChangePassword(token, {"Flow#111", "Flow#222", "Flow#222"});
  Expect(changed.status == 200, "expected password change success");

  auto reuse = controller.ChangePassword(token, {"Flow#222", "Flow#222", "Flow#222"});
  Expect(reuse.status == 400, "expected password reuse rejection");

  Expect(!auth.Login("flowpwd", "Flow#111"), "expected old password rejection");
  Expect(auth.Login("flowpwd", "Flow#222"), "expected new password success");

  std::cout << "password change integration test passed\n";
  return 0;
}
