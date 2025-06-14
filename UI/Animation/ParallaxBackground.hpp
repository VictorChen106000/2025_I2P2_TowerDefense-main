// ParallaxBackground.hpp
#pragma once
#include <vector>
#include <string>
#include <allegro5/allegro.h>

class ParallaxBackground {
public:
  ParallaxBackground(float l4=37.5f, float l3=75.0f, float l2=150.0f, float l1=300.0f);
  ~ParallaxBackground();

  // paths[0] = static back, paths[1..4] = scroll layers 4→1
  bool    Load(const std::vector<std::string>& paths);
  void    Unload();
  void    Draw(int screenW, int screenH, double time) const;

private:
  ALLEGRO_BITMAP* bm[5]{};
  float           speed[5]{ 0, /* l4,l3,l2,l1 */ };
};
