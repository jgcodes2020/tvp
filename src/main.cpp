#include <fmt/core.h>
#include <libbase64.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <atomic>
#include <thread>
#include <stdexcept>
#include <vector>
#include <signal.h>
#include "term.hpp"
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

namespace thread = std::this_thread;
namespace chr = std::chrono;
using namespace std::literals;

void resize_to_term(cv::Mat& img) {
  cv::Size2d tsize = term::sixel_size();
  cv::Size2d isize = img.size();

  double scale_factor =
    (tsize.width / tsize.height > isize.width / isize.height)
    ? (tsize.height / isize.height)
    : (tsize.width / isize.width);

  cv::resize(
    img, img, cv::Size(), scale_factor, scale_factor,
    (scale_factor > 1) ? cv::INTER_LINEAR : cv::INTER_AREA);
}

int main(int argc, char* argv[]) {
  cv::VideoCapture video(argv[1]);
  if (!video.isOpened()) {
    throw std::runtime_error("Video could not be opened");
  }

  cv::Mat frame;
  
  static std::atomic_bool stop_flag = true;
  
  signal(SIGINT, [](int x) {
    stop_flag = false;
  });
#if 1
  term::set_alt_buffer(true);
  term::set_decsdm(true);
  auto mspf = 1000ms / video.get(cv::CAP_PROP_FPS);
  auto time = chr::high_resolution_clock::now();
  while ((void(video >> frame), !frame.empty()) && stop_flag) {
    resize_to_term(frame);
    term::encode_sixel(frame);
    thread::sleep_until(time + mspf);
    time = chr::high_resolution_clock::now();
  }
  term::set_alt_buffer(false);
  term::set_decsdm(false);
#else
  for (size_t i = 0; i < 60; i++) {
    video >> frame;
  }
  resize_to_term(frame);
  term::encode_iterm2(frame);
#endif
  // cv::destroyAllWindows();
}