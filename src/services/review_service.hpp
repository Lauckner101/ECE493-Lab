#pragma once

#include <optional>
#include <string>
#include <vector>

#include "models/review.hpp"
#include "services/referee_service.hpp"

namespace cms::services {

struct ReviewSubmissionRequest {
  int paper_id;
  std::string referee_email;
  std::string review_text;
  std::string recommendation;
  bool simulate_failure = false;
};

enum class ReviewSubmitStatus {
  kSubmitted,
  kMissingFields,
  kInvalidData,
  kSystemError,
  kDuplicate,
  kNotAssigned,
};

struct ReviewSubmitResult {
  ReviewSubmitStatus status;
  std::string message;
  int review_id;
};

class ReviewService {
 public:
  explicit ReviewService(RefereeService* referee_service) : referee_service_(referee_service) {}

  ReviewSubmitResult SubmitReview(const ReviewSubmissionRequest& request);
  int CountReviewsForPaper(int paper_id) const;
  bool HasReviewForPaperByReferee(int paper_id, const std::string& referee_email) const;
  int EditorNotificationCount() const;
  std::vector<cms::models::Review> GetReviewsForPaper(int paper_id) const;

 private:
  static bool IsBlank(const std::string& value);
  static bool IsValidReviewData(const ReviewSubmissionRequest& request);
  static std::string NormalizeEmail(const std::string& email);

  RefereeService* referee_service_;
  int next_review_id_ = 1;
  int editor_notification_count_ = 0;
  std::vector<cms::models::Review> reviews_;
};

}  // namespace cms::services
