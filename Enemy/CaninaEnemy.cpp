#include "CaninaEnemy.hpp"
#include <algorithm>
#include <cmath>
#include <allegro5/allegro_primitives.h>
#include "Scene/PlayScene.hpp"
#include "Engine/Resources.hpp"

extern std::vector<Enemy*> g_enemies;

CaninaEnemy::CaninaEnemy(int x, int y)
  : Enemy("play/wolf.png", x, y, 20, 100, 30, 20),
    originalHP(hp)
{
    SetAnimation(4, 2, 10.0f);
    SetFrameSequence({0,1,2,3,6,7});
    float fw = GetBitmapWidth()  / 4.0f;
    float fh = GetBitmapHeight() / 2.0f;
    Size.x = fw * drawScale;
    Size.y = fh * drawScale;
}

void CaninaEnemy::Hit(float damage) {
    if (_isDying) return;

    // if this hit would kill, start death animation
    if (hp - damage <= 0.0f) {
        _isDying         = true;
        _deathFrame      = 0;
        _deathFrameTimer = 0.0f;
        return;
    }
    // else normal damage
    Enemy::Hit(damage);
}

void CaninaEnemy::Update(float deltaTime) {
    _shieldTimer += deltaTime;
    if (_shieldTimer >= shieldFrameDuration) {
        _shieldTimer -= shieldFrameDuration;
        _shieldFrame = (_shieldFrame + 1) % (shieldCols * shieldRows);
    }
    if (_isDying) {
        // advance death-animation
        _deathFrameTimer += deltaTime;
        if (_deathFrameTimer >= deathFrameDuration) {
            _deathFrameTimer -= deathFrameDuration;
            ++_deathFrame;
            if (_deathFrame >= deathFrames) {
                // once finished, finally explode and remove
                Enemy::Hit(hp);
            }
        }
        return;
    }

    // ─── Healing Aura ───────────────────
    healTimer += deltaTime;
    if (healTimer >= healInterval) {
        healTimer -= healInterval;
        hp = std::min(originalHP, hp + healAmount);
        for (auto obj : getPlayScene()->EnemyGroup->GetObjects()) {
            Enemy* e = dynamic_cast<Enemy*>(obj);
            if (e == this) continue;
            float dx = e->Position.x - Position.x;
            float dy = e->Position.y - Position.y;
            if (dx*dx + dy*dy <= healRadius*healRadius) {
                e->Hit(-healAmount);
            }
        }
    }

    // movement & facing
    Enemy::Update(deltaTime);
    float vx = Velocity.x, vy = Velocity.y;
    if (std::fabs(vy) > std::fabs(vx)) {
        Rotation = (vy > 0 ? ALLEGRO_PI/2 : -ALLEGRO_PI/2);
        Size.x   = std::fabs(Size.x);
    } else {
        Rotation = 0.0f;
        float w  = std::fabs(Size.x);
        Size.x   = (vx < 0 ? -w : w);
    }
}

void CaninaEnemy::Draw() const {
    if (_isDying) {
        // death animation
        auto bmpPtr = Engine::Resources::GetInstance()
                          .GetBitmap("play/caninedeath.png");
        ALLEGRO_BITMAP* bmp = bmpPtr.get();
        int fw = al_get_bitmap_width(bmp)  / deathCols;
        int fh = al_get_bitmap_height(bmp) / deathRows;
        int sx = (_deathFrame % deathCols) * fw;
        int sy = (_deathFrame / deathCols) * fh;
        int dw = int(fw * drawScale);
        int dh = int(fh * drawScale);
        al_draw_scaled_bitmap(
            bmp,
            sx, sy, fw, fh,
            Position.x - dw/2.0f,
            Position.y - dh/2.0f,
            dw, dh,
            0
        );
        return;
    }

    // 1) Animated shield sprite
    {
        auto bmpPtr = Engine::Resources::GetInstance()
                          .GetBitmap("play/greenshield.png");
        ALLEGRO_BITMAP* bmp = bmpPtr.get();

        int fw = al_get_bitmap_width(bmp)  / shieldCols;  // =128
        int fh = al_get_bitmap_height(bmp) / shieldRows;  // =128

        // pick the slowed frame:
        int sx = (_shieldFrame % shieldCols) * fw;
        int sy = (_shieldFrame / shieldCols) * fh;

        float scale = 2.0f;  // same as before
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
    }

    // 2) Translucent green circle aura
    al_draw_filled_circle(
      Position.x, Position.y,
      healRadius,
      al_map_rgba(0, 255, 0, 64)
    );

    // 3) Wolf sprite
    Enemy::Draw();
}
