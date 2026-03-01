#include "services/payment_service.hpp"

namespace cms::services {

bool PaymentService::IsAllDigits(const std::string& value) {
  for (char c : value) {
    if (c < '0' || c > '9') {
      return false;
    }
  }
  return true;
}

bool PaymentService::IsValidPaymentInfo(const PaymentRequest& request) {
  if (request.card_number.size() != 16 || request.cvv.size() != 3 || request.expiry.size() != 5) {
    return false;
  }
  if (request.expiry[2] != '/') {
    return false;
  }
  return IsAllDigits(request.card_number) && IsAllDigits(request.cvv) &&
         IsAllDigits(request.expiry.substr(0, 2)) && IsAllDigits(request.expiry.substr(3, 2));
}

PaymentResult PaymentService::ProcessPayment(int registration_id, const PaymentRequest& request) {
  auto registration = registration_service_->FindById(registration_id);
  if (!registration.has_value()) {
    return {PaymentStatus::kRegistrationNotFound, "registration not found", 0, 0};
  }
  if (registration->status == cms::models::RegistrationStatus::kPaid) {
    return {PaymentStatus::kDuplicatePayment, "duplicate payment blocked", 0, 0};
  }
  if (!IsValidPaymentInfo(request)) {
    return {PaymentStatus::kInvalidPaymentInfo, "invalid payment information", 0, 0};
  }
  if (request.simulate_decline) {
    registration_service_->MarkRegistrationFailed(registration_id);
    payments_.push_back(
        {next_payment_id_++, registration_id, "failed"});
    return {PaymentStatus::kDeclined, "payment declined", 0, 0};
  }
  if (request.simulate_record_failure) {
    return {PaymentStatus::kSystemError, "registration could not be completed", 0, 0};
  }

  cms::models::PaymentRecord payment{next_payment_id_++, registration_id, "succeeded"};
  payments_.push_back(payment);
  registration_service_->MarkRegistrationPaid(registration_id);

  cms::models::Ticket ticket{next_ticket_id_++, registration_id,
                             "TICKET-" + std::to_string(registration_id) + "-" +
                                 std::to_string(next_ticket_id_)};
  tickets_.push_back(ticket);

  ++confirmation_count_;
  last_confirmation_message_ = "Registration confirmed for id=" + std::to_string(registration_id);
  return {PaymentStatus::kPaid, "payment processed", payment.id, ticket.id};
}

int PaymentService::ConfirmationCount() const { return confirmation_count_; }

std::string PaymentService::LastConfirmationMessage() const {
  return last_confirmation_message_;
}

bool PaymentService::HasTicketForRegistration(int registration_id) const {
  for (const auto& ticket : tickets_) {
    if (ticket.registration_id == registration_id) {
      return true;
    }
  }
  return false;
}

}  // namespace cms::services
