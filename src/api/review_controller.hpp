#pragma once

#include "api/http_errors.hpp"
#include "services/review_service.hpp"

namespace cms::api {

class ReviewController {
 public:
  explicit ReviewController(cms::services::ReviewService* review_service)
      : review_service_(review_service) {}

  HttpResponse SubmitReview(const cms::services::ReviewSubmissionRequest& request);

 private:
  cms::services::ReviewService* review_service_;
};

}  // namespace cms::api
