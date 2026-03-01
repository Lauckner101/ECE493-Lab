#pragma once

#include <string>

namespace cms::models {

struct Review {
  int id;
  int paper_id;
  std::string referee_email;
  std::string review_text;
  std::string recommendation;
};

}  // namespace cms::models
