// WolfEnemy.cpp
#include "WolfEnemy.hpp"
#include "cmath"

WolfEnemy::WolfEnemy(int x, int y)
    : Enemy("play/frog.png",  // path under resources/images/
            x, y,
            /*radius=*/20,   // collision circle radius
            /*speed=*/90,    // pixels/sec
            /*hp=*/30,       // hit points
            /*money=*/15),   // reward when killed
      originalSpeed(speed),
      originalHP(hp)
{
    // Tell the sprite to chop 192×64 into 6×2 frames and play at 12fps:
    SetAnimation(
      /*cols=*/8,
      /*rows=*/1,
      /*fps=*/12.0f
    );
    SetFrameSequence({
    0,  1,  2,  3, 4,  5,  6,  7 // tergantung gambarmu di png nya
    });

    float frameW = GetBitmapWidth()  / 20.0f;  // 192 / 6  = 32  // ini buat misal pixel di gmbr 3000 kan gede bs disini bagi
    float frameH = GetBitmapHeight() / 4.0f;  // 64  / 2  = 32

    //–– draw at exactly half size (16×16) → scale factor .5 ––
    Size.x = frameW * 1.7f;
    Size.y = frameH * 1.7f;
}

void WolfEnemy::Update(float deltaTime) {

    // If we’re currently in a dash, count it down
    if (dashRemaining > 0.0f) {
        dashRemaining -= deltaTime;
        if (dashRemaining <= 0.0f) {
            // Dash ended → restore normal/enraged speed
            speed = originalSpeed * (enraged ? 1.5f : 1.0f);
        }
    }

    // If cooldown elapsed and not currently dashing → start a new dash
    if (dashTimer >= dashCooldown && dashRemaining <= 0.0f) {
        dashTimer     = 0.0f;
        dashRemaining = dashDuration;
        // Dash at double (or triple if enraged) speed
        speed = originalSpeed * (enraged ? 1.5f : 1.0f) * 2.0f;
    }

    // ─── Movement & animation handled by base ─────────────
   // ── Move & animate (once!) ──────────────────────────────
    Enemy::Update(deltaTime);

    // ── Orientation based on direction ──────────────────────
    float vx = Velocity.x;
    float vy = Velocity.y;

    if (std::fabs(vy) > std::fabs(vx)) {
        // Vertical movement dominates: face up/down
        Rotation = (vy > 0.0f ?  ALLEGRO_PI/2  // down = +90°
                             : -ALLEGRO_PI/2); // up   = -90°
        // no horizontal flip when vertical
        Size.x =  std::abs(Size.x);
    }
    else {
        // Horizontal movement dominates: keep upright
        Rotation = 0.0f;
        float w = std::abs(Size.x);
        // flip left (<0) or right (>0)
        Size.x = (vx < 0.0f ? -w : w);
    }
}

void WolfEnemy::Hit(float damage) {
    bool willDie = (hp - damage) <= 0.0f;
    Enemy::Hit(damage);

    // On first time dropping below 50% HP (and not dying), enrage:
    if (!willDie && !enraged && hp < originalHP * 0.5f) {
        enraged = true;
        // +50% permanent speed boost
        speed   = originalSpeed * 1.5f;
    }
}

// DASH