#include <string>

namespace {
int SignBucket(int value) {
  if (value < 0) {
    return -1;
  }
  return 1;
}
}  // namespace

int main() {
  // Exercise both branches of SignBucket without introducing extra control flow.
  return (SignBucket(-5) != -1) + (SignBucket(7) != 1);
}
