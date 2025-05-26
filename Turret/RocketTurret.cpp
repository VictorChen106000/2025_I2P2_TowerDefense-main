#include <allegro5/base.h>
#include <cmath>
#include "Enemy/Enemy.hpp"
#include "RocketTurret.hpp"
#include "Bullet/RocketBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Point.hpp"
#include "Scene/PlayScene.hpp"

const int RocketTurret::Price = 50;   

RocketTurret::RocketTurret(float x, float y)
    : Turret(
        "play/tower-base.png",         // base image
        "play/turret-3.png",           // head image
        x, y,
        800,                            // wider range (vs. 200)
        Price,
        0.8f                           // faster cooldown (vs. 0.5f)
      )
{
    // same downward shift so head aligns
    Anchor.y += 8.0f / GetBitmapHeight();
}

void RocketTurret::CreateBullet() {
    // same aim logic as MachineGunTurret
    Engine::Point diff = Engine::Point(
        cos(Rotation - ALLEGRO_PI/2),
        sin(Rotation - ALLEGRO_PI/2)
    );
    float rotation = atan2(diff.y, diff.x);
    Engine::Point norm = diff.Normalize();

    // spawn a rocket-style bullet
    {
        auto bullet = new RocketBullet(
            Position + norm * 36,
            diff,
            rotation,
            this
        );
        // give it the same enemy this turret is locked on
        bullet->Target = this->Target;
        if (bullet->Target) {
            bullet->Target->lockedBullets.push_back(bullet);
        }
        getPlayScene()->BulletGroup->AddNewObject(bullet);
    }
    AudioHelper::PlayAudio("rocket.wav"); // pick a rocket SFX
}
