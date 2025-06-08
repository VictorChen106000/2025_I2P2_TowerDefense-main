#ifndef ROCKETTURRET_HPP
#define ROCKETTURRET_HPP

#include "Turret.hpp"
#include <vector>

class RocketTurret : public Turret {
public:
    static const int Price;
    RocketTurret(float x, float y);
   //void CreateBullet() override;
private:
    void applylevelstats()override;
    void CreateBullet() override;
    void setlevelimages() override;

    std::vector<std::string>bulletImages;
    void setBulletImages();
};

#endif  // ROCKETTURRET_HPP
