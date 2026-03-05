#include "services/decision_service.hpp"

#include <algorithm>
#include <cctype>

namespace cms::services {

bool DecisionService::IsBlank(const std::string& value) {
  for (char c : value) {
    if (!std::isspace(static_cast<unsigned char>(c))) {  // GCOVR_EXCL_BR_LINE
      return false;
    }
  }
  return true;
}

DecisionResult DecisionService::RecordDecision(const DecisionRequest& request) {
  auto paper = paper_service_->FindById(request.paper_id);
  if (!paper.has_value()) {
    return {DecisionStatus::kPaperNotFound, "paper not found", 0};
  }

  if (request.simulate_failure) {
    return {DecisionStatus::kSystemError, "decision could not be saved", 0};
  }

  std::string normalized = request.decision;
  std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  if (IsBlank(normalized) || (normalized != "accept" && normalized != "reject")) {
    return {DecisionStatus::kMissingDecision, "a valid decision must be selected", 0};
  }

  if (review_service_->CountReviewsForPaper(request.paper_id) < 3) {
    return {DecisionStatus::kInsufficientReviews,
            "three completed reviews are required before final decision", 0};
  }

  if (GetDecisionForPaper(request.paper_id).has_value()) {
    return {DecisionStatus::kDuplicateDecision, "final decision already recorded", 0};
  }

  cms::models::Decision decision{next_decision_id_++, request.paper_id, normalized};
  decisions_.push_back(decision);

  if (normalized == "accept") {
    paper_service_->UpdatePaperStatus(request.paper_id, cms::models::PaperStatus::kAccepted);
  } else {
    paper_service_->UpdatePaperStatus(request.paper_id, cms::models::PaperStatus::kRejected);
  }

  ++author_notification_count_;
  last_author_notification_ = "Paper " + std::to_string(request.paper_id) + " decision: " + normalized;
  return {DecisionStatus::kRecorded, "decision recorded", decision.id};
}

std::optional<cms::models::Decision> DecisionService::GetDecisionForPaper(int paper_id) const {
  for (const auto& decision : decisions_) {
    if (decision.paper_id == paper_id) {
      return decision;
    }
  }
  return std::nullopt;
}

int DecisionService::AuthorNotificationCount() const { return author_notification_count_; }

std::string DecisionService::LastAuthorNotification() const { return last_author_notification_; }

}  // namespace cms::services
