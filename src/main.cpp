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
#include <avcpp/videorescaler.h>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <cxxabi.h>

#include "term.hpp"



int main(int argc, char* argv[]) {
  using namespace std::literals;
  namespace this_thread = std::this_thread;
  namespace chr = std::chrono;
  
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
  for (size_t i = 0; i < format.streamsCount(); i++) {
    av::Stream s = format.stream(i);
    if (s.isVideo()) {
      vstream = s;
      vindex = i;
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
  
  // Create rescaler
  av::VideoRescaler scaler {
    10 * 80, 20 * 24, av::PixelFormat("gray")
  };
  // switch to alt buffer, unset DECSDM
  std::fputs("\e[?1049h\e[?80h", stdout);
  std::fflush(stdout);
  
  // Decode stuff
  av::Packet packet;
  size_t count = 0;
  
  chr::high_resolution_clock::time_point frame_begin;
  
  while (packet = format.readPacket(), bool(packet)) {
    frame_begin = chr::high_resolution_clock::now();
    if (packet.streamIndex() != vindex) continue;
    
    av::VideoFrame frame = decoder.decode(packet);
    if (!frame.isComplete()) continue;
    
    av::VideoFrame frame2 = scaler.rescale(frame, av::throws());
    term::sixel_encode(frame2, {.ncols = 16});
    
    this_thread::sleep_until(frame_begin + 50ms);
    count++;
  }
  
  // switch to main buffer, set DECSDM
  std::fputs("\e[?1049l\e[?80l", stdout);
  std::fflush(stdout);
}