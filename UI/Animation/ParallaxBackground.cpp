// ParallaxBackground.cpp
#include "ParallaxBackground.hpp"
#include <cstdio>
#include <stdexcept>
#include <cmath>

ParallaxBackground::ParallaxBackground(float l4, float l3, float l2, float l1) {
  // speed[0] unused for static layer
  speed[1] = l4;
  speed[2] = l3;
  speed[3] = l2;
  speed[4] = l1;
}

ParallaxBackground::~ParallaxBackground() { Unload(); }

bool ParallaxBackground::Load(const std::vector<std::string>& paths) {
  if (paths.size() != 5) throw std::invalid_argument("Need exactly 5 image paths");
  for (int i = 0; i < 5; i++) {
    bm[i] = al_load_bitmap(paths[i].c_str());
    if (!bm[i]) {
      std::fprintf(stderr, "Failed to load %s\n", paths[i].c_str());
      Unload();
      return false;
    }
  }
  return true;
}

void ParallaxBackground::Unload() {
  for (auto &b : bm) {
    if (b) { al_destroy_bitmap(b); b = nullptr; }
  }
}

void ParallaxBackground::Draw(int w, int h, double t) const {
  // draw static back (index 0)
  {
    int bw = al_get_bitmap_width(bm[0]), bh = al_get_bitmap_height(bm[0]);
    al_draw_scaled_bitmap(bm[0], 0,0, bw,bh, 0,0, w,h, 0);
  }
  // draw scrolling layers 1..4
  for (int i = 1; i < 5; i++) {
    int bw = al_get_bitmap_width(bm[i]), bh = al_get_bitmap_height(bm[i]);
    float scale = float(h) / bh;
    float sw    = bw * scale;
    float xoff  = std::fmod(t * speed[i], sw);
    // two‐tile trick
    al_draw_scaled_bitmap(bm[i], 0,0, bw,bh, -xoff,    0, sw,h, 0);
    al_draw_scaled_bitmap(bm[i], 0,0, bw,bh, -xoff+sw, 0, sw,h, 0);
  }
}