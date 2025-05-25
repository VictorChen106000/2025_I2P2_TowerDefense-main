#include <allegro5/base.h>
#include "RocketBullet.hpp"
#include "Scene/PlayScene.hpp"
#include "Enemy/Enemy.hpp"
#include "UI/Animation/DirtyEffect.hpp"  // fallback effect

RocketBullet::RocketBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret *parent)
    : Bullet(
        "play/bullet-3.png",      // Rocket sprite
        600.0f,                    // speed (faster than machine gun)
        15.0f,                     // damage (higher than fire)
        position,
        forwardDirection,
        rotation - ALLEGRO_PI/2,   // align sprite
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
