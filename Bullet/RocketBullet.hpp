#ifndef ROCKETBULLET_HPP
#define ROCKETBULLET_HPP

#include "Bullet.hpp"
#include <string>

class Enemy;
class Turret;

class RocketBullet : public Bullet {
public:
    RocketBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret* parent, Enemy* target, std::string img = "play/bullet-3.png");
    void OnExplode(Enemy* enemy) override;
    void Update(float deltaTime) override;

private:
    Enemy* _target;
    float _turnRate;
};

#endif // ROCKETBULLET_HPP
