#ifndef ROCKETBULLET_HPP
#define ROCKETBULLET_HPP

#include "Bullet.hpp"
#include <string>

class Enemy;
class Turret;
namespace Engine{struct Point;}

class RocketBullet : public Bullet {
public:
    // position, direction, turret owner
    RocketBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret *parent);

    //tambahan buat edit bulet jadi lebi banyak
     RocketBullet(const std::string &spriteFile,Engine::Point position,Engine::Point forwardDirection,float rotation,Turret *parent);


    void Update(float deltaTime) override;

protected:

    void OnExplode(Enemy *enemy)override;


};

#endif // ROCKETBULLET_HPP
