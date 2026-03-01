#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "models/paper_submission.hpp"

namespace cms::services {

struct SubmitPaperRequest {
  std::string title;
  std::string abstract_text;
  std::string keywords;
  std::string manuscript_file_name;
  size_t manuscript_file_size_bytes;
};

struct SaveDraftRequest {
  std::string title;
  std::string abstract_text;
  std::string keywords;
  int draft_id = 0;
  bool simulate_failure = false;
};

enum class SubmitPaperStatus {
  kSubmitted,
  kMissingMetadata,
  kInvalidFileFormat,
  kFileTooLarge,
  kDuplicateSubmission,
  kDraftNotFound,
  kDraftIncomplete,
};

enum class SaveDraftStatus {
  kSaved,
  kEmptyDraft,
  kInvalidDraftData,
  kDraftNotFound,
  kSystemError,
};

struct SubmitPaperResult {
  SubmitPaperStatus status;
  std::string message;
  int paper_id;
};

struct SaveDraftResult {
  SaveDraftStatus status;
  std::string message;
  int draft_id;
};

class PaperService {
 public:
  SubmitPaperResult SubmitPaper(int author_id, const SubmitPaperRequest& request);
  SaveDraftResult SaveDraft(int author_id, const SaveDraftRequest& request);
  SubmitPaperResult SubmitDraft(int author_id, int draft_id, const std::string& manuscript_file_name,
                                size_t manuscript_file_size_bytes);

  std::optional<cms::models::PaperSubmission> FindById(int paper_id) const;
  bool UpdatePaperStatus(int paper_id, cms::models::PaperStatus status);
  int SubmittedCount() const;
  int DraftCount() const;
  int AcceptedCount() const;
  std::vector<int> GetAcceptedPaperIds() const;

 private:
  static bool IsBlank(const std::string& value);
  static bool IsPdf(const std::string& file_name);
  static bool IsValidDraftData(const SaveDraftRequest& request);
  static std::string BuildSubmissionKey(int author_id, const SubmitPaperRequest& request);

  int next_paper_id_ = 1;
  std::vector<cms::models::PaperSubmission> papers_;
  std::unordered_map<std::string, int> submission_keys_;
};

}  // namespace cms::services
