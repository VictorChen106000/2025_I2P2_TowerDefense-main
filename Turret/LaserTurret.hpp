#ifndef LASERTURRET_HPP
#define LASERTURRET_HPP
#include "Turret.hpp"

class LaserTurret : public Turret {
public:
    static const int Price;
    LaserTurret(float x, float y);
    void CreateBullet() override;
     float GetDPS() const override {
        return 10.0f / coolDown;  // so if coolDown=0.5, DPS=20.0
    }
};
#endif   // LASERTURRET_HPP
