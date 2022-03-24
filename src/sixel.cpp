#include <stdexcept>
#include "term.hpp"

uint8_t select(uint8_t x, uint8_t n) {
  uint8_t q = x / n, r = x % n;
  return (q + (r < (n / 2))) * n;
}

void term::sixel_encode(
  av::VideoFrame frame, size_t width, size_t height, sixel_params params) {
  if (frame.pixelFormat() != AV_PIX_FMT_GRAY8) {
    throw std::invalid_argument("Requires grayscale image");
  }
}