#pragma once

#include <optional>
#include <string>
#include <vector>

#include "models/decision.hpp"
#include "services/paper_service.hpp"
#include "services/review_service.hpp"

namespace cms::services {

struct DecisionRequest {
  int paper_id;
  std::string decision;  // accept|reject
  bool simulate_failure = false;
};

enum class DecisionStatus {
  kRecorded,
  kMissingDecision,
  kInsufficientReviews,
  kDuplicateDecision,
  kSystemError,
  kPaperNotFound,
};

struct DecisionResult {
  DecisionStatus status;
  std::string message;
  int decision_id;
};

class DecisionService {
 public:
  DecisionService(PaperService* paper_service, ReviewService* review_service)
      : paper_service_(paper_service), review_service_(review_service) {}

  DecisionResult RecordDecision(const DecisionRequest& request);
  std::optional<cms::models::Decision> GetDecisionForPaper(int paper_id) const;
  int AuthorNotificationCount() const;
  std::string LastAuthorNotification() const;

 private:
  static bool IsBlank(const std::string& value);

  PaperService* paper_service_;
  ReviewService* review_service_;
  int next_decision_id_ = 1;
  int author_notification_count_ = 0;
  std::string last_author_notification_;
  std::vector<cms::models::Decision> decisions_;
};

}  // namespace cms::services
