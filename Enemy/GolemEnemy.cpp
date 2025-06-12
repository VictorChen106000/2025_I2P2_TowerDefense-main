// GolemEnemy.cpp
#include "GolemEnemy.hpp"
#include <cmath>
#include <allegro5/allegro.h>  // for ALLEGRO_PI

GolemEnemy::GolemEnemy(int x, int y)
    : Enemy("play/golemattack.png",  // under resources/images/play/
            x, y,
            /*radius=*/30,
            /*speed=*/40,
            /*hp=*/200,
            /*money=*/25),
      originalSpeed(speed),
      originalHP(hp)
{
    // 1) Animate 4×3 @ 6fps
    SetAnimation(4, 3, 6.0f);
    // skip the blank last cell → use frames 0..10
    SetFrameSequence({0,1,2,3, 4,5,6,7, 8,9,10});

    // 2) Scale each 64×64 cell to 75% → 48×48 on-screen
    float frameW = GetBitmapWidth()  / 4.0f;   // 256/4 = 64
    float frameH = GetBitmapHeight() / 3.0f;   // 192/3 = 64
    Size.x = frameW * 1.7f;
    Size.y = frameH * 1.7f;
}

void GolemEnemy::Update(float deltaTime) {
    // ─── Shield logic ────────────────────────────────────
    shieldTimer += deltaTime;
    if (shieldRemaining > 0.0f) {
        shieldRemaining -= deltaTime;
        if (shieldRemaining <= 0.0f) {
            shieldActive = false;
        }
    }
    else if (shieldTimer >= shieldCooldown) {
        shieldTimer     = 0.0f;
        shieldRemaining = shieldDuration;
        shieldActive    = true;
    }

    // ─── Move & animate once ─────────────────────────────
    Enemy::Update(deltaTime);

    // ─── Orientation: vertical vs horizontal ─────────────
    float vx = Velocity.x, vy = Velocity.y;
    if (std::fabs(vy) > std::fabs(vx)) {
        // vertical → rotate ±90°
        Rotation = (vy > 0 ? ALLEGRO_PI/2 : -ALLEGRO_PI/2);
        Size.x = std::fabs(Size.x);
    } else {
        // horizontal → upright + mirror
        Rotation = 0.0f;
        float w = std::fabs(Size.x);
        Size.x = (vx < 0 ? -w : w);
    }
}

void GolemEnemy::Hit(float damage) {
    // apply shield reduction
    float actual = shieldActive ? damage * 0.5f : damage;
    bool willDie = (hp - actual) <= 0.0f;
    Enemy::Hit(actual);

    // enrage once below 30% HP
    if (!willDie && !enraged && hp < originalHP * 0.3f) {
        enraged = true;
        speed   = originalSpeed * 5.0f;
    }
}
