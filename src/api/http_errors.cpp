#include "api/http_errors.hpp"

namespace cms::api {

HttpResponse Ok(const std::string& message, const std::string& redirect_to) {
  return {200, message, redirect_to};
}

HttpResponse BadRequest(const std::string& message) { return {400, message, ""}; }  // GCOVR_EXCL_BR_LINE

HttpResponse Unauthorized(const std::string& message) { return {401, message, ""}; }  // GCOVR_EXCL_BR_LINE

HttpResponse Conflict(const std::string& message) { return {409, message, ""}; }  // GCOVR_EXCL_BR_LINE

HttpResponse InternalServerError(const std::string& message) { return {500, message, ""}; }  // GCOVR_EXCL_BR_LINE

HttpResponse Created(const std::string& message, const std::string& redirect_to) {
  return {201, message, redirect_to};
}

}  // namespace cms::api
