#include "GolemEnemy.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/Resources.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>

// global list of enemies
extern std::vector<Enemy*> g_enemies;

GolemEnemy::GolemEnemy(int x, int y)
    : Enemy("play/golemattack.png",
            x, y,
            /*radius=*/30,
            /*speed=*/40,
            /*hp=*/200,
            /*money=*/25),
      originalSpeed(speed),
      originalHP(hp)
{
    // 4×3 @6fps, frames 0..10
    SetAnimation(4, 3, 6.0f);
    SetFrameSequence({0,1,2,3, 4,5,6,7, 8,9,10});

    // scale each cell
    float frameW = GetBitmapWidth()  / 4.0f;  // =64
    float frameH = GetBitmapHeight() / 3.0f;  // =64
    Size.x = frameW * 1.8f;
    Size.y = frameH * 1.8f;
}

void GolemEnemy::Hit(float damage) {
    if (_isDying) return;  // already in death animation

    // apply shield reduction
    float actual = shieldActive ? damage * 0.5f : damage;
    bool willDie = (hp - actual) <= 0.0f;

    if (willDie) {
        // start death animation instead of immediate removal
        _isDying         = true;
        _deathFrame      = 0;
        _deathFrameTimer = 0.0f;
        return;
    }

    // normal hit
    Enemy::Hit(actual);

    // enrage at low HP
    if (!enraged && hp < originalHP * 0.6f) {
        enraged = true;
        speed   = originalSpeed * 5.0f;
    }
}

void GolemEnemy::Update(float deltaTime) {
    if (_isDying) {
        // advance death frames
        _deathFrameTimer += deltaTime;
        if (_deathFrameTimer >= deathFrameDuration) {
            _deathFrameTimer -= deathFrameDuration;
            ++_deathFrame;
            if (_deathFrame >= deathFrames) {
                // finished → trigger actual removal/explosion
                Enemy::Hit(hp);  // hp<=0 so base Hit will handle cleanup
            }
        }
        return;  // skip shield/move while dying
    }

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

    // ─── Movement & animation ───────────────────────────
    Enemy::Update(deltaTime);

    // ─── Orientation: always horizontal facing ──────────
    Rotation = 0.0f;
    float w = std::fabs(Size.x);
    Size.x = (Velocity.x < 0 ? -w : w);
}

void GolemEnemy::Draw() const {
    if (_isDying) {
    // grab the death‐sheet
    auto bmpPtr = Engine::Resources::GetInstance()
                      .GetBitmap("play/golemdie.png");
    ALLEGRO_BITMAP* bmp = bmpPtr.get();
    const int cols = 4, rows = 3;
    int fw = al_get_bitmap_width(bmp)  / cols;
    int fh = al_get_bitmap_height(bmp) / rows;
    int sx = (_deathFrame % cols) * fw;
    int sy = (_deathFrame / cols) * fh;

    // ** scale up **
    float deathScale = 2.0f;                // 2× size
    int dw = int(fw * deathScale);
    int dh = int(fh * deathScale);

    al_draw_scaled_bitmap(
        bmp,
        sx, sy, fw, fh,
        // center the scaled quad on Position
        Position.x - dw/2.0f,
        Position.y - dh/2.0f,
        dw, dh,
        0
    );
    return;
}

    
        // normal golem draw
        Enemy::Draw();
    
}
