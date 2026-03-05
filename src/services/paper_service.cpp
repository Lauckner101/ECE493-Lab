#include "services/paper_service.hpp"

#include <algorithm>
#include <cctype>

namespace cms::services {
namespace {
constexpr size_t kMaxManuscriptBytes = 10 * 1024 * 1024;
}

bool PaperService::IsBlank(const std::string& value) {
  for (char c : value) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  return true;
}

bool PaperService::IsPdf(const std::string& file_name) {
  if (file_name.size() < 4) {
    return false;
  }
  std::string suffix = file_name.substr(file_name.size() - 4);
  std::transform(suffix.begin(), suffix.end(), suffix.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return suffix == ".pdf";
}

bool PaperService::IsValidDraftData(const SaveDraftRequest& request) {
  if (!IsBlank(request.title) && request.title.find('@') != std::string::npos) {
    return false;
  }
  return true;
}

std::string PaperService::BuildSubmissionKey(int author_id, const SubmitPaperRequest& request) {
  std::string key = std::to_string(author_id) + "|" + request.title + "|" + request.abstract_text + "|" +
                    request.keywords + "|" + request.manuscript_file_name + "|" +
                    std::to_string(request.manuscript_file_size_bytes);
  std::transform(key.begin(), key.end(), key.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return key;
}

SubmitPaperResult PaperService::SubmitPaper(int author_id, const SubmitPaperRequest& request) {
  if (author_id <= 0 || IsBlank(request.title) || IsBlank(request.abstract_text) ||
      IsBlank(request.keywords)) {
    return {SubmitPaperStatus::kMissingMetadata, "missing required metadata", 0};
  }
  if (!IsPdf(request.manuscript_file_name)) {
    return {SubmitPaperStatus::kInvalidFileFormat, "unsupported file format", 0};
  }
  if (request.manuscript_file_size_bytes == 0 || request.manuscript_file_size_bytes > kMaxManuscriptBytes) {
    return {SubmitPaperStatus::kFileTooLarge, "manuscript exceeds 10 MB limit", 0};
  }

  std::string key = BuildSubmissionKey(author_id, request);
  if (submission_keys_.contains(key)) {
    return {SubmitPaperStatus::kDuplicateSubmission, "duplicate submission ignored",
            submission_keys_.at(key)};
  }

  cms::models::PaperSubmission paper{next_paper_id_++,
                                     author_id,
                                     request.title,
                                     request.abstract_text,
                                     request.keywords,
                                     request.manuscript_file_name,
                                     request.manuscript_file_size_bytes,
                                     cms::models::PaperStatus::kSubmitted};  // GCOVR_EXCL_BR_LINE
  papers_.push_back(paper);
  submission_keys_[key] = paper.id;
  return {SubmitPaperStatus::kSubmitted, "paper submitted", paper.id};
}

SaveDraftResult PaperService::SaveDraft(int author_id, const SaveDraftRequest& request) {
  if (author_id <= 0) {
    return {SaveDraftStatus::kSystemError, "authentication required", 0};
  }
  if (request.simulate_failure) {
    return {SaveDraftStatus::kSystemError, "draft save failed due to system error", 0};
  }
  if (!IsValidDraftData(request)) {
    return {SaveDraftStatus::kInvalidDraftData, "invalid draft information", 0};
  }
  if (IsBlank(request.title) && IsBlank(request.abstract_text) && IsBlank(request.keywords)) {
    return {SaveDraftStatus::kEmptyDraft, "empty draft cannot be saved", 0};
  }

  if (request.draft_id > 0) {
    for (auto& paper : papers_) {
      if (paper.id == request.draft_id && paper.author_id == author_id &&  // GCOVR_EXCL_BR_LINE
          paper.status == cms::models::PaperStatus::kDraft) {  // GCOVR_EXCL_BR_LINE
        paper.title = request.title;
        paper.abstract_text = request.abstract_text;
        paper.keywords = request.keywords;
        return {SaveDraftStatus::kSaved, "draft updated", paper.id};
      }
    }
    return {SaveDraftStatus::kDraftNotFound, "draft not found", 0};
  }

  cms::models::PaperSubmission draft{next_paper_id_++,
                                     author_id,
                                     request.title,
                                     request.abstract_text,
                                     request.keywords,
                                     "",
                                     0,
                                     cms::models::PaperStatus::kDraft};  // GCOVR_EXCL_BR_LINE
  papers_.push_back(draft);
  return {SaveDraftStatus::kSaved, "draft saved", draft.id};
}

SubmitPaperResult PaperService::SubmitDraft(int author_id, int draft_id,
                                            const std::string& manuscript_file_name,
                                            size_t manuscript_file_size_bytes) {
  if (!IsPdf(manuscript_file_name)) {
    return {SubmitPaperStatus::kInvalidFileFormat, "unsupported file format", 0};
  }
  if (manuscript_file_size_bytes == 0 || manuscript_file_size_bytes > kMaxManuscriptBytes) {
    return {SubmitPaperStatus::kFileTooLarge, "manuscript exceeds 10 MB limit", 0};
  }

  for (auto& paper : papers_) {
    if (paper.id == draft_id && paper.author_id == author_id && paper.status == cms::models::PaperStatus::kDraft) {
      if (IsBlank(paper.title) || IsBlank(paper.abstract_text) || IsBlank(paper.keywords)) {
        return {SubmitPaperStatus::kDraftIncomplete, "draft metadata incomplete", 0};
      }

      SubmitPaperRequest as_submit{paper.title, paper.abstract_text, paper.keywords, manuscript_file_name,  // GCOVR_EXCL_BR_LINE
                                   manuscript_file_size_bytes};  // GCOVR_EXCL_BR_LINE
      std::string key = BuildSubmissionKey(author_id, as_submit);
      if (submission_keys_.contains(key)) {
        return {SubmitPaperStatus::kDuplicateSubmission, "duplicate submission ignored",
                submission_keys_.at(key)};
      }

      paper.manuscript_file_name = manuscript_file_name;
      paper.manuscript_file_size_bytes = manuscript_file_size_bytes;
      paper.status = cms::models::PaperStatus::kSubmitted;
      submission_keys_[key] = paper.id;
      return {SubmitPaperStatus::kSubmitted, "draft submitted", paper.id};
    }
  }

  return {SubmitPaperStatus::kDraftNotFound, "draft not found", 0};
}

std::optional<cms::models::PaperSubmission> PaperService::FindById(int paper_id) const {
  for (const auto& paper : papers_) {
    if (paper.id == paper_id) {
      return paper;
    }
  }
  return std::nullopt;
}

bool PaperService::UpdatePaperStatus(int paper_id, cms::models::PaperStatus status) {
  for (auto& paper : papers_) {
    if (paper.id == paper_id) {
      paper.status = status;
      return true;
    }
  }
  return false;
}

int PaperService::SubmittedCount() const {
  int count = 0;
  for (const auto& paper : papers_) {
    if (paper.status == cms::models::PaperStatus::kSubmitted) {
      ++count;
    }
  }
  return count;
}

int PaperService::DraftCount() const {
  int count = 0;
  for (const auto& paper : papers_) {
    if (paper.status == cms::models::PaperStatus::kDraft) {
      ++count;
    }
  }
  return count;
}

int PaperService::AcceptedCount() const {
  int count = 0;
  for (const auto& paper : papers_) {
    if (paper.status == cms::models::PaperStatus::kAccepted) {
      ++count;
    }
  }
  return count;
}

std::vector<int> PaperService::GetAcceptedPaperIds() const {
  std::vector<int> ids;
  for (const auto& paper : papers_) {
    if (paper.status == cms::models::PaperStatus::kAccepted) {
      ids.push_back(paper.id);
    }
  }
  std::sort(ids.begin(), ids.end());
  return ids;
}

}  // namespace cms::services
