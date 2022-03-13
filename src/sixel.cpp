#include <emmintrin.h>
#include <fmt/compile.h>
#include <fmt/core.h>

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <bit>
#include <cstdio>
#include <chrono>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <nmmintrin.h>
#include <pmmintrin.h>

#include "fixed_string.hpp"
#include "term.hpp"

namespace {
  template <uint8_t D>
  constexpr uint8_t part_size = 255 / D;
  template <uint8_t D>
  uint8_t select(uint8_t x) {
    uint8_t q = x / D, r = x % D;
    return q + (r >= (D / 2));
  }
  template <uint8_t D>
  uint8_t round(uint8_t x) {
    uint8_t q = x / D, r = x % D;
    return (q + (r >= (D / 2))) * D;
  }

  // Saturated ADD Unsigned with Signed
  uint8_t saddus(uint8_t a, int8_t b) {
    if (b < 0) {
      uint8_t sum = a + std::bit_cast<uint8_t>(b);
      return (sum > a) ? 0 : sum;
    }
    else {
      uint8_t sum = a + std::bit_cast<uint8_t>(b);
      return (sum < a) ? 255 : sum;
    }
  }

  constexpr size_t digit_cnt(size_t n) {
    size_t c = 0;
    do {
      n /= 10, c++;
    } while (n > 0);
    return c;
  }
  constexpr size_t palette_string_len(size_t n) {
    size_t total = 0;
    for (size_t i = 0; i <= n; i++) {
      // format for palette entry:
      // #{register #};2;{scale};{scale};{scale}
      total += (digit_cnt(i) + digit_cnt(100 * i / n) * 3 + 6);
    }
    return total;
  }

  template <size_t n>
  constexpr mtap::fixed_string<palette_string_len(n)> make_palette_string() {
    constexpr size_t len = palette_string_len(n);
    mtap::fixed_string<len> str;
    size_t ptr = 0;
    for (size_t i = 0; i <= n; i++) {
      str[ptr++]   = '#';
      size_t begin = ptr;
      size_t k     = i;
      do {
        str[ptr++] = (k % 10) + '0';
        k /= 10;
      } while (k > 0);
      std::reverse(&str[begin], &str[ptr]);
      str[ptr++] = ';';
      str[ptr++] = '2';
      str[ptr++] = ';';
      begin      = ptr;
      k          = (100 * i / n);
      do {
        str[ptr++] = (k % 10) + '0';
        k /= 10;
      } while (k > 0);
      std::reverse(&str[begin], &str[ptr]);
      str[ptr++] = ';';
      std::copy(&str[begin], &str[ptr], &str[ptr]);
      size_t len = ptr - begin;
      begin += len;
      ptr += len;
      std::copy(&str[begin], &str[ptr - 1], &str[ptr]);
      ptr += len - 1;
    }
    str[len] = '\0';
    return str;
  }
  
  // Takes 6 row pointers, an output buffer, a palette colour and a width.
  // Converts a cv::Mat row into sixel.
  void encode_scanline(
    std::string& buffer, const std::array<uint8_t*, 6>& rps, uint8_t pc, size_t width, bool rescan) {

    size_t i;
    
    // either 0x0000 or 0xFFFF.
    uint16_t rle_state = 0;
    
    const __m128i spread_pc = _mm_set1_epi8(pc);
    // SIMD encoder
    for (i = 0; i + 15 < width; i += 16) {
      // load 16 bytes from each row
      __m128i mask_r0 = rps[0]
        ? _mm_loadu_si128(reinterpret_cast<__m128i*>(rps[0] + i))
        : _mm_setzero_si128();
      __m128i mask_r1 = rps[1]
        ? _mm_loadu_si128(reinterpret_cast<__m128i*>(rps[1] + i))
        : _mm_setzero_si128();
      __m128i mask_r2 = rps[2]
        ? _mm_loadu_si128(reinterpret_cast<__m128i*>(rps[2] + i))
        : _mm_setzero_si128();
      __m128i mask_r3 = rps[3]
        ? _mm_loadu_si128(reinterpret_cast<__m128i*>(rps[3] + i))
        : _mm_setzero_si128();
      __m128i mask_r4 = rps[4]
        ? _mm_loadu_si128(reinterpret_cast<__m128i*>(rps[4] + i))
        : _mm_setzero_si128();
      __m128i mask_r5 = rps[5]
        ? _mm_loadu_si128(reinterpret_cast<__m128i*>(rps[5] + i))
        : _mm_setzero_si128();
      // create bitmask
      mask_r0 = _mm_cmpeq_epi8(mask_r0, spread_pc);
      mask_r1 = _mm_cmpeq_epi8(mask_r1, spread_pc);
      mask_r2 = _mm_cmpeq_epi8(mask_r2, spread_pc);
      mask_r3 = _mm_cmpeq_epi8(mask_r3, spread_pc);
      mask_r4 = _mm_cmpeq_epi8(mask_r4, spread_pc);
      mask_r5 = _mm_cmpeq_epi8(mask_r5, spread_pc);
      // mask each row
      mask_r0 = _mm_and_si128(mask_r0, _mm_set1_epi8(0x01));
      mask_r1 = _mm_and_si128(mask_r1, _mm_set1_epi8(0x02));
      mask_r2 = _mm_and_si128(mask_r2, _mm_set1_epi8(0x04));
      mask_r3 = _mm_and_si128(mask_r3, _mm_set1_epi8(0x08));
      mask_r4 = _mm_and_si128(mask_r4, _mm_set1_epi8(0x10));
      mask_r5 = _mm_and_si128(mask_r5, _mm_set1_epi8(0x20));
      // merge rows
      mask_r0 = _mm_or_si128(mask_r0, mask_r1);
      mask_r0 = _mm_or_si128(mask_r0, mask_r2);
      mask_r3 = _mm_or_si128(mask_r3, mask_r4);
      mask_r3 = _mm_or_si128(mask_r3, mask_r5);
      mask_r0 = _mm_or_si128(mask_r0, mask_r3);
      // Add 0x3F ('?')
      mask_r0 = _mm_add_epi8(mask_r0, _mm_set1_epi8(0x3F));
      // Store into buffer
      _mm_storeu_si128(reinterpret_cast<__m128i_u*>(&buffer[i]), mask_r0);
    }
    // Scalar encode for last chunk
    for (; i < width; ++i) {
      uint8_t flag_r0 = rps[0]? (uint8_t(rps[0][i] == pc) << 0) : 0;
      uint8_t flag_r1 = rps[1]? (uint8_t(rps[1][i] == pc) << 0) : 0;
      uint8_t flag_r2 = rps[2]? (uint8_t(rps[2][i] == pc) << 0) : 0;
      uint8_t flag_r3 = rps[3]? (uint8_t(rps[3][i] == pc) << 0) : 0;
      uint8_t flag_r4 = rps[4]? (uint8_t(rps[4][i] == pc) << 0) : 0;
      uint8_t flag_r5 = rps[5]? (uint8_t(rps[5][i] == pc) << 0) : 0;
      
      flag_r0 |= flag_r1 | flag_r2;
      flag_r3 |= flag_r4 | flag_r5;
      flag_r0 |= flag_r3;
      
      buffer[i] = flag_r0 + 0x3F;
    }
    fmt::print("#{}{}{}", pc, buffer, rescan? '$' : '-');
  }
  
  using time_pt = std::chrono::high_resolution_clock::time_point;
  using time_dur = std::chrono::high_resolution_clock::duration;
  using clock = std::chrono::high_resolution_clock;
}  // namespace

namespace term {
  time_dur process_time;  
  time_dur encode_time;  

  void encode_sixel(cv::InputArray src) {
    using namespace std::literals;

    // Number of palette colours.
    constexpr uint8_t pcols = 17;
    // Rounding distance for palette colours.
    constexpr uint8_t pdist              = 255 / pcols;
    constexpr mtap::fixed_string palette = make_palette_string<pcols>();
    
    // PALETTIZE IMAGE
    // ===================
    time_pt t0 = clock::now();
    
    // convert to grayscale
    cv::Mat img;
    cv::cvtColor(src, img, cv::COLOR_BGR2GRAY);
    // Perform Floyd-Steinberg dithering
    for (size_t r = 0; r < img.rows; ++r) {
      uint8_t* rp0 = img.ptr<uint8_t>(r);
      uint8_t* rp1 = img.ptr<uint8_t>(r + 1);
      for (size_t c = 0; c < img.cols; ++c) {
        uint8_t oldv = img.at<uint8_t>(r, c);
        int16_t err  = int16_t(oldv) - int16_t(round<255 / pcols>(oldv));
        img.at<uint8_t>(r, c) = select<255 / pcols>(oldv);

        if (c + 1 < img.cols) {
          rp0[c + 1] = saddus(rp0[c + 1], err * 7 / 16);
        }
        if (r + 1 < img.rows) {
          rp1[c] = saddus(rp1[c], err * 5 / 16);
          if (c > 0) {
            rp1[c - 1] = saddus(rp1[c - 1], err * 3 / 16);
          }
          if (c + 1 < img.cols) {
            rp1[c + 1] = saddus(rp1[c + 1], err * 1 / 16);
          }
        }
      }
    }
    process_time = clock::now() - t0;

    // ENCODE SIXELS
    // ==================

    // Sixel header - set palette
    t0 = clock::now();
    fmt::print(FMT_COMPILE("\ePq{}"), std::string_view(palette));

    // setup row buffer
    std::string buf(img.cols, '\0');

    for (size_t r = 0; r < img.rows; r += 6) {
      // cache 6 row pointers
      std::array<uint8_t*, 6> rps = {
        img.ptr<uint8_t>(r),     
        (r + 1 < img.rows)? img.ptr<uint8_t>(r + 1) : nullptr,
        (r + 2 < img.rows)? img.ptr<uint8_t>(r + 2) : nullptr,
        (r + 3 < img.rows)? img.ptr<uint8_t>(r + 3) : nullptr,
        (r + 4 < img.rows)? img.ptr<uint8_t>(r + 4) : nullptr,
        (r + 5 < img.rows)? img.ptr<uint8_t>(r + 5) : nullptr,
      };
      for (size_t pc = 0; pc <= pcols; ++pc) {
        encode_scanline(buf, rps, pc, img.cols, pc != pcols);
      }
    }
    fmt::print("\e\\");
    std::fflush(stdout);
    encode_time = clock::now() - t0;
  }
}  // namespace term