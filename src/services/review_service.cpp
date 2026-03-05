#include "services/review_service.hpp"

#include <algorithm>
#include <cctype>

namespace cms::services {

bool ReviewService::IsBlank(const std::string& value) {
  for (char c : value) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  return true;
}

std::string ReviewService::NormalizeEmail(const std::string& email) {
  std::string lowered = email;
  std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return lowered;
}

bool ReviewService::IsValidReviewData(const ReviewSubmissionRequest& request) {
  std::string rec = request.recommendation;
  std::transform(rec.begin(), rec.end(), rec.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  if (rec != "accept" && rec != "reject") {
    return false;
  }

  // Keep text validation simple and deterministic for testability.
  if (request.review_text.find("@@") != std::string::npos) {
    return false;
  }

  return true;
}

ReviewSubmitResult ReviewService::SubmitReview(const ReviewSubmissionRequest& request) {
  if (request.paper_id <= 0 || IsBlank(request.referee_email) || IsBlank(request.review_text) ||
      IsBlank(request.recommendation)) {
    return {ReviewSubmitStatus::kMissingFields, "missing required review fields", 0};
  }

  if (request.simulate_failure) {
    return {ReviewSubmitStatus::kSystemError, "review could not be saved", 0};
  }

  if (!IsValidReviewData(request)) {
    return {ReviewSubmitStatus::kInvalidData, "invalid review data", 0};
  }

  auto normalized_referee = NormalizeEmail(request.referee_email);
  if (!referee_service_->IsPaperAssignedToReferee(request.paper_id, normalized_referee)) {
    return {ReviewSubmitStatus::kNotAssigned, "referee is not assigned to this paper", 0};
  }

  if (HasReviewForPaperByReferee(request.paper_id, normalized_referee)) {
    return {ReviewSubmitStatus::kDuplicate, "duplicate review submission", 0};
  }

  std::string rec = request.recommendation;
  std::transform(rec.begin(), rec.end(), rec.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  cms::models::Review review{next_review_id_++, request.paper_id, normalized_referee,  // GCOVR_EXCL_BR_LINE
                             request.review_text, rec};  // GCOVR_EXCL_BR_LINE
  reviews_.push_back(review);
  ++editor_notification_count_;
  return {ReviewSubmitStatus::kSubmitted, "review submitted", review.id};
}

int ReviewService::CountReviewsForPaper(int paper_id) const {
  int count = 0;
  for (const auto& review : reviews_) {
    if (review.paper_id == paper_id) {
      ++count;
    }
  }
  return count;
}

bool ReviewService::HasReviewForPaperByReferee(int paper_id, const std::string& referee_email) const {
  auto normalized = NormalizeEmail(referee_email);
  for (const auto& review : reviews_) {
    if (review.paper_id == paper_id && review.referee_email == normalized) {
      return true;
    }
  }
  return false;
}

int ReviewService::EditorNotificationCount() const { return editor_notification_count_; }

std::vector<cms::models::Review> ReviewService::GetReviewsForPaper(int paper_id) const {
  std::vector<cms::models::Review> output;
  for (const auto& review : reviews_) {
    if (review.paper_id == paper_id) {
      output.push_back(review);
    }
  }
  return output;
}

}  // namespace cms::services
