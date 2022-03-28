#include <atomic>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <ios>
#include <chrono>
#include <iterator>
#include <optional>
#include <system_error>
#include <vector>
#include <thread>

#include <avcpp/dictionary.h>
#include <avcpp/frame.h>
#include <avcpp/pixelformat.h>
#include <avcpp/formatcontext.h>
#include <avcpp/codec.h>
#include <avcpp/codeccontext.h>
#include <avcpp/stream.h>
#include <avcpp/packet.h>
#include <avcpp/rational.h>
#include <avcpp/videorescaler.h>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <cxxabi.h>
#include <signal.h>

#include "term.hpp"



int main(int argc, char* argv[]) {
  using namespace std::literals;
  namespace this_thread = std::this_thread;
  namespace chr = std::chrono;
  using hires_clock = chr::high_resolution_clock;
  
  // check args
  if (argc < 2) {
    return 1;
  }
  
  // setup terminate handler
  std::set_terminate([]() {
    bool exc_flag = true;
    try {
      std::rethrow_exception(std::current_exception());
    }
    catch (const std::exception& err) {
      size_t len;
      int status = -4;
      std::unique_ptr<char[], decltype(std::free)*> str(
        abi::__cxa_demangle(typeid(err).name(), NULL, &len, &status), std::free);
      
      fmt::print("exception occurred ({})\n {}\n", std::string_view(str.get(), len), err.what());
      exc_flag = false;
    }
    catch (...) {
      fmt::print("Exception of unknown type\n");
      exc_flag = false;
    }
    if (exc_flag)
      fmt::print("std::terminate called without an exception\n");
    std::exit(1);
  });
  
  // Load file
  av::FormatContext format;
  format.openInput(argv[1]);
  format.findStreamInfo();
  
  // Search for video stream
  av::Stream vstream;
  int vindex;
  chr::microseconds frame_time;
  for (size_t i = 0; i < format.streamsCount(); i++) {
    av::Stream s = format.stream(i);
    if (s.isVideo()) {
      vstream = s;
      vindex = i;
      
      auto fps = s.frameRate();
      // no overloaded operator, multiply by reciprocal
      frame_time = 1'000'000us * fps.getDenominator() / fps.getNumerator();
      
      break;
    }
  }
  if (vstream.isNull()) {
    fmt::print(stderr, "No video stream");
    return 1;
  }
  
  // Load and start codec
  av::VideoDecoderContext decoder(vstream);
  {
    av::Codec codec = av::findDecodingCodec(decoder.raw()->codec_id);
    decoder.setCodec(codec);
    decoder.setRefCountedFrames(true);
    
    decoder.open(codec);
  }
  
  // Setup atomic run flag
  static std::atomic_bool run_flag = true;
  signal(SIGINT, [](int sig) {
    run_flag = false;
  });
  
  // switch to alt buffer, unset DECSDM
  std::fputs("\e[?1049h\e[?80h", stdout);
  std::fflush(stdout);
  
  // Decode variables
  av::Packet packet;
  size_t count = 0;
  
  // Timekeeping variables
  hires_clock::time_point frame_begin = hires_clock::now();
  chr::microseconds lag = 0us;
  
  while ((packet = format.readPacket(), bool(packet)) & run_flag) {
    // check for complete frame
    frame_begin = hires_clock::now();
    // only worry about video frames
    if (packet.streamIndex() != vindex) continue;
    // check frame is complete
    av::VideoFrame frame = decoder.decode(packet);
    if (!frame.isComplete()) continue;
    // check if we need to skip a frame
    if (lag > frame_time) {
      lag -= frame_time;
      continue;
    }
    
    
    // Determine rescale dimensions
    term::term_size ts = term::query_size();
    size_t fw, fh;
    {
      av::Rational term_aspect {int(ts.width), int(ts.height)};
      av::Rational video_aspect {int(decoder.width()), int(decoder.height())};
      
      if (video_aspect < term_aspect) {
        fh = ts.height;
        fw = decoder.width() * ts.height / decoder.height();
      }
      else {
        fw = ts.width;
        fh = decoder.height() * ts.width / decoder.width();
      }
    }
    av::VideoRescaler scaler(fw, fh, av::PixelFormat("gray"));
    av::VideoFrame frame2 = scaler.rescale(frame, av::throws());
    term::sixel_encode(frame2, {.ncols = 16});
    
    {
      auto now = hires_clock::now();
      auto exp = frame_begin + frame_time;
      if (now > exp) {
        // keep track of current lag for frameskipping
        lag += chr::duration_cast<chr::microseconds>(now - exp);
      }
      else {
        this_thread::sleep_for(exp - now);
      }
    }
    count++;
  }
  
  // switch to main buffer, set DECSDM
  std::fputs("\e[?1049l\e[?80l", stdout);
  std::fflush(stdout);
}