#ifndef LASERTURRET_HPP
#define LASERTURRET_HPP
#include "Turret.hpp"

class LaserTurret : public Turret {
public:
    static const int Price;
    LaserTurret(float x, float y);
    //void CreateBullet() override;
private:
    void applylevelstats()override;
    void CreateBullet() override;
    void setlevelimages() override;
    
};
#endif   // LASERTURRET_HPP
