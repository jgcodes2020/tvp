#include "term.hpp"
#include <fmt/core.h>
#include <cstdio>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

template <uint8_t D>
uint8_t select(uint8_t x) {
  uint8_t q = x / D, r = x % D;
  return q + (r >= (D/2));
}

template <uint8_t D>
uint8_t round(uint8_t x) {
  uint8_t q = x / D, r = x % D;
  return (q + (r >= (D/2))) * D;
}

uint8_t extract(cv::Mat& img, size_t r, size_t c) {
  return (r >= img.rows || c >= img.cols)? 0 : img.at<uint8_t>(r, c);
}

namespace term {
  
  void sixelize_bw(cv::InputArray src) {
    cv::Mat img;
    cv::cvtColor(src, img, cv::COLOR_BGR2GRAY);

    cv::threshold(img, img, 128, 255, cv::THRESH_BINARY);

    fmt::print("\ePq#0;2;0;0;0#1;2;100;100;100;");
    std::string buffer(img.cols, '\0');
    for (size_t i = 0; i < img.rows; i += 6) {
      // print WHITE
      for (size_t j = 0; j < img.cols; ++j) {
        uint8_t sixel_bits = (uchar(img.at<uchar>(i + 0, j) != 0) << 0) |
          (uchar(img.at<uchar>(i + 1, j) != 0) << 1) |
          (uchar(img.at<uchar>(i + 2, j) != 0) << 2) |
          (uchar(img.at<uchar>(i + 3, j) != 0) << 3) |
          (uchar(img.at<uchar>(i + 4, j) != 0) << 4) |
          (uchar(img.at<uchar>(i + 5, j) != 0) << 5);

        buffer[j] = sixel_bits + 0x3F;
      }
      fmt::print("#1{}$", buffer);
      // print BLACK
      for (size_t j = 0; j < img.cols; ++j) {
        uint8_t sixel_bits = (uchar(img.at<uchar>(i + 0, j) == 0) << 0) |
          (uchar(img.at<uchar>(i + 1, j) == 0) << 1) |
          (uchar(img.at<uchar>(i + 2, j) == 0) << 2) |
          (uchar(img.at<uchar>(i + 3, j) == 0) << 3) |
          (uchar(img.at<uchar>(i + 4, j) == 0) << 4) |
          (uchar(img.at<uchar>(i + 5, j) == 0) << 5);

        buffer[j] = sixel_bits + 0x3F;
      }
      fmt::print("#0{}-", buffer);
    }
    fmt::print("\e\\");
  }

  void sixelize_gs6(cv::InputArray src) {
    cv::Mat img;
    cv::cvtColor(src, img, cv::COLOR_BGR2GRAY);
    img.forEach<uint8_t>([](uint8_t& x, const int pos[]) {
      x = select<51>(x);
    });
    
    // clang-format off
    // Palette setup
    fmt::print(
      "\ePq"
      "#0;2;0;0;0"
      "#1;2;20;20;20"
      "#2;2;40;40;40"
      "#3;2;60;60;60"
      "#4;2;80;80;80"
      "#5;2;100;100;100"
    );
    // clang-format on
    std::string buffer(img.cols, '\0');
    for (size_t r = 0; r < img.rows; r += 6) {
      for (size_t pc = 0; pc <= 5; ++pc) {
        fmt::print("#{}", pc);
        
        for (size_t c = 0; c < img.cols; ++c) {
          uint8_t sixel_bits = 
            (uchar(extract(img, r + 0, c) == pc) << 0) |
            (uchar(extract(img, r + 1, c) == pc) << 1) |
            (uchar(extract(img, r + 2, c) == pc) << 2) |
            (uchar(extract(img, r + 3, c) == pc) << 3) |
            (uchar(extract(img, r + 4, c) == pc) << 4) |
            (uchar(extract(img, r + 5, c) == pc) << 5);
          
          buffer[c] = sixel_bits + 0x3F;
        }
        fmt::print("{}{}", buffer, (pc == 5)? '-' : '$');
      }
    }
    fmt::print("\e\\");
    std::fflush(stdout);
  }
}  // namespace term