#include <cstddef>
#include <cstdint>

#include <avcpp/frame.h>

namespace term {
  struct sixel_params {
    uint8_t ncols = 16;
  };

  void sixel_encode(const av::VideoFrame& buffer, sixel_params params = {});
  
  struct term_size {
    size_t width, height;
  };
  
  // Queries terminal size. If there is no terminal, return 80x24.
  term_size query_size();
}  // namespace term