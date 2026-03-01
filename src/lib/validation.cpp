#include "lib/validation.hpp"

#include <cctype>
#include <regex>

namespace cms::lib {

bool IsBlank(const std::string& value) {
  for (char c : value) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  return true;
}

bool IsValidEmailFormat(const std::string& email) {
  static const std::regex kPattern(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
  return std::regex_match(email, kPattern);
}

bool IsPasswordComplex(const std::string& password) {
  if (password.size() < 8) {
    return false;
  }
  bool has_upper = false;
  bool has_lower = false;
  bool has_digit = false;
  bool has_symbol = false;
  for (char c : password) {
    if (std::isupper(static_cast<unsigned char>(c))) has_upper = true;
    if (std::islower(static_cast<unsigned char>(c))) has_lower = true;
    if (std::isdigit(static_cast<unsigned char>(c))) has_digit = true;
    if (!std::isalnum(static_cast<unsigned char>(c))) has_symbol = true;
  }
  return has_upper && has_lower && has_digit && has_symbol;
}

}  // namespace cms::lib
