#pragma once

#include <string>

#include "api/http_errors.hpp"
#include "services/paper_service.hpp"
#include "services/session_service.hpp"

namespace cms::api {

class PaperController {
 public:
  explicit PaperController(cms::services::PaperService* paper_service,
                           cms::services::SessionService* session_service)
      : paper_service_(paper_service), session_service_(session_service) {}

  HttpResponse SubmitPaper(const std::string& session_token,
                           const cms::services::SubmitPaperRequest& request);
  HttpResponse SaveDraft(const std::string& session_token,
                         const cms::services::SaveDraftRequest& request);
  HttpResponse SubmitDraft(const std::string& session_token, int draft_id,
                           const std::string& manuscript_file_name,
                           size_t manuscript_file_size_bytes);

 private:
  cms::services::PaperService* paper_service_;
  cms::services::SessionService* session_service_;
};

}  // namespace cms::api
