#include <allegro5/base.h>
#include <cmath>
#include "RocketBullet.hpp"
#include "Scene/PlayScene.hpp"
#include "Enemy/Enemy.hpp"
#include "UI/Animation/DirtyEffect.hpp"  // fallback effect

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

void RocketBullet::OnExplode(Enemy *enemy) {
    // simple ground effect on hit — feel free to swap in an explosion sprite
    getPlayScene()->GroundEffectGroup
        ->AddNewObject(new DirtyEffect(
            "play/dirty-1.png",      // reuse existing dirty effect
            3,                       // 3 frames
            enemy->Position.x,
            enemy->Position.y
        ));
}
void RocketBullet::Update(float deltaTime) {
    if (Target && Target->Visible) {
        // compute new direction
        Engine::Point toTarget = (Target->Position - Position).Normalize();
        // update our velocity & rotation
        Velocity = toTarget * speed;
        Rotation = atan2(toTarget.y, toTarget.x) + ALLEGRO_PI / 2;
    }
    // now do all the normal movement, collision & removal
    Bullet::Update(deltaTime);
}