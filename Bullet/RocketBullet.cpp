#include <allegro5/base.h>
#include <random>
#include <string>
#include <algorithm>

#include "Enemy/Enemy.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "RocketBullet.hpp"
#include "Scene/PlayScene.hpp"
#include "UI/Animation/DirtyEffect.hpp"

RocketBullet::RocketBullet(const std::string &spriteFile,
                           Engine::Point position,
                           Engine::Point forwardDirection,
                           float rotation,
                           Turret *parent)
    : Bullet(
        spriteFile,       // ← now parameterized
        600.0f,
        3.0f,
        position,
        forwardDirection,
        rotation + ALLEGRO_PI / 2,
        parent
      )
{ }

RocketBullet::RocketBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret *parent)
    : Bullet(
        "play/bullet-3.png",      // Rocket sprite
        600.0f,                    // speed (faster than machine gun)
        3.0f,                     // damage (higher than fire)
        position,
        forwardDirection,
        rotation + ALLEGRO_PI / 2,   // align sprite
        parent
      )
{
}

void RocketBullet::Update(float deltaTime) {
    // If target still alive, adjust our velocity toward it
    if (_target && _target->Visible) {
        Engine::Point toT = (_target->Position - Position).Normalize();
        Engine::Point cur = Velocity.Normalize();
        float dot = std::clamp(cur.Dot(toT), -1.0f, 1.0f);
        float angle = acos(dot);
        float maxTurn = _turnRate * deltaTime;

        Engine::Point newDir;
        if (angle <= maxTurn) {
            newDir = toT;
        } else {
            // pick rotation direction via cross product sign
            float sign = (cur.x * toT.y - cur.y * toT.x) < 0 ? -1 : 1;
            float theta = maxTurn * sign;
            newDir.x = cur.x * cos(theta) - cur.y * sin(theta);
            newDir.y = cur.x * sin(theta) + cur.y * cos(theta);
            newDir = newDir.Normalize();
        }
        Velocity = newDir * speed;
        Rotation = atan2(newDir.y, newDir.x) + ALLEGRO_PI/2;
    }

    // Then let the base class handle movement & collision
    Bullet::Update(deltaTime);
}

void RocketBullet::OnExplode(Enemy* enemy) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(2, 5);
    getPlayScene()->GroundEffectGroup->AddNewObject(new DirtyEffect("play/dirty-1.png", dist(rng), enemy->Position.x, enemy->Position.y));
}
