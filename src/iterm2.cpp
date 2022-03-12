#include "term.hpp"

#include <fmt/core.h>
#include <libbase64.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace term {
  void encode_iterm2(cv::InputArray arr) {
    cv::Mat img;
    cv::cvtColor(arr, img, cv::COLOR_BGR2RGB);

    std::string bytes = fmt::format(
      "P6 {} {} 255 {}", arr.size().width, arr.size().height,
      std::string_view(
        reinterpret_cast<char*>(img.data),
        reinterpret_cast<char*>(img.data + (img.rows * img.cols * 3))));
    std::string b64(bytes.size() * 4 / 3, '\0');

    size_t outlen;
    base64_encode(
      reinterpret_cast<const char*>(bytes.data()), bytes.size(), b64.data(),
      &outlen, 0);

    fmt::print(
      "\e]1337;File=inline=1;width={}px;height={}px:{}\a", arr.size().width,
      arr.size().height, b64.c_str());
  }
}  // namespace term