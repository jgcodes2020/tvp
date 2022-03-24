#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <ios>
#include <iterator>
#include <optional>
#include <system_error>
#include <vector>


#include <avcpp/frame.h>
#include <avcpp/pixelformat.h>
#include <cxxabi.h>

#include <avcpp/formatcontext.h>
#include <avcpp/codec.h>
#include <avcpp/codeccontext.h>
#include <avcpp/stream.h>
#include <avcpp/packet.h>
#include <avcpp/videorescaler.h>

#include <fmt/core.h>
#include <fmt/ostream.h>



int main(int argc, char* argv[]) {
  // check args
  if (argc < 2) {
    return 1;
  }
  
  // setup terminate handler
  std::set_terminate([]() {
    if (std::uncaught_exceptions() > 0) {
      try {
        std::rethrow_exception(std::exception_ptr());
      }
      catch (const std::exception& err) {
        size_t len;
        int status = -4;
        std::unique_ptr<char[], decltype(std::free)*> str(
          abi::__cxa_demangle(typeid(err).name(), NULL, &len, &status), std::free);
        
        fmt::print("Exception of type {}: {}\n", str.get(), err.what());
      }
      catch (...) {
        fmt::print("Exception of unknown type\n");
      }
    }
    else {
      fmt::print("std::terminate called without an exception\n");
    }
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
    decoder.width(), decoder.height(), av::PixelFormat("gray")
  };
  
  // Decode stuff
  av::Packet packet;
  size_t count = 0;
  while (packet = format.readPacket(), bool(packet)) {
    if (count >= 10) break;
    if (packet.streamIndex() != vindex) continue;
    
    av::VideoFrame frame = decoder.decode(packet);
    if (!frame.isComplete()) continue;
    
    av::VideoFrame frame2 = scaler.rescale(frame, av::throws());
    
    
    count++;
  }
}