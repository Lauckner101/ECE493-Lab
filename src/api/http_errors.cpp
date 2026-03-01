#include "api/http_errors.hpp"

namespace cms::api {

HttpResponse Ok(const std::string& message, const std::string& redirect_to) {
  return {200, message, redirect_to};
}

HttpResponse BadRequest(const std::string& message) { return {400, message, ""}; }

HttpResponse Unauthorized(const std::string& message) { return {401, message, ""}; }

HttpResponse Conflict(const std::string& message) { return {409, message, ""}; }

HttpResponse InternalServerError(const std::string& message) { return {500, message, ""}; }

HttpResponse Created(const std::string& message, const std::string& redirect_to) {
  return {201, message, redirect_to};
}

}  // namespace cms::api
