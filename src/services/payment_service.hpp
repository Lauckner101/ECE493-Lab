#pragma once

#include <string>
#include <vector>

#include "models/registration.hpp"
#include "services/registration_service.hpp"

namespace cms::services {

struct PaymentRequest {
  std::string card_number;
  std::string expiry;
  std::string cvv;
  bool simulate_decline = false;
  bool simulate_record_failure = false;
};

enum class PaymentStatus {
  kPaid,
  kInvalidPaymentInfo,
  kDeclined,
  kRegistrationNotFound,
  kDuplicatePayment,
  kSystemError,
};

struct PaymentResult {
  PaymentStatus status;
  std::string message;
  int payment_id;
  int ticket_id;
};

class PaymentService {
 public:
  explicit PaymentService(RegistrationService* registration_service)
      : registration_service_(registration_service) {}

  PaymentResult ProcessPayment(int registration_id, const PaymentRequest& request);
  int ConfirmationCount() const;
  std::string LastConfirmationMessage() const;
  bool HasTicketForRegistration(int registration_id) const;

 private:
  static bool IsValidPaymentInfo(const PaymentRequest& request);
  static bool IsAllDigits(const std::string& value);

  RegistrationService* registration_service_;
  int next_payment_id_ = 1;
  int next_ticket_id_ = 1;
  int confirmation_count_ = 0;
  std::string last_confirmation_message_;
  std::vector<cms::models::PaymentRecord> payments_;
  std::vector<cms::models::Ticket> tickets_;
};

}  // namespace cms::services
