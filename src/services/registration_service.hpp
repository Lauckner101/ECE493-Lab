#pragma once

#include <optional>
#include <string>
#include <vector>

#include "models/registration.hpp"

namespace cms::services {

struct CreateRegistrationRequest {
  std::string attendance_type;
  bool registration_open = true;
};

enum class CreateRegistrationStatus {
  kCreated,
  kInvalidAttendanceType,
  kRegistrationClosed,
  kDuplicateRegistration,
};

struct CreateRegistrationResult {
  CreateRegistrationStatus status;
  std::string message;
  int registration_id;
};

class RegistrationService {
 public:
  CreateRegistrationResult CreateRegistration(int attendee_id,
                                              const CreateRegistrationRequest& request);
  std::optional<cms::models::Registration> FindById(int registration_id) const;
  std::optional<cms::models::Registration> FindByAttendee(int attendee_id) const;
  bool MarkRegistrationPaid(int registration_id);
  bool MarkRegistrationFailed(int registration_id);
  bool IsRegistrationOwnedBy(int registration_id, int attendee_id) const;
  bool IsAttendeeRegisteredAndPaid(int attendee_id) const;
  int RegistrationCount() const;

 private:
  static bool IsValidAttendanceType(const std::string& attendance_type);

  int next_registration_id_ = 1;
  std::vector<cms::models::Registration> registrations_;
};

}  // namespace cms::services
