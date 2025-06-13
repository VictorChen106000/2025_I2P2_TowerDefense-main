// WolfEnemy.cpp
#include "WolfEnemy.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/Resources.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>

WolfEnemy::WolfEnemy(int x, int y)
  : Enemy("play/skeleton.png", x, y,
          /*radius=*/20, /*speed=*/60, /*hp=*/30, /*money=*/15),
    originalSpeed(speed),
    originalHP(hp)
{
    // 13×5 @12fps, walk frames are in your custom sequence
    SetAnimation(13, 5, 12.0f);
    SetFrameSequence({ 26,27,28,29,30,31,32,33,34,35,36,37 });

    float frameW = GetBitmapWidth()  / 13.0f;  // =64
    float frameH = GetBitmapHeight() /  5.0f;  // =64
    Size.x = frameW * 1.7f;
    Size.y = frameH * 1.7f;
}

void WolfEnemy::Hit(float damage) {
    // if already dying, ignore
    if (_isDying) return;

    bool willDie = (hp - damage) <= 0.0f;
    if (willDie) {
        // start death animation
        _isDying         = true;
        _deathFrame      = 0;
        _deathFrameTimer = 0.0f;
        return;
    }

    // normal damage & enrage logic
    Enemy::Hit(damage);
    if (!enraged && hp < originalHP * 0.5f) {
        enraged = true;
        speed   = originalSpeed * 1.5f;
    }
}

void WolfEnemy::Update(float deltaTime) {
    if (_isDying) {
        // advance death frames
        _deathFrameTimer += deltaTime;
        if (_deathFrameTimer >= deathFrameDuration) {
            _deathFrameTimer -= deathFrameDuration;
            ++_deathFrame;
            if (_deathFrame >= deathFrames) {
                // done → actually explode & remove
                Enemy::Hit(hp);  // hp<=0 triggers cleanup
            }
        }
        return;  // skip movement while dying
    }

    // ── Dash logic ──────────────────────────────────────
    dashTimer += deltaTime;
    if (dashRemaining > 0.0f) {
        dashRemaining -= deltaTime;
        if (dashRemaining <= 0.0f)
            speed = originalSpeed * (enraged ? 1.5f : 1.0f);
    } else if (dashTimer >= dashCooldown) {
        dashTimer     = 0.0f;
        dashRemaining = dashDuration;
        speed = originalSpeed * (enraged ? 1.5f : 1.0f) * 2.0f;
    }

    // ── Movement & animation ─────────────────────────────
    Enemy::Update(deltaTime);

    // ── Orientation: no vertical rotate, just mirror horizontally ──
    Rotation = 0.0f;
    float w = std::fabs(Size.x);
    Size.x = (Velocity.x < 0.0f ? -w : w);
}

void WolfEnemy::Draw() const {
    if (_isDying) {
        // draw death frame from second row of skeleton.png
        auto bmpPtr = Engine::Resources::GetInstance()
                          .GetBitmap("play/skeleton.png");
        ALLEGRO_BITMAP* bmp = bmpPtr.get();
        int cols = deathCols, rows = 5;
        int fw = al_get_bitmap_width(bmp)  / cols;
        int fh = al_get_bitmap_height(bmp) / rows;
        int sx = _deathFrame * fw;
        int sy = deathRowIndex * fh;

        float scale = 1.7f;
        int dw = int(fw * scale);
        int dh = int(fh * scale);

        al_draw_scaled_bitmap(
            bmp,
            sx, sy, fw, fh,
            Position.x - dw/2.0f,
            Position.y - dh/2.0f,
            dw, dh,
            0
        );
    } else {
        Enemy::Draw();
    }
}
