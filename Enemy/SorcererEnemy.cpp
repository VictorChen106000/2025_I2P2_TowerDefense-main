#include "SorcererEnemy.hpp"
#include "Scene/PlayScene.hpp"
#include "Turret/Turret.hpp"
#include "Engine/Resources.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>

// global list
extern std::vector<Enemy*> g_enemies;

SorcererEnemy::SorcererEnemy(int x, int y)
  : Enemy("play/sorcerer.png", x, y,
          /*radius=*/20, /*speed=*/70, /*hp=*/400, /*money=*/30)
{
    // walking sheet: 10×1 @6fps
    SetAnimation(10, 1, 6.0f);
    SetFrameSequence({0,1,2,3,4,5,6,7,8,9});

    float fw = GetBitmapWidth()  / 10.0f;
    float fh = GetBitmapHeight() /  1.0f;
    Size.x = fw * 1.0f;
    Size.y = fh * 1.0f;
}

void SorcererEnemy::Hit(float damage) {
    if (_isDying) return;
    if (hp - damage <= 0.0f) {
        // start death‐animation (necromancer row 6)
        _isDying    = true;
        _deathFrame = 0;
        _deathTimer = 0.0f;
        return;
    }
    Enemy::Hit(damage);
}

void SorcererEnemy::Update(float deltaTime) {
    auto scene = getPlayScene();

    // ─── death animation?
    if (_isDying) {
        _deathTimer += deltaTime;
        if (_deathTimer >= deathFrameDur) {
            _deathTimer -= deathFrameDur;
            ++_deathFrame;
            if (_deathFrame >= deathFrames) {
                // now finally die
                Enemy::Hit(hp);
            }
        }
        return;
    }

    // ─── 1) re-enable turrets leaving aura
    std::vector<Turret*> keep;
    keep.reserve(_disabled.size());
    for (auto* t : _disabled) {
        float dx = t->Position.x - Position.x;
        float dy = t->Position.y - Position.y;
        if (dx*dx + dy*dy <= empRadius*empRadius) {
            keep.push_back(t);
        } else {
            t->Enabled = true;
            t->Target  = nullptr;
        }
    }
    _disabled.swap(keep);

    // ─── 2) disable any turret newly inside
    for (auto obj : scene->TowerGroup->GetObjects()) {
        Turret* t = dynamic_cast<Turret*>(obj);
        if (!t) continue;
        float dx = t->Position.x - Position.x;
        float dy = t->Position.y - Position.y;
        if (dx*dx + dy*dy <= empRadius*empRadius && t->Enabled) {
            t->Enabled = false;
            t->Target  = nullptr;
            _disabled.push_back(t);
        }
    }

    // ─── 3) advance aura sprite
    _auraTimer += deltaTime;
    if (_auraTimer >= auraFrameDur) {
        _auraTimer -= auraFrameDur;
        _auraFrame = (_auraFrame + 1) % (auraCols * auraRows);
    }

    // ─── 4) normal movement & facing
    Enemy::Update(deltaTime);
    Rotation = 0.0f;
    float w = std::fabs(Size.x);
    Size.x = (Velocity.x < 0 ? -w : w);
}

void SorcererEnemy::Draw() const {
    // ─── death?
    if (_isDying) {
        auto bmpPtr = Engine::Resources::GetInstance().GetBitmap("play/necromancer.png");
        ALLEGRO_BITMAP* bmp = bmpPtr.get();
        constexpr int cols = 17, rows = 7;
        int fw = al_get_bitmap_width(bmp) / cols;
        int fh = al_get_bitmap_height(bmp) / rows;
        int sx = _deathFrame * fw;
        int sy = 6 * fh;  // row 6

        float deathScale = 1.8f;
        int dw = int(fw * deathScale);
        int dh = int(fh * deathScale);

        const float hoverOffset = 35.0f;
        al_draw_scaled_bitmap(
            bmp, sx, sy, fw, fh,
            Position.x - dw/2.0f,
            (Position.y - dh/2.0f) - hoverOffset,
            dw, dh,
            0
        );
        return;
    }

    // ─── 1) animated aura from shieldbiru.png ───────────
    auto auraPtr = Engine::Resources::GetInstance().GetBitmap("play/shieldbiru.png");
    ALLEGRO_BITMAP* auraBmp = auraPtr.get();
    int fw = al_get_bitmap_width(auraBmp)  / auraCols;
    int fh = al_get_bitmap_height(auraBmp) / auraRows;
    int sx = (_auraFrame % auraCols)*fw;
    int sy = (_auraFrame / auraCols)*fh;
    float scale = 2.8f;
    int dw = int(fw * scale), dh = int(fh * scale);
    al_draw_scaled_bitmap(
      auraBmp, sx, sy, fw, fh,
      Position.x - dw/2.0f,
      Position.y - dh/2.0f,
      dw, dh, 0
    );

    // 2) draw the EMP circle
    al_draw_filled_circle(
      Position.x, Position.y,
      empRadius,
      al_map_rgba(0, 0, 255, 64)
    );

    // 3) draw the Sorcerer *hovering*  — shift up by `hoverOffset`
    const float hoverOffset = 35.0f;  
    // we need to temporarily adjust Position.y, so cast away const:
    auto self = const_cast<SorcererEnemy*>(this);
    self->Position.y -= hoverOffset;
    Enemy::Draw();
    self->Position.y += hoverOffset;
}
