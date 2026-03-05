#include "api/registration_controller.hpp"

namespace cms::api {

HttpResponse RegistrationController::CreateRegistration(
    const std::string& session_token,
    const cms::services::CreateRegistrationRequest& request) {
  auto user_id = session_service_->GetUserIdForSession(session_token);
  if (!user_id.has_value()) {
    return Unauthorized("authentication required");
  }

  auto result = registration_service_->CreateRegistration(user_id.value(), request);
  using cms::services::CreateRegistrationStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case CreateRegistrationStatus::kCreated:
      return Created(result.message);
    case CreateRegistrationStatus::kInvalidAttendanceType:
    case CreateRegistrationStatus::kRegistrationClosed:
      return BadRequest(result.message);
    case CreateRegistrationStatus::kDuplicateRegistration:
      return Conflict(result.message);
  }
  return BadRequest("unknown registration error");  // GCOVR_EXCL_BR_LINE
}

HttpResponse RegistrationController::PayForRegistration(
    const std::string& session_token, int registration_id,
    const cms::services::PaymentRequest& request) {
  auto user_id = session_service_->GetUserIdForSession(session_token);
  if (!user_id.has_value()) {
    return Unauthorized("authentication required");
  }
  if (!registration_service_->IsRegistrationOwnedBy(registration_id, user_id.value())) {
    return Unauthorized("registration not found for attendee");
  }

  auto result = payment_service_->ProcessPayment(registration_id, request);
  using cms::services::PaymentStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case PaymentStatus::kPaid:
      return Ok(result.message);
    case PaymentStatus::kInvalidPaymentInfo:
      return BadRequest(result.message);
    case PaymentStatus::kDeclined:
      return Conflict(result.message);
    case PaymentStatus::kRegistrationNotFound:  // GCOVR_EXCL_BR_LINE
      return BadRequest(result.message);  // GCOVR_EXCL_BR_LINE
    case PaymentStatus::kDuplicatePayment:
      return Conflict(result.message);
    case PaymentStatus::kSystemError:
      return InternalServerError(result.message);
  }
  return BadRequest("unknown payment error");  // GCOVR_EXCL_BR_LINE
}

HttpResponse RegistrationController::ViewMyRegistrationStatus(
    const std::string& session_token) const {
  auto user_id = session_service_->GetUserIdForSession(session_token);
  if (!user_id.has_value()) {
    return Unauthorized("authentication required");
  }

  auto registration = registration_service_->FindByAttendee(user_id.value());
  if (!registration.has_value()) {
    return BadRequest("no registration found");
  }

  std::string status = "pending";
  if (registration->status == cms::models::RegistrationStatus::kPaid) {
    status = "paid";
  } else if (registration->status == cms::models::RegistrationStatus::kFailed) {
    status = "failed";
  }
  return Ok("registration status: " + status);
}

}  // namespace cms::api
