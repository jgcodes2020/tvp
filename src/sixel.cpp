#include "term.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <memory>
#include <random>
#include <smmintrin.h>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <avcpp/frame.h>
#include <emmintrin.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <libdivide.h>
#include <nmmintrin.h>

#define NO_SIXEL 0

using namespace libdivide;

namespace {
uint8_t quantize(uint8_t x, uint8_t n) {
  uint8_t q = x / n, r = x % n;
  return (q + (r >= (n / 2)));
}
uint8_t quantize_ld(uint8_t x, const libdivide_u16_t &n, const uint16_t div) {
  uint16_t q = libdivide_u16_do(x, &n), r = x - (q * div);
  return (q + (r >= (div / 2)));
}

uint8_t adds_su8_si16(uint8_t x, int16_t y) {
  if (y < 0) {
    uint8_t res;
    bool k = __builtin_sub_overflow(x, uint8_t(-y), &res);
    return k ? 0 : res;
  } else {
    uint8_t res;
    bool k = __builtin_add_overflow(x, uint8_t(y), &res);
    return k ? 255 : res;
  }
}

std::array<uint8_t, 16> _cpp_spill_epu8(__m128i x) {
  std::array<uint8_t, 16> v;
  _mm_storeu_si128(reinterpret_cast<__m128i_u *>(v.data()), x);
  return v;
}
} // namespace

void term::sixel_encode(const av::VideoFrame &frame, sixel_params params) {
  if (frame.pixelFormat() != AV_PIX_FMT_GRAY8) {
    throw std::invalid_argument("Requires grayscale image");
  }
  // Useful constants
  const uint8_t pcol_div = 255 / params.ncols;
  const size_t width = frame.width();
  const size_t height = frame.height();
  const size_t frame_size = frame.size();

  libdivide_u16_t ncdiv = libdivide_u16_gen(pcol_div);
  // init framebuffer
  auto fb = std::make_unique<uint8_t[]>(frame_size);
  std::copy_n(frame.data(), frame.size(), fb.get());

  // DITHERING
  // =====================
  {
    size_t i = 0;
    for (size_t r = 0; r < frame.height(); r++) {
      for (size_t c = 0; c < frame.width(); c++, i++) {
        uint8_t oldv = fb[i];
        fb[i] = quantize(oldv, pcol_div);
        int16_t diff = int16_t(oldv) - int16_t(fb[i] * pcol_div);
        // Floyd-Steinberg matrix looks like this:
        // o x 7
        // 3 5 1

        // relevant indices
        const size_t in1 = i + 1;
        const size_t in2 = i + frame.width();
        const size_t in3 = in2 - 1;
        const size_t in4 = in2 + 1;

        // bounds checking
        const bool right = c + 1 < frame.width();
        const bool down = r + 1 < frame.height();
        const bool left = c > 0;

        if (right) {
          fb[in1] = adds_su8_si16(fb[in1], diff * 7 / 16);
        }
        if (down) {
          fb[in2] = adds_su8_si16(fb[in2], diff * 5 / 16);
          if (left) {
            fb[in3] = adds_su8_si16(fb[in3], diff * 3 / 16);
          }
          if (right) {
            fb[in4] = adds_su8_si16(fb[in4], diff * 1 / 16);
          }
        }
      }
    }
  }
  std::ofstream ofs("out.pgm");
  fmt::format_to(std::ostreambuf_iterator(ofs), "P5 {} {} 255\n", frame.width(),
                 frame.height());
  ofs.write(reinterpret_cast<const char *>(fb.get()), frame.size());

  // ENCODE SIXEL
  // =========

  // Encoding buffers
  std::vector<size_t> edge_idxs;
  std::vector<char> edge_vals;
  edge_idxs.reserve(width);
  edge_vals.reserve(width);
  decltype(auto) row_buf = std::make_unique<char[]>(width);

  // Sixel introducer
  std::fputs("\ePq", stdout);
  // Palette
  for (size_t i = 0; i <= params.ncols; i++) {
    fmt::print("#{0};2;{1};{1};{1}", i, (i * 100 / params.ncols));
  }
  // 6 rows = 1 sixel scanline
  for (size_t r = 0; r < height; r += 6) {
    // row pointers, set to null if OoB
    std::array<uint8_t *, 6> rps{
        (r + 0 < height) ? (&fb[(r + 0) * width]) : nullptr,
        (r + 1 < height) ? (&fb[(r + 1) * width]) : nullptr,
        (r + 2 < height) ? (&fb[(r + 2) * width]) : nullptr,
        (r + 3 < height) ? (&fb[(r + 3) * width]) : nullptr,
        (r + 4 < height) ? (&fb[(r + 4) * width]) : nullptr,
        (r + 5 < height) ? (&fb[(r + 5) * width]) : nullptr,
    };
    for (size_t pc = 0; pc <= params.ncols; pc++) {
      // Reset edge vectors
      edge_idxs.clear();
      edge_vals.clear();

      // Carry byte
      char carry = '\0';

      // SIMD scatter PC
      const __m128i pc_scatter = _mm_set1_epi8(pc);
      size_t c = 0;

      // SIMD loop until <16 bytes remaining
      for (c = 0; (c + 15) < width; c += 16) {
        __m128i chunk;
        // Encode sixels to chunk
        {
          // clang-format off
          
          // Load each row (zero if OoB)
          __m128i c0 = (rps[0])? 
            _mm_loadu_si128(reinterpret_cast<__m128i_u*>(rps[0] + c)) : 
            _mm_setzero_si128();
          __m128i c1 = (rps[1])? 
            _mm_loadu_si128(reinterpret_cast<__m128i_u*>(rps[1] + c)) : 
            _mm_setzero_si128();
          __m128i c2 = (rps[2])? 
            _mm_loadu_si128(reinterpret_cast<__m128i_u*>(rps[2] + c)) : 
            _mm_setzero_si128();
          __m128i c3 = (rps[3])? 
            _mm_loadu_si128(reinterpret_cast<__m128i_u*>(rps[3] + c)) : 
            _mm_setzero_si128();
          __m128i c4 = (rps[4])? 
            _mm_loadu_si128(reinterpret_cast<__m128i_u*>(rps[4] + c)) : 
            _mm_setzero_si128();
          __m128i c5 = (rps[5])? 
            _mm_loadu_si128(reinterpret_cast<__m128i_u*>(rps[5] + c)) : 
            _mm_setzero_si128();
          // clang-format on

          // Check against current palette colour
          c0 = _mm_cmpeq_epi8(c0, pc_scatter);
          c1 = _mm_cmpeq_epi8(c1, pc_scatter);
          c2 = _mm_cmpeq_epi8(c2, pc_scatter);
          c3 = _mm_cmpeq_epi8(c3, pc_scatter);
          c4 = _mm_cmpeq_epi8(c4, pc_scatter);
          c5 = _mm_cmpeq_epi8(c5, pc_scatter);

          // Mask corresponding bits
          c0 = _mm_and_si128(c0, _mm_set1_epi8(0b000001));
          c1 = _mm_and_si128(c1, _mm_set1_epi8(0b000010));
          c2 = _mm_and_si128(c2, _mm_set1_epi8(0b000100));
          c3 = _mm_and_si128(c3, _mm_set1_epi8(0b001000));
          c4 = _mm_and_si128(c4, _mm_set1_epi8(0b010000));
          c5 = _mm_and_si128(c5, _mm_set1_epi8(0b100000));

          // Combine isolated bits
          c0 = _mm_or_si128(c0, c1);
          c0 = _mm_or_si128(c0, c2);
          c3 = _mm_or_si128(c3, c4);
          c3 = _mm_or_si128(c3, c5);
          c0 = _mm_or_si128(c0, c3);

          // Add 0x3F to generate sixel
          chunk = _mm_add_epi8(c0, _mm_set1_epi8(0x3F));
        }
        // Detect and store edges
        {
          // Spill to memory for random access later
          std::array<char, 16> dump;
          _mm_storeu_si128(reinterpret_cast<__m128i_u *>(dump.data()), chunk);
          
          // Shift right 1 byte + carry
          __m128i check = _mm_bslli_si128(chunk, 1);
          check = _mm_insert_epi8(check, carry, 0);
          // Generate compare bitmask
          check = _mm_cmpeq_epi8(chunk, check);
          uint16_t edge_mask = ~_mm_movemask_epi8(check);
          
          // Extract bit positions
          while (edge_mask != 0) {
            uint16_t ctz = std::countr_zero(edge_mask);
            edge_idxs.push_back(c + ctz);
            edge_vals.push_back(dump[ctz]);
            edge_mask &= (edge_mask - 1);
          }
        }
      }
      // Scalar loop to cover last few bytes
      for (; c < width; c++) {
        char byte =
            uint8_t(rps[0][c] == pc) << 0 | uint8_t(rps[1][c] == pc) << 1 |
            uint8_t(rps[2][c] == pc) << 2 | uint8_t(rps[3][c] == pc) << 3 |
            uint8_t(rps[4][c] == pc) << 4 | uint8_t(rps[5][c] == pc) << 5;
        byte += 0x3F;

        if (byte != carry) {
          edge_idxs.push_back(c);
          edge_vals.push_back(byte);
        }
        carry = byte;
      }
      // Complete edge vector
      edge_idxs.push_back(width);
      if (edge_idxs.size() - 1 != edge_vals.size()) {
        throw std::runtime_error("Internal error: edge vectors don't match");
      }
      
      // Run length encoder
      char* buf_end = row_buf.get();
      for (size_t i = 0; i < edge_vals.size(); i++) {
        const size_t len = edge_idxs[i + 1] - edge_idxs[i];
        const char val = edge_vals[i];
        
        switch (len) {
        case 3:
          *buf_end++ = val;
        case 2:
          *buf_end++ = val;
        case 1:
          *buf_end++ = val;
        case 0:
          break;
        default:
          buf_end = fmt::format_to(buf_end, "!{}{}", len, val);
        }
      }
      // Line terminator char
      char lt = (pc == params.ncols)? '-' : '$';
      fmt::print("#{}{}{}", pc, std::string_view(row_buf.get(), buf_end), lt);
    }
  }
  std::fputs("\e\\", stdout);
  std::fflush(stdout);
}