#include <allegro5/base.h>
#include <cmath>
#include "RocketTurret.hpp"
#include "Bullet/RocketBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Scene/PlayScene.hpp"
#include "Enemy/Enemy.hpp"

const int RocketTurret::Price = 80;

RocketTurret::RocketTurret(float x, float y)
    : Turret("play/tower-base.png", "play/turret-3.png", x, y, 9999, Price, 1.2) {
    Anchor.y += 8.0f / GetBitmapHeight();
}

// helper: returns the enemy with the largest reachEndTime (furthest from exit)
static Enemy* pickBackMost(PlayScene* scene) {
    float bestTime = -1;
    Enemy* best    = nullptr;
    for (auto& obj : scene->EnemyGroup->GetObjects()) {
        Enemy* e = dynamic_cast<Enemy*>(obj);
        if (!e || !e->Visible) continue;
        if (e->reachEndTime > bestTime) {
            bestTime = e->reachEndTime;
            best     = e;
        }
    }
    return best;
}

void RocketTurret::Update(float deltaTime) {
    // 1) pick the same back-most (or whatever) target
    Enemy* target = pickBackMost(getPlayScene());
    if (target && target->Visible) {
        // 2) compute direction & rotate smoothly
        Engine::Point toT = (target->Position - Position).Normalize();
        float desired = atan2(toT.y, toT.x) + ALLEGRO_PI/2;

        // optional: interpolate Rotation toward desired at some speed
        Rotation = desired;
    }

    // 3) let the base class handle cooldown & firing as usual
    Turret::Update(deltaTime);
}

void RocketTurret::CreateBullet() {
    auto* scene = getPlayScene();
    if (!scene) return;

    // pick back-most target
    Enemy* target = pickBackMost(scene);
    if (!target) return;

    Engine::Point dir = (target->Position - Position).Normalize();
    float rotation    = atan2(dir.y, dir.x);
    Engine::Point perp(-dir.y, dir.x);

    float spawnDist = 36, offset = 6;
    // left rocket
    scene->BulletGroup->AddNewObject(new RocketBullet(
        Position + dir*spawnDist - perp*offset,
        dir,
        rotation,
        this,
        target,               // pass it here
        "play/bullet-3.png"
    ));
    // right rocket
    scene->BulletGroup->AddNewObject(new RocketBullet(
        Position + dir*spawnDist + perp*offset,
        dir,
        rotation,
        this,
        target,
        "play/bullet-3.png"
    ));

    AudioHelper::PlayAudio("laser.wav");
}
