#include "api/paper_controller.hpp"

namespace cms::api {

HttpResponse PaperController::SubmitPaper(const std::string& session_token,
                                          const cms::services::SubmitPaperRequest& request) {
  auto user_id = session_service_->GetUserIdForSession(session_token);
  if (!user_id.has_value()) {
    return Unauthorized("authentication required");
  }

  auto result = paper_service_->SubmitPaper(user_id.value(), request);
  using cms::services::SubmitPaperStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case SubmitPaperStatus::kSubmitted:
      return Created("paper submitted");
    case SubmitPaperStatus::kDuplicateSubmission:
      return Ok("duplicate prevented");
    case SubmitPaperStatus::kMissingMetadata:
    case SubmitPaperStatus::kInvalidFileFormat:
    case SubmitPaperStatus::kFileTooLarge:
    case SubmitPaperStatus::kDraftIncomplete:
      return BadRequest(result.message);
    case SubmitPaperStatus::kDraftNotFound:  // GCOVR_EXCL_BR_LINE
      return Unauthorized(result.message);  // GCOVR_EXCL_BR_LINE
  }
  return BadRequest("unknown paper submission error");  // GCOVR_EXCL_BR_LINE
}

HttpResponse PaperController::SaveDraft(const std::string& session_token,
                                        const cms::services::SaveDraftRequest& request) {
  auto user_id = session_service_->GetUserIdForSession(session_token);
  if (!user_id.has_value()) {
    return Unauthorized("authentication required");
  }

  auto result = paper_service_->SaveDraft(user_id.value(), request);
  using cms::services::SaveDraftStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case SaveDraftStatus::kSaved:
      return Created("draft saved");
    case SaveDraftStatus::kEmptyDraft:
    case SaveDraftStatus::kInvalidDraftData:
      return BadRequest(result.message);
    case SaveDraftStatus::kDraftNotFound:
      return Unauthorized(result.message);
    case SaveDraftStatus::kSystemError:
      return InternalServerError(result.message);
  }
  return BadRequest("unknown draft save error");  // GCOVR_EXCL_BR_LINE
}

HttpResponse PaperController::SubmitDraft(const std::string& session_token, int draft_id,
                                          const std::string& manuscript_file_name,
                                          size_t manuscript_file_size_bytes) {
  auto user_id = session_service_->GetUserIdForSession(session_token);
  if (!user_id.has_value()) {
    return Unauthorized("authentication required");
  }

  auto result = paper_service_->SubmitDraft(user_id.value(), draft_id, manuscript_file_name,
                                            manuscript_file_size_bytes);
  using cms::services::SubmitPaperStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case SubmitPaperStatus::kSubmitted:
      return Ok("draft submitted");
    case SubmitPaperStatus::kDuplicateSubmission:
      return Ok("duplicate prevented");
    case SubmitPaperStatus::kMissingMetadata:
    case SubmitPaperStatus::kInvalidFileFormat:
    case SubmitPaperStatus::kFileTooLarge:
    case SubmitPaperStatus::kDraftIncomplete:
      return BadRequest(result.message);
    case SubmitPaperStatus::kDraftNotFound:
      return Unauthorized(result.message);
  }
  return BadRequest("unknown draft submit error");  // GCOVR_EXCL_BR_LINE
}

}  // namespace cms::api
