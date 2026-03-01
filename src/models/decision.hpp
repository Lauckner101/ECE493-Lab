#pragma once

#include <string>

namespace cms::models {

struct Decision {
  int id;
  int paper_id;
  std::string decision;  // accept|reject
};

}  // namespace cms::models
