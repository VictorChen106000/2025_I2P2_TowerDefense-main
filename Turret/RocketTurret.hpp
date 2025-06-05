#ifndef ROCKETTURRET_HPP
#define ROCKETTURRET_HPP

#include "Turret.hpp"

class PlayScene;
class Enemy;

class RocketTurret : public Turret {
public:
    static const int Price;
    RocketTurret(float x, float y);
    void Update(float deltaTime) override;
    void CreateBullet() override;
};

#endif // ROCKETTURRET_HPP
