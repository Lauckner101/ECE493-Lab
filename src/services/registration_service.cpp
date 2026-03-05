#include "services/registration_service.hpp"

#include <algorithm>
#include <cctype>

#include "lib/validation.hpp"

namespace cms::services {

bool RegistrationService::IsValidAttendanceType(const std::string& attendance_type) {
  if (cms::lib::IsBlank(attendance_type)) {
    return false;
  }
  std::string normalized = attendance_type;
  std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return normalized == "student" || normalized == "regular" || normalized == "vip";
}

CreateRegistrationResult RegistrationService::CreateRegistration(
    int attendee_id, const CreateRegistrationRequest& request) {
  if (!request.registration_open) {
    return {CreateRegistrationStatus::kRegistrationClosed, "conference registration is closed", 0};
  }
  if (!IsValidAttendanceType(request.attendance_type)) {
    return {CreateRegistrationStatus::kInvalidAttendanceType, "invalid attendance type", 0};
  }

  auto existing = FindByAttendee(attendee_id);
  if (existing.has_value()) {
    return {CreateRegistrationStatus::kDuplicateRegistration, "registration already exists",
            existing->id};
  }

  cms::models::Registration reg{next_registration_id_++, attendee_id, request.attendance_type,
                                cms::models::RegistrationStatus::kPending};
  registrations_.push_back(reg);
  return {CreateRegistrationStatus::kCreated, "registration created", reg.id};
}

std::optional<cms::models::Registration> RegistrationService::FindById(int registration_id) const {
  for (const auto& registration : registrations_) {
    if (registration.id == registration_id) {
      return registration;
    }
  }
  return std::nullopt;
}

std::optional<cms::models::Registration> RegistrationService::FindByAttendee(int attendee_id) const {
  for (const auto& registration : registrations_) {
    if (registration.attendee_id == attendee_id) {
      return registration;
    }
  }
  return std::nullopt;
}

bool RegistrationService::MarkRegistrationPaid(int registration_id) {
  for (auto& registration : registrations_) {
    if (registration.id == registration_id) {
      registration.status = cms::models::RegistrationStatus::kPaid;
      return true;
    }
  }
  return false;
}

bool RegistrationService::MarkRegistrationFailed(int registration_id) {
  for (auto& registration : registrations_) {
    if (registration.id == registration_id) {
      registration.status = cms::models::RegistrationStatus::kFailed;
      return true;
    }
  }
  return false;
}

bool RegistrationService::IsRegistrationOwnedBy(int registration_id, int attendee_id) const {
  auto registration = FindById(registration_id);
  if (!registration.has_value()) {
    return false;
  }
  return registration->attendee_id == attendee_id;
}

bool RegistrationService::IsAttendeeRegisteredAndPaid(int attendee_id) const {
  auto registration = FindByAttendee(attendee_id);
  return registration.has_value() && registration->status == cms::models::RegistrationStatus::kPaid;  // GCOVR_EXCL_BR_LINE
}

int RegistrationService::RegistrationCount() const {
  return static_cast<int>(registrations_.size());
}

}  // namespace cms::services
