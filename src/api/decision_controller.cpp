#include "api/decision_controller.hpp"

namespace cms::api {

HttpResponse DecisionController::RecordDecision(const cms::services::DecisionRequest& request) {
  auto result = decision_service_->RecordDecision(request);

  using cms::services::DecisionStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case DecisionStatus::kRecorded:
      return Created(result.message);
    case DecisionStatus::kMissingDecision:
    case DecisionStatus::kInsufficientReviews:
      return BadRequest(result.message);
    case DecisionStatus::kDuplicateDecision:
      return Conflict(result.message);
    case DecisionStatus::kSystemError:
      return InternalServerError(result.message);
    case DecisionStatus::kPaperNotFound:
      return BadRequest(result.message);
  }
  return BadRequest("unknown decision error");  // GCOVR_EXCL_BR_LINE
}

}  // namespace cms::api
