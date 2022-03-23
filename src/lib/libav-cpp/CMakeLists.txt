add_executable(fftvp
  "src/ff_main.cpp"
)
set_target_properties(fftvp PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED yes
)
target_compile_features(fftvp PUBLIC cxx_std_20)
target_link_libraries(fftvp PUBLIC ffmpeg::ffmpeg base64::base64 fmt::fmt)
target_compile_options(fftvp PRIVATE "-msse4.2")