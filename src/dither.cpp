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
          
          res[36 * r + 6 * g + b + 16] = (rc << 16) + (gc << 8) + b;
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
    
    // gsc: grayscale colour
    // ccc: colour cube colour
    uint32_t gs_index, cc_index;
    float gsi;
    float ccr, ccg, ccb;
    
    // Generate the closest grayscale colour
    {
      // CIE Y component formula
      // Decimal weights are actual values. I've changed them
      // to fractions of 65536 for efficiency.
      // r: 13933 / 65536 ~ 0.2126
      // g: 46871 / 65536 ~ 0.7152
      // b: 4732  / 65536 ~ 0.0722
      uint32_t l = ((r * 13933) + (g * 46871) + (b * 4732)) / 65536;
      
      // simplified from (24 / 255).
      // the + 42 is to correct rounding.
      gs_index = (((l * 8) + 42) / 85) + 232;
      gsi = xterm_256[gs_index] & 0xFF;
    }
    
    // Generate the closest colour-cube color
    {
      // perform rounding divide by 51 (255 / 5)
      uint32_t rr = (r + 25) / 51;
      uint32_t gr = (g + 25) / 51;
      uint32_t br = (b + 25) / 51;
      
      ccr = rr * 51;
      ccg = gr * 51;
      ccb = br * 51;
      
      cc_index = 36 * rr + 6 * gr + br + 16;
    }
    
    // component distances for each colour
    float ccr_diff = (r - ccr);
    float ccg_diff = (g - ccg);
    float ccb_diff = (b - ccb);
    
    float gsr_diff = (r - gsi);
    float gsg_diff = (g - gsi);
    float gsb_diff = (b - gsi);
    
    // calculate distances squared
    float cc_distsq = ccr_diff*ccr_diff + ccg_diff*ccg_diff + ccb_diff*ccb_diff;
    float gs_distsq = gsr_diff*gsr_diff + gsg_diff*gsg_diff + gsb_diff*gsb_diff;
    
    // return the closer colour
    return (gs_distsq < cc_distsq)? gs_index : cc_index;
  }
}