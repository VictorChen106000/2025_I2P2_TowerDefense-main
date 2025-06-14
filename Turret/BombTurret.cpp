#include <cmath>
#include "BombTurret.hpp"
#include "Engine/Collider.hpp"
#include "Scene/PlayScene.hpp"
#include "Enemy/Enemy.hpp"
#include "UI/Animation/ExplosionEffect.hpp"
#include "Engine/AudioHelper.hpp"

const int BombTurret::Price = 150;

BombTurret::BombTurret(float x, float y)
  : Turret("play/bomb.png", "play/blank.png",
           x, y,
           /*rotateRate*/ 0.0f,
           Price,
           /*bulletCooldown*/ 0.0f),
    triggerRadius(32.0f),
    explosionRadius(128.0f)
{
    Anchor.y += 0.5f;  // sink it a bit into the ground
}

void BombTurret::Update(float deltaTime) {
    // 1) Run the base class (for any sprite animation, rotation, etc.)
    Turret::Update(deltaTime);

    // 2) If we're still in “preview” mode, bail out immediately.
    if (this->Preview) 
        return;

    // 3) If we've already exploded, remove ourselves now.
    if (hasExploded) {
        return;
    }

    // 4) Otherwise—we’re placed and haven't gone off yet—scan for any enemy in range.
    auto scene = getPlayScene();
    for (auto* obj : scene->EnemyGroup->GetObjects()) {
        auto* e = dynamic_cast<Enemy*>(obj);
        if (!e) continue;
        if (Engine::Collider::IsCircleOverlap(
                Position, triggerRadius,
                e->Position, e->CollisionRadius))
        {
            Explode();      // this sets hasExploded = true
            break;
        }
    }
}

void BombTurret::applylevelstats() {
    // no extra per‐level stats for the bomb turret
}

void BombTurret::setlevelimages() {
    // no level‐up images for the bomb turret
}

void BombTurret::Explode() {
    auto scene = getPlayScene();

    // 1) Explosion effect + sound
    scene->EffectGroup->AddNewObject(
        new ExplosionEffect(Position.x, Position.y)
    );
    AudioHelper::PlayAudio("explosion.wav");

    // 2) Damage
    for (auto* obj : scene->EnemyGroup->GetObjects()) {
        auto* e = dynamic_cast<Enemy*>(obj);
        if (!e) continue;
        float dx = e->Position.x - Position.x;
        float dy = e->Position.y - Position.y;
        if (dx*dx + dy*dy <= explosionRadius * explosionRadius)
            e->Hit(INFINITY);
    }

    // 3) Hide and remove *right now*
    this->Visible = false;   // no more drawing
    scene->TowerGroup->RemoveObject(GetObjectIterator());
}



