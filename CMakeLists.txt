cmake_minimum_required(VERSION 3.16)
project(cms LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(cms_core
  src/models/user_account.cpp
  src/models/conference.cpp
  src/models/paper_submission.cpp
  src/models/referee_assignment.cpp
  src/models/review.cpp
  src/models/decision.cpp
  src/models/schedule.cpp
  src/models/registration.cpp
  src/services/db.cpp
  src/lib/validation.cpp
  src/api/auth_middleware.cpp
  src/api/http_errors.cpp
  src/lib/logging.cpp
  src/services/notification_service.cpp
  src/services/user_repository.cpp
  src/services/session_service.cpp
  src/services/auth_service.cpp
  src/services/paper_service.cpp
  src/services/referee_service.cpp
  src/services/review_service.cpp
  src/services/decision_service.cpp
  src/services/schedule_service.cpp
  src/services/registration_service.cpp
  src/services/payment_service.cpp
  src/api/auth_controller.cpp
  src/api/paper_controller.cpp
  src/api/referee_controller.cpp
  src/api/review_controller.cpp
  src/api/decision_controller.cpp
  src/api/schedule_controller.cpp
  src/api/registration_controller.cpp
  src/api/routes.cpp
)

target_include_directories(cms_core PUBLIC src)

add_subdirectory(tests)
