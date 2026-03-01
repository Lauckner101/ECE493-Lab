#include "services/referee_service.hpp"

#include <algorithm>
#include <cctype>

#include "lib/validation.hpp"

namespace cms::services {
namespace {
constexpr int kMaxWorkloadPerReferee = 5;
constexpr int kRequiredRefereesPerPaper = 3;
}

std::string RefereeService::NormalizeEmail(const std::string& email) {
  std::string lowered = email;
  std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return lowered;
}

AssignRefereeResult RefereeService::AssignReferee(int paper_id, const std::string& referee_email) {
  auto paper = paper_service_->FindById(paper_id);
  if (!paper.has_value() || paper->status != cms::models::PaperStatus::kSubmitted) {
    return {AssignRefereeStatus::kPaperNotSubmitted, "paper must be submitted", 0};
  }

  if (!cms::lib::IsValidEmailFormat(referee_email)) {
    return {AssignRefereeStatus::kInvalidEmail, "invalid referee email", 0};
  }

  auto normalized_email = NormalizeEmail(referee_email);
  auto& paper_assignments = assignments_by_paper_[paper_id];
  if (static_cast<int>(paper_assignments.size()) >= kRequiredRefereesPerPaper) {
    return {AssignRefereeStatus::kPaperAlreadyFullyAssigned,
            "paper already has three assigned referees", 0};
  }

  for (const auto& assignment : paper_assignments) {
    if (assignment.referee_email == normalized_email) {
      return {AssignRefereeStatus::kDuplicateAssignment, "referee already assigned to paper", 0};
    }
  }

  int current_workload = workload_by_referee_[normalized_email];
  if (current_workload >= kMaxWorkloadPerReferee) {
    return {AssignRefereeStatus::kWorkloadExceeded, "referee workload limit exceeded", 0};
  }

  cms::models::RefereeAssignment assignment{next_assignment_id_++, paper_id, normalized_email,
                                            cms::models::RefereeAssignmentStatus::kInvited};
  paper_assignments.push_back(assignment);
  workload_by_referee_[normalized_email] = current_workload + 1;
  ++invitation_count_;

  if (static_cast<int>(paper_assignments.size()) == kRequiredRefereesPerPaper) {
    incomplete_papers_.erase(paper_id);
  }

  return {AssignRefereeStatus::kAssigned, "referee assigned", assignment.id};
}

int RefereeService::AssignRefereesBatch(int paper_id, const std::vector<std::string>& referee_emails) {
  int assigned = 0;
  for (const auto& email : referee_emails) {
    auto result = AssignReferee(paper_id, email);
    if (result.status == AssignRefereeStatus::kAssigned) {
      ++assigned;
      continue;
    }
    if (result.status == AssignRefereeStatus::kPaperAlreadyFullyAssigned) {
      break;
    }
  }
  return assigned;
}

void RefereeService::MarkIncompleteIfNeeded(int paper_id) {
  if (!IsPaperFullyAssigned(paper_id)) {
    incomplete_papers_.insert(paper_id);
    return;
  }
  incomplete_papers_.erase(paper_id);
}

InvitationResponseResult RefereeService::AcceptInvitation(int invitation_id) {
  for (auto& [paper_id, assignments] : assignments_by_paper_) {
    for (auto& assignment : assignments) {
      if (assignment.id != invitation_id) {
        continue;
      }

      if (assignment.status == cms::models::RefereeAssignmentStatus::kAccepted) {
        return {InvitationResponseStatus::kAlreadyAccepted, "invitation already accepted"};
      }
      if (assignment.status == cms::models::RefereeAssignmentStatus::kRejected) {
        return {InvitationResponseStatus::kAlreadyRejected, "invitation already rejected"};
      }

      if (CountAcceptedAssignmentsForPaper(paper_id) >= kRequiredRefereesPerPaper) {
        ++editor_notification_count_;
        return {InvitationResponseStatus::kPaperCapacityReached,
                "paper already has three confirmed referees"};
      }

      if (GetRefereeWorkload(assignment.referee_email) >= kMaxWorkloadPerReferee) {
        ++editor_notification_count_;
        return {InvitationResponseStatus::kRefereeWorkloadExceeded,
                "referee workload limit reached"};
      }

      assignment.status = cms::models::RefereeAssignmentStatus::kAccepted;
      ++editor_notification_count_;
      return {InvitationResponseStatus::kAccepted, "invitation accepted"};
    }
  }

  return {InvitationResponseStatus::kInvalidInvitation, "invalid or expired invitation"};
}

InvitationResponseResult RefereeService::RejectInvitation(int invitation_id) {
  for (auto& [paper_id, assignments] : assignments_by_paper_) {
    (void)paper_id;
    for (auto& assignment : assignments) {
      if (assignment.id != invitation_id) {
        continue;
      }

      if (assignment.status == cms::models::RefereeAssignmentStatus::kRejected) {
        return {InvitationResponseStatus::kAlreadyRejected, "invitation already rejected"};
      }
      if (assignment.status == cms::models::RefereeAssignmentStatus::kAccepted) {
        return {InvitationResponseStatus::kAlreadyAccepted, "invitation already accepted"};
      }

      assignment.status = cms::models::RefereeAssignmentStatus::kRejected;
      int current = workload_by_referee_[assignment.referee_email];
      if (current > 0) {
        workload_by_referee_[assignment.referee_email] = current - 1;
      }
      ++editor_notification_count_;
      return {InvitationResponseStatus::kRejected, "invitation rejected"};
    }
  }

  return {InvitationResponseStatus::kInvalidInvitation, "invalid or expired invitation"};
}

int RefereeService::CountAssignmentsForPaper(int paper_id) const {
  auto it = assignments_by_paper_.find(paper_id);
  if (it == assignments_by_paper_.end()) {
    return 0;
  }
  return static_cast<int>(it->second.size());
}

int RefereeService::CountAcceptedAssignmentsForPaper(int paper_id) const {
  auto it = assignments_by_paper_.find(paper_id);
  if (it == assignments_by_paper_.end()) {
    return 0;
  }
  int count = 0;
  for (const auto& assignment : it->second) {
    if (assignment.status == cms::models::RefereeAssignmentStatus::kAccepted) {
      ++count;
    }
  }
  return count;
}

int RefereeService::GetRefereeWorkload(const std::string& referee_email) const {
  auto normalized_email = NormalizeEmail(referee_email);
  auto it = workload_by_referee_.find(normalized_email);
  if (it == workload_by_referee_.end()) {
    return 0;
  }
  return it->second;
}

int RefereeService::CountAcceptedPapersForReferee(const std::string& referee_email) const {
  int count = 0;
  auto normalized = NormalizeEmail(referee_email);
  for (const auto& [paper_id, assignments] : assignments_by_paper_) {
    (void)paper_id;
    for (const auto& assignment : assignments) {
      if (assignment.referee_email == normalized &&
          assignment.status == cms::models::RefereeAssignmentStatus::kAccepted) {
        ++count;
      }
    }
  }
  return count;
}

bool RefereeService::IsPaperAssignedToReferee(int paper_id, const std::string& referee_email) const {
  auto it = assignments_by_paper_.find(paper_id);
  if (it == assignments_by_paper_.end()) {
    return false;
  }
  auto normalized = NormalizeEmail(referee_email);
  for (const auto& assignment : it->second) {
    if (assignment.referee_email == normalized &&
        assignment.status == cms::models::RefereeAssignmentStatus::kAccepted) {
      return true;
    }
  }
  return false;
}

int RefereeService::InvitationCount() const { return invitation_count_; }

int RefereeService::EditorNotificationCount() const { return editor_notification_count_; }

bool RefereeService::IsPaperFullyAssigned(int paper_id) const {
  return CountAssignmentsForPaper(paper_id) == kRequiredRefereesPerPaper;
}

bool RefereeService::IsPaperIncomplete(int paper_id) const { return incomplete_papers_.contains(paper_id); }

int RefereeService::CreateInvitationForTesting(int paper_id, const std::string& referee_email) {
  auto normalized = NormalizeEmail(referee_email);
  auto& paper_assignments = assignments_by_paper_[paper_id];
  cms::models::RefereeAssignment assignment{next_assignment_id_++, paper_id, normalized,
                                            cms::models::RefereeAssignmentStatus::kInvited};
  paper_assignments.push_back(assignment);
  return assignment.id;
}

void RefereeService::SetRefereeWorkloadForTesting(const std::string& referee_email, int workload) {
  workload_by_referee_[NormalizeEmail(referee_email)] = workload;
}

}  // namespace cms::services
