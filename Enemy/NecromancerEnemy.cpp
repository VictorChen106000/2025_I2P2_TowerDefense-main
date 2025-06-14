#include "NecromancerEnemy.hpp"
#include <allegro5/allegro_primitives.h>
#include "Scene/PlayScene.hpp"
#include "Engine/Resources.hpp"
#include <cmath>

NecromancerEnemy::NecromancerEnemy(int x, int y)
  : Enemy("play/necromancer.png", x, y, 25, 60, 400, 35)
{
    // walk animation: 17×7 @8fps, use row 2 only for movement
    SetAnimation(17, 7, 8.0f);
    SetFrameSequence({ 34,35,36,37,38,39,40,41,42,43,44,45,46 });
    float fw = GetBitmapWidth()  / 17.0f;
    float fh = GetBitmapHeight() /  7.0f;
    Size.x = fw * 1.8f;
    Size.y = fh * 1.8f;
}

void NecromancerEnemy::Hit(float damage) {
    if (_isDying) return;           // already playing death
    if (hp - damage <= 0.0f) {
        // start the death animation instead of instantly removing
        _isDying    = true;
        _deathFrame = 0;
        _deathTimer = 0.0f;
        return;
    }
    // otherwise take damage normally
    Enemy::Hit(damage);
}

void NecromancerEnemy::Update(float deltaTime) {
    // ─── if dying, advance death frames ─────────────────
    if (_isDying) {
        _deathTimer += deltaTime;
        if (_deathTimer >= deathFrameDur) {
            _deathTimer -= deathFrameDur;
            ++_deathFrame;
            if (_deathFrame >= deathFrames) {
                // once done, call base Hit with full hp to trigger removal/explosion
                Enemy::Hit(hp);
            }
        }
        return; // skip everything else while dying
    }

    // ─── otherwise, aura animation + buff + movement ────
    _auraTimer += deltaTime;
    if (_auraTimer >= auraFrameDur) {
        _auraTimer -= auraFrameDur;
        _auraFrame = (_auraFrame + 1) % (auraCols * auraRows);
    }

    // buff allies in the aura
    auto scene = getPlayScene();
    for (auto obj : scene->EnemyGroup->GetObjects()) {
        Enemy* e = dynamic_cast<Enemy*>(obj);
        if (!e || e == this) continue;
        float dx = e->Position.x - Position.x;
        float dy = e->Position.y - Position.y;
        if (dx*dx + dy*dy <= auraRadius * auraRadius) {
            e->Position.x += e->Velocity.x * (speedBuff - 1.0f) * deltaTime;
            e->Position.y += e->Velocity.y * (speedBuff - 1.0f) * deltaTime;
        }
    }

    // do your normal movement & facing
    Enemy::Update(deltaTime);
    Rotation = 0.0f;
    float w = std::fabs(Size.x);
    Size.x = (Velocity.x < 0 ? -w : w);
}

void NecromancerEnemy::Draw() const {
    // ─── death frame ───────────────────────────────────
    if (_isDying) {
        auto bmpPtr = Engine::Resources::GetInstance()
                          .GetBitmap("play/necromancer.png");
        ALLEGRO_BITMAP* bmp = bmpPtr.get();
        // sheet is 17×7, we want row index 6
        constexpr int cols = 17, rows = 7;
        int fw = al_get_bitmap_width(bmp)  / cols;
        int fh = al_get_bitmap_height(bmp) / rows;
        int sx = _deathFrame * fw;
        int sy = 6 * fh;  // row 6 (zero-based: 0..6)

        float scale = 1.8f;
        int dw = int(fw * scale);
        int dh = int(fh * scale);

        // lift the sprite by the same hoverOffset you use for the normal draw
        const float deathOffset = 45.0f;

        al_draw_scaled_bitmap(
            bmp,
            sx, sy, fw, fh,
            // subtract half‐width, half‐height, and extra offset:
            Position.x - dw/2.0f,
            (Position.y - dh/2.0f) - deathOffset,
            dw, dh,
            0
        );
        return;
    }
    // ─── aura circle ───────────────────────────────────
    al_draw_filled_circle(
        Position.x, Position.y,
        auraRadius,
        al_map_rgba(128, 0, 0, 96)
    );

    // 2) aura sprite at true Position
    auto auraBmpPtr = Engine::Resources::GetInstance()
                          .GetBitmap("play/redaura.png");
    ALLEGRO_BITMAP* auraBmp = auraBmpPtr.get();
    int fw = al_get_bitmap_width(auraBmp)  / auraCols;
    int fh = al_get_bitmap_height(auraBmp) / auraRows;
    int sx = (_auraFrame % auraCols) * fw;
    int sy = (_auraFrame / auraCols) * fh;
    float auraScale = 1.5f;
    int dw = int(fw * auraScale);
    int dh = int(fh * auraScale);
    al_draw_scaled_bitmap(
        auraBmp,
        sx, sy, fw, fh,
        Position.x - dw/2.0f,
        Position.y - dh/2.0f,
        dw, dh,
        0
    );

    // 3) draw the necromancer **hovering** above its Position
    const float hoverOffset = 45.0f;            // how many pixels to lift
    auto self = const_cast<NecromancerEnemy*>(this);
    self->Position.y -= hoverOffset;
      Enemy::Draw();  // now drawn slightly higher
    self->Position.y += hoverOffset;
}
