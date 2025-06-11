#ifndef BALLISTATURRET_HPP
#define BALLISTATURRET_HPP

#include "Turret.hpp"
#include <vector>

class BallistaTurret : public Turret {
public:
    static const int Price;
    BallistaTurret(float x, float y);
   //void CreateBullet() override;
   
private:
    void applylevelstats()override;
    void CreateBullet() override;
    void setlevelimages() override;
   

    std::vector<std::string>bulletImages;
    void setBulletImages();
};

#endif  // BallistaTurret_HPP
