#ifndef BOWTURRET_HPP
#define BOWTURRET_HPP

#include "Turret.hpp"
#include <vector>

class BowTurret : public Turret {
public:
  static const int Price;
  BowTurret(float x, float y);

  // ...
    void Update(float deltaTime) override;
private:
  void applylevelstats() override;
  void CreateBullet() override;
  void setlevelimages() override;
  void setBulletImages();

  // prevent auto–locking
  Enemy* AcquireTarget() override { return nullptr; }

 

  std::vector<std::string> bulletImages;
};


#endif  // BallistaTurret_HPP
