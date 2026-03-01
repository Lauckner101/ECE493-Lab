#pragma once

#include <string>

namespace cms::models {

enum class RegistrationStatus {
  kPending,
  kPaid,
  kFailed,
};

struct Registration {
  int id;
  int attendee_id;
  std::string attendance_type;
  RegistrationStatus status;
};

struct PaymentRecord {
  int id;
  int registration_id;
  std::string status;  // succeeded|failed
};

struct Ticket {
  int id;
  int registration_id;
  std::string ticket_code;
};

}  // namespace cms::models
