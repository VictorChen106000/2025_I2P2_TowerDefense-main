#ifndef ROCKETBULLET_HPP
#define ROCKETBULLET_HPP

#include "Bullet.hpp"

class Enemy;
class Turret;
namespace Engine { struct Point; }

class RocketBullet : public Bullet {
public:
    // position, direction, turret owner
    RocketBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret *parent);
protected:
    // spawn explosion or effect on hit
    void OnExplode(Enemy *enemy) override;
};

#endif  // ROCKETBULLET_HPP
