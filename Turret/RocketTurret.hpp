#ifndef ROCKETTURRET_HPP
#define ROCKETTURRET_HPP

#include "Turret.hpp"

class RocketTurret : public Turret {
public:
    static const int Price;
    RocketTurret(float x, float y);
    void CreateBullet() override;
     float GetDPS() const override {
        return 10.0f / coolDown;  // so if coolDown=0.5, DPS=20.0
    }
};

#endif  // ROCKETTURRET_HPP
