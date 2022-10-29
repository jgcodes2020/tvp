#include "dither.hpp"

namespace {
  consteval std::array<uint32_t, 256> init_xterm_table() {
    std::array<uint32_t, 256> res {
      // 8 base colours
      0x000000, 0x800000, 0x008000, 0x808000,
      0x000080, 0x800080, 0x008080, 0xC0C0C0,
      // 8 bright colours
      0x808080, 0xFF0000, 0x00FF00, 0xFFFF00,
      0x0000FF, 0xFF00FF, 0x00FFFF, 0xFFFFFF
    };
    
    // 6x6x6 colour cube
    for (int r = 0; r < 6; r++) {
      for (int g = 0; g < 6; g++) {
        for (int b = 0; b < 6; b++) {
          // 51 = 255 / 5, values range from 0-5 where 5 is maximum
          int rc = r * 51, gc = g * 51, bc = b * 51;
          
          res[36 * r + 6 * g + b + 16] = (rc << 16) + (gc << 8) + bc;
        }
      }
    }
    
    for (int i = 0; i < 24; i++) {
      int ic = 255 * i / 24;
      
      res[232 + i] = ic * 0x010101;
    }
    
    return res;
  }
}

namespace tvp {
  const std::array<uint32_t, 256> xterm_256 = init_xterm_table();
  
  uint32_t truncate_xterm(uint32_t x) {
    uint32_t r = x & 0xFF;
    uint32_t g = (x >> 8) & 0xFF;
    uint32_t b = (x >> 16) & 0xFF;
    
    
  }
}