#include "BatEnemy.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/Resources.hpp"
#include <allegro5/allegro_primitives.h>

BatEnemy::BatEnemy(int x, int y)
  : Enemy(
      /*img=*/    "play/bat.png",
      /*x,y=*/    x, y,
      /*radius=*/ 20,
      /*speed=*/  150,
      /*hp=*/     3,
      /*money=*/  8
    )
{
    // 4×1 flap animation @14fps
    SetAnimation(4, 1, 14.0f);

    // draw at native frame size * 2
    float fw = GetBitmapWidth()  / 4.0f;  // =256/4=64
    float fh = GetBitmapHeight()        ; // =64/1=64
    Size.x = fw * drawScale;
    Size.y = fh * drawScale;
}

void BatEnemy::Hit(float damage) {
    if (_isDying) return;  // already in death sequence

    // if this hit would kill:
    if (hp - damage <= 0.0f) {
        // start death animation (don’t remove yet)
        _isDying         = true;
        _deathFrame      = 0;
        _deathFrameTimer = 0.0f;
        return;
    }
    // otherwise normal damage
    Enemy::Hit(damage);
}

void BatEnemy::Update(float deltaTime) {
    if (_isDying) {
        // advance death frames
        _deathFrameTimer += deltaTime;
        if (_deathFrameTimer >= deathFrameDuration) {
            _deathFrameTimer -= deathFrameDuration;
            ++_deathFrame;
            if (_deathFrame >= deathFrames) {
                // once finished, trigger real death
                Enemy::Hit(hp);
            }
        }
    } else {
        // still alive → normal movement & flap
        Enemy::Update(deltaTime);
    }
}

void BatEnemy::Draw() const {
    if (_isDying) {
        // draw death animation scaled 2×
        auto bmpPtr = Engine::Resources::GetInstance()
                          .GetBitmap("play/batdeath.png");
        ALLEGRO_BITMAP* bmp = bmpPtr.get();
        int fw = al_get_bitmap_width(bmp)  / deathCols;
        int fh = al_get_bitmap_height(bmp) / deathRows;
        int sx = (_deathFrame % deathCols) * fw;
        int sy = (_deathFrame / deathCols) * fh;
        int drawW = int(fw * drawScale);
        int drawH = int(fh * drawScale);
        al_draw_scaled_bitmap(
            bmp,
            sx, sy, fw, fh,
            Position.x - drawW/2.0f,
            Position.y - drawH/2.0f,
            drawW, drawH,
            0
        );
    } else {
        // normal bat sprite is already set to Size = native*2
        Enemy::Draw();
    }
}
