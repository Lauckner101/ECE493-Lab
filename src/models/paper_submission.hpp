#pragma once

#include <string>

namespace cms::models {

enum class PaperStatus {
  kDraft,
  kSubmitted,
  kAccepted,
  kRejected,
};

struct PaperSubmission {
  int id;
  int author_id;
  std::string title;
  std::string abstract_text;
  std::string keywords;
  std::string manuscript_file_name;
  size_t manuscript_file_size_bytes;
  PaperStatus status;
};

}  // namespace cms::models
