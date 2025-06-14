// SlimeEnemy.cpp
#include "SlimeEnemy.hpp"
#include <cmath>
#include <allegro5/allegro.h>  // for ALLEGRO_PI

SlimeEnemy::SlimeEnemy(int x, int y)
    : Enemy("play/slime.png",  // your 256×64 PNG under resources/images/play/
            x, y,
            /*radius=*/20,
            /*speed=*/50,
            /*hp=*/40,
            /*money=*/10),
      originalHP(hp)
{
    // 1) Chop into 4×1 frames @ 8 fps
    SetAnimation(4, 1, 8.0f);

    // 2) Scale to 75% of each 64×64 frame → 48×48 on‐screen
    float frameW = GetBitmapWidth()  / 4.0f;  // = 256/4 = 64
    float frameH = GetBitmapHeight() / 1.0f;  // =  64/1 = 64
    Size.x = frameW * 1.75f;
    Size.y = frameH * 1.75f;
}

void SlimeEnemy::Update(float deltaTime) {
    // ─── Regeneration ───────────────────────────────
    regenTimer += deltaTime;
    if (regenTimer >= regenCooldown) {
        regenTimer -= regenCooldown;
        hp += regenAmount;
        if (hp > originalHP) hp = originalHP;
    }

    // ─── Move + animate once ─────────────────────────
    Enemy::Update(deltaTime);

    // ─── Orientation: face up/down on vertical, left/right on horiz ───
    float vx = Velocity.x;
    float vy = Velocity.y;
    if (std::fabs(vy) > std::fabs(vx)) {
        // mostly vertical → rotate ±90°
        Rotation = (vy > 0.0f ?  ALLEGRO_PI/2  // down
                             : -ALLEGRO_PI/2); // up
        // no mirror
        Size.x = std::fabs(Size.x);
    }
    else {
        // mostly horizontal → keep upright + mirror
        Rotation = 0.0f;
        float w = std::fabs(Size.x);
        Size.x = (vx < 0.0f ? -w : w);
    }
}

void SlimeEnemy::Hit(float damage) {
    bool willDie = (hp - damage) <= 0.0f;
    Enemy::Hit(damage);

    // on first drop below half‐health, double regen rate
    if (!willDie && !enraged && hp < originalHP * 0.5f) {
        enraged       = true;
        regenAmount  *= 2.0f;     // twice the HP per tick
        regenCooldown *= 0.5f;    // twice as often
    }
}

//REGEN
