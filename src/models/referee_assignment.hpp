#pragma once

#include <string>

namespace cms::models {

enum class RefereeAssignmentStatus {
  kInvited,
  kAccepted,
  kRejected,
};

struct RefereeAssignment {
  int id;
  int paper_id;
  std::string referee_email;
  RefereeAssignmentStatus status;
};

}  // namespace cms::models
