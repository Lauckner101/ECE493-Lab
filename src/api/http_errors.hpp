#pragma once

#include <string>

namespace cms::api {

struct HttpResponse {
  int status;
  std::string body;
  std::string redirect_to;
};

HttpResponse Ok(const std::string& message, const std::string& redirect_to = "");
HttpResponse BadRequest(const std::string& message);
HttpResponse Unauthorized(const std::string& message);
HttpResponse Conflict(const std::string& message);
HttpResponse InternalServerError(const std::string& message);
HttpResponse Created(const std::string& message, const std::string& redirect_to = "");

}  // namespace cms::api
