#include "term.hpp"
#include <fmt/compile.h>
#include <fmt/core.h>
#include <opencv2/core/hal/interface.h>
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <memory>
#include <utility>
#include "fixed_string.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace {
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

  inline uint8_t extract(cv::Mat& img, size_t r, size_t c) {
    return (r >= img.rows || c >= img.cols) ? 0 : img.at<uint8_t>(r, c);
  }

  inline char encode(cv::Mat& img, uint8_t pc, size_t r, size_t c) {
    return ((uchar(extract(img, r + 0, c) == pc) << 0) |
            (uchar(extract(img, r + 1, c) == pc) << 1) |
            (uchar(extract(img, r + 2, c) == pc) << 2) |
            (uchar(extract(img, r + 3, c) == pc) << 3) |
            (uchar(extract(img, r + 4, c) == pc) << 4) |
            (uchar(extract(img, r + 5, c) == pc) << 5)) +
      0x3F;
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
}  // namespace

namespace term {

  void encode_sixel(cv::InputArray src) {
    using namespace std::literals;

    constexpr uint8_t pcols              = 17;
    constexpr mtap::fixed_string palette = make_palette_string<pcols>();

    cv::Mat img;
    cv::cvtColor(src, img, cv::COLOR_BGR2GRAY);
    img.forEach<uint8_t>(
      [](uint8_t& x, const int pos[]) { x = select<255 / pcols>(x); });

    // clang-format off
    fmt::print(FMT_COMPILE("\ePq{}"), std::string_view(palette));
    // clang-format on
    std::string buf;
    buf.reserve(img.cols);
    size_t len;

    for (size_t r = 0; r < img.rows; r += 6) {
      for (size_t pc = 0; pc <= pcols; ++pc) {
        char prev    = '\0';
        size_t count = 0;
        buf.clear();
        for (size_t c = 0; c < img.cols; ++c) {
          char curr = encode(img, pc, r, c);
          if (curr != prev) {
            if (count > 0) {
              if (count > 3) {
                fmt::format_to(std::back_inserter(buf), "!{}{}", count, prev);
              }
              else {
                buf.append(std::string(count, prev));
              }
            }
            count = 0;
            prev  = curr;
          }
          ++count;
        }
        if (count > 0) {
          if (count > 3) {
            fmt::format_to(std::back_inserter(buf), "!{}{}", count, prev);
          }
          else {
            buf.append(std::string(count, prev));
          }
        }
        fmt::print("#{}{}{}", pc, buf, (pc == pcols) ? '-' : '$');
      }
    }
    fmt::print("\e\\");
    std::fflush(stdout);
  }
}  // namespace term