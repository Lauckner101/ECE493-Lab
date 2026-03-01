#pragma once

#include <string>

namespace cms::lib {

bool IsBlank(const std::string& value);
bool IsValidEmailFormat(const std::string& email);
bool IsPasswordComplex(const std::string& password);

}  // namespace cms::lib
