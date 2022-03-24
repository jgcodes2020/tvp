#include <cstddef>
#include <cstdint>

#include <avcpp/frame.h>

namespace term {
  struct sixel_params {
    uint8_t palette;
  };

  void sixel_encode(
    av::VideoFrame buffer, size_t width, size_t height,
    sixel_params params = {});
}  // namespace term