add_executable(tvp
  "src/main.cpp"
  "src/term.hpp"
  "src/fixed_string.hpp"
  "src/sixel.cpp"
)
set_target_properties(tvp PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED yes
)
target_compile_features(tvp PUBLIC cxx_std_20)
target_link_libraries(tvp PUBLIC opencv::opencv ffmpeg::ffmpeg base64::base64 fmt::fmt)
target_compile_options(tvp PRIVATE "-msse4.2")