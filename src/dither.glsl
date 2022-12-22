#version 430 core

#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
#extension GL_EXT_shader_integer_mix : enable

layout(std430, binding = 1) readonly buffer data_in {
  uint width;
  uint ibuf[];
};

layout(std430, binding = 2) writeonly buffer data_out {
  uint8_t obuf[];
};

layout(local_size_x = 1, local_size_y = 1) in;

/*
const int dither_table[4][4] = {
  { 0,  8,  2,  10 },
  { 12, 4,  14, 6  },
  { 3,  11, 1,  9  },
  { 15, 7,  13, 5  },
};
*/

int dist_sq(ivec3 x) {
  ivec3 sq = x * x;
  return sq.x + sq.y + sq.z;
}

uint8_t dither_calc(ivec3 col, uint x, uint y) {
  // calculate thresholds
  int tval = 0;
  const int thr_clo = (95 * tval) >> 4;
  const int thr_chi = (40 * tval) >> 4;
  const int thr_gs = (10 * tval) >> 4;
  
  // find colour components
  ivec3 cube_crgb = mix(ivec3(0, 0, 0), (col - (15 + thr_chi)), lessThan(col, thr_clo.xxx));
  ivec3 colc = mix(ivec3(0, 0, 0), (cube_crgb * 40 + 55), equal(cube_crgb, ivec3(0, 0, 0)));
  
  // find grayscale
  int avg = (col.r + col.g + col.b) / 3;
  int gs_idx = (avg > 238)? 23 : (avg + 2 - thr_gs) / 10;
  ivec3 gsc = ivec3(gs_idx * 10 + 8);
  
  // pick the closer colour
  int gs_dist = dist_sq(col - gsc);
  int col_dist = dist_sq(col - colc);
  
  // first check is necessary to increase colour resolution
  return (gs_dist < 400 && gs_dist < col_dist)? uint8_t(gs_idx + 232) : 
    uint8_t(cube_crgb.r * 36 + cube_crgb.g * 6 + cube_crgb.b + 16);
}

void main() {
  uint id = gl_GlobalInvocationID.y * width + gl_GlobalInvocationID.x;
  
  ivec3 col = ivec3((ibuf[id].xxx >> ivec3(24, 16, 8)) & 0xFF);
  obuf[id] = dither_calc(col, gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
}