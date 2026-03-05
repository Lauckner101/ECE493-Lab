#include "api/review_controller.hpp"

namespace cms::api {

HttpResponse ReviewController::SubmitReview(const cms::services::ReviewSubmissionRequest& request) {
  auto result = review_service_->SubmitReview(request);
  using cms::services::ReviewSubmitStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case ReviewSubmitStatus::kSubmitted:
      return Created(result.message);
    case ReviewSubmitStatus::kMissingFields:
    case ReviewSubmitStatus::kInvalidData:
      return BadRequest(result.message);
    case ReviewSubmitStatus::kSystemError:
      return InternalServerError(result.message);
    case ReviewSubmitStatus::kDuplicate:
    case ReviewSubmitStatus::kNotAssigned:
      return Conflict(result.message);
  }
  return BadRequest("unknown review submission error");  // GCOVR_EXCL_BR_LINE
}

}  // namespace cms::api
