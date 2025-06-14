// SlimeEnemy.cpp
#include "SlimeEnemy.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/Resources.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>

SlimeEnemy::SlimeEnemy(int x, int y)
    : Enemy("play/slimejump.png", x, y, 20, 50, 40, 10),
      originalHP(hp)
{
    // 4×1 @8fps
    SetAnimation(4, 2, 8.0f);
    float frameW = GetBitmapWidth()  / 4.0f;  // =64
    float frameH = GetBitmapHeight() / 2.0f;  // =64
    Size.x = frameW * 1.75f;
    Size.y = frameH * 1.75f;
}

void SlimeEnemy::Update(float deltaTime) {
    if (_isDying) {
        // ─── advance death animation ─────────────────
        _deathFrameTimer += deltaTime;
        if (_deathFrameTimer >= deathFrameDuration) {
            _deathFrameTimer -= deathFrameDuration;
            ++_deathFrame;
            if (_deathFrame >= deathFrames) {
                // finally kill
                Enemy::Hit(hp);
            }
        }
    } else {
        // ─── regeneration ─────────────────────────────
        regenTimer += deltaTime;
        if (regenTimer >= regenCooldown) {
            regenTimer -= regenCooldown;
            hp += regenAmount;
            if (hp > originalHP) hp = originalHP;
        }

        // ─── movement + orientation ───────────────────
        Enemy::Update(deltaTime);
        float vx = Velocity.x, vy = Velocity.y;
         Rotation = 0.0f;
    float w = std::fabs(Size.x);
    Size.x = (Velocity.x < 0 ? -w : w);
    }
}

void SlimeEnemy::Draw() const {
    if (_isDying) {
        // draw one frame from slimedie.png (4×2 grid)
        auto bmpPtr = Engine::Resources::GetInstance()
                          .GetBitmap("play/slimedie.png");
        ALLEGRO_BITMAP* bmp = bmpPtr.get();
        int fw = al_get_bitmap_width(bmp)  / deathCols;
        int fh = al_get_bitmap_height(bmp) / deathRows;
        int sx = (_deathFrame % deathCols) * fw;
        int sy = (_deathFrame / deathCols) * fh;

        float scale = 1.75f;
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

void SlimeEnemy::Hit(float damage) {
    if (_isDying) return;

    bool willDie = (hp - damage) <= 0.0f;
    if (willDie) {
        // start death animation
        _isDying         = true;
        _deathFrame      = 0;
        _deathFrameTimer = 0.0f;
        return;
    }

    Enemy::Hit(damage);

    // enrage on low HP
    if (!willDie && !enraged && hp < originalHP * 0.5f) {
        enraged      = true;
        regenAmount *= 2.0f;
        regenCooldown *= 0.5f;
    }
}
