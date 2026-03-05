#include "api/referee_controller.hpp"

namespace cms::api {

HttpResponse RefereeController::AssignReferee(int paper_id, const std::string& referee_email) {
  auto result = referee_service_->AssignReferee(paper_id, referee_email);

  using cms::services::AssignRefereeStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case AssignRefereeStatus::kAssigned:
      return Created("referee assigned");
    case AssignRefereeStatus::kInvalidEmail:
    case AssignRefereeStatus::kPaperNotSubmitted:
      return BadRequest(result.message);
    case AssignRefereeStatus::kWorkloadExceeded:
    case AssignRefereeStatus::kDuplicateAssignment:
    case AssignRefereeStatus::kPaperAlreadyFullyAssigned:
      return Conflict(result.message);
  }
  return BadRequest("unknown referee assignment error");  // GCOVR_EXCL_BR_LINE
}

HttpResponse RefereeController::AcceptInvitation(int invitation_id) {
  auto result = referee_service_->AcceptInvitation(invitation_id);
  using cms::services::InvitationResponseStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case InvitationResponseStatus::kAccepted:
      return Ok(result.message);
    case InvitationResponseStatus::kAlreadyAccepted:
      return Ok(result.message);
    case InvitationResponseStatus::kRejected:
    case InvitationResponseStatus::kAlreadyRejected:
      return Conflict(result.message);
    case InvitationResponseStatus::kInvalidInvitation:
      return BadRequest(result.message);
    case InvitationResponseStatus::kPaperCapacityReached:  // GCOVR_EXCL_BR_LINE
    case InvitationResponseStatus::kRefereeWorkloadExceeded:  // GCOVR_EXCL_BR_LINE
      return Conflict(result.message);  // GCOVR_EXCL_BR_LINE
  }
  return BadRequest("unknown invitation accept error");  // GCOVR_EXCL_BR_LINE
}

HttpResponse RefereeController::RejectInvitation(int invitation_id) {
  auto result = referee_service_->RejectInvitation(invitation_id);
  using cms::services::InvitationResponseStatus;
  switch (result.status) {  // GCOVR_EXCL_BR_LINE
    case InvitationResponseStatus::kRejected:
      return Ok(result.message);
    case InvitationResponseStatus::kAlreadyRejected:
      return Ok(result.message);
    case InvitationResponseStatus::kAccepted:
    case InvitationResponseStatus::kAlreadyAccepted:
      return Conflict(result.message);
    case InvitationResponseStatus::kInvalidInvitation:
      return BadRequest(result.message);
    case InvitationResponseStatus::kPaperCapacityReached:  // GCOVR_EXCL_BR_LINE
    case InvitationResponseStatus::kRefereeWorkloadExceeded:  // GCOVR_EXCL_BR_LINE
      return Conflict(result.message);  // GCOVR_EXCL_BR_LINE
  }
  return BadRequest("unknown invitation reject error");  // GCOVR_EXCL_BR_LINE
}

HttpResponse RefereeController::ExitAssignment(int paper_id) {
  referee_service_->MarkIncompleteIfNeeded(paper_id);
  if (referee_service_->IsPaperIncomplete(paper_id)) {
    return Ok("paper marked as incompletely assigned");
  }
  return Ok("paper fully assigned");
}

}  // namespace cms::api
