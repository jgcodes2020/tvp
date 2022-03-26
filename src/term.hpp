#include <cstddef>
#include <cstdint>

#include <avcpp/frame.h>

namespace term {
  struct sixel_params {
    uint8_t ncols = 16;
  };

  void sixel_encode(const av::VideoFrame& buffer, sixel_params params = {});
}  // namespace term