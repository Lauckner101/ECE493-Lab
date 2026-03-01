#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "models/referee_assignment.hpp"
#include "services/paper_service.hpp"

namespace cms::services {

enum class AssignRefereeStatus {
  kAssigned,
  kPaperNotSubmitted,
  kInvalidEmail,
  kWorkloadExceeded,
  kDuplicateAssignment,
  kPaperAlreadyFullyAssigned,
};

struct AssignRefereeResult {
  AssignRefereeStatus status;
  std::string message;
  int assignment_id;
};

enum class InvitationResponseStatus {
  kAccepted,
  kRejected,
  kInvalidInvitation,
  kAlreadyAccepted,
  kAlreadyRejected,
  kPaperCapacityReached,
  kRefereeWorkloadExceeded,
};

struct InvitationResponseResult {
  InvitationResponseStatus status;
  std::string message;
};

class RefereeService {
 public:
  explicit RefereeService(PaperService* paper_service) : paper_service_(paper_service) {}

  AssignRefereeResult AssignReferee(int paper_id, const std::string& referee_email);
  int AssignRefereesBatch(int paper_id, const std::vector<std::string>& referee_emails);
  void MarkIncompleteIfNeeded(int paper_id);

  InvitationResponseResult AcceptInvitation(int invitation_id);
  InvitationResponseResult RejectInvitation(int invitation_id);

  int CountAssignmentsForPaper(int paper_id) const;
  int CountAcceptedAssignmentsForPaper(int paper_id) const;
  int GetRefereeWorkload(const std::string& referee_email) const;
  int CountAcceptedPapersForReferee(const std::string& referee_email) const;
  bool IsPaperAssignedToReferee(int paper_id, const std::string& referee_email) const;
  int InvitationCount() const;
  int EditorNotificationCount() const;
  bool IsPaperFullyAssigned(int paper_id) const;
  bool IsPaperIncomplete(int paper_id) const;

  // Test helper hooks used by acceptance tests for edge conditions.
  int CreateInvitationForTesting(int paper_id, const std::string& referee_email);
  void SetRefereeWorkloadForTesting(const std::string& referee_email, int workload);

 private:
  static std::string NormalizeEmail(const std::string& email);

  PaperService* paper_service_;
  int next_assignment_id_ = 1;
  int invitation_count_ = 0;
  int editor_notification_count_ = 0;
  std::unordered_map<int, std::vector<cms::models::RefereeAssignment>> assignments_by_paper_;
  std::unordered_map<std::string, int> workload_by_referee_;
  std::unordered_set<int> incomplete_papers_;
};

}  // namespace cms::services
