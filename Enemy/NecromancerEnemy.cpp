// NecromancerEnemy.cpp
#include "NecromancerEnemy.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include "Scene/PlayScene.hpp"
#include "Engine/Resources.hpp"
NecromancerEnemy::NecromancerEnemy(int x, int y)
    : Enemy(
        /*img=*/    "play/necromancer.png",
        /*x,y=*/    x, y,
        /*radius=*/ 25,
        /*speed=*/  60,
        /*hp=*/     80,
        /*money=*/  35
      )
{
    // 2760×896 ⇒ 12 cols × 7 rows @ 8fps
    SetAnimation(17, 7, 8.0f);

    // third row only: indices 2*12…2*12+11 = 24…35
    SetFrameSequence({
        34,35,36,37,38,39,40,41,42,43,44,45,46
    });

    // scale if desired
    float fw = GetBitmapWidth()  / 17.0f;  // =230
    float fh = GetBitmapHeight() /  7.0f;  // =128
    Size.x = fw * 1.2f;
    Size.y = fh * 1.2f;
}

void NecromancerEnemy::Update(float deltaTime) {
    // 1) Do normal movement & animation
    Enemy::Update(deltaTime);

    // 2) After everyone has moved this frame, buff any inside the aura
    auto scene = getPlayScene();
    for (auto obj : scene->EnemyGroup->GetObjects()) {
        Enemy* e = dynamic_cast<Enemy*>(obj);
        if (!e || e == this) continue;

        float dx = e->Position.x - Position.x;
        float dy = e->Position.y - Position.y;
        if (dx*dx + dy*dy <= auraRadius * auraRadius) {
            // extra displacement = (speedBuff-1)×their Velocity×dt
            e->Position.x += e->Velocity.x * (speedBuff - 1.0f) * deltaTime;
            e->Position.y += e->Velocity.y * (speedBuff - 1.0f) * deltaTime;
        }
    }

    // 3) Keep your own facing logic
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

void NecromancerEnemy::Draw() const {
    // dark red circle (optional)
    al_draw_filled_circle(
      Position.x, Position.y,
      auraRadius,
      al_map_rgba(128, 0, 0, 96)
    );

    // FETCH the shared_ptr, then .get() to raw pointer:
    auto auraBmpPtr = Engine::Resources::GetInstance()
                        .GetBitmap("play/redaura.png");
    ALLEGRO_BITMAP* auraBmp = auraBmpPtr.get(); 

    // Now draw the aura frames as before…
    float scale = 2.0f;
    const int cols = 5, rows = 3;
    int fw = al_get_bitmap_width(auraBmp) / cols;
    int fh = al_get_bitmap_height(auraBmp) / rows;
    int drawW = int(fw * scale);
    int drawH = int(fh * scale);
    float drawX = Position.x - drawW/2.0f;
    float drawY = Position.y - drawH/2.0f;
    static int auraFrame = 0;
    auraFrame = (auraFrame + 1) % (cols * rows);
    int sx = (auraFrame % cols) * fw;
    int sy = (auraFrame / cols) * fh;
    al_draw_scaled_bitmap(
  auraBmp,                // your ALLEGRO_BITMAP*
  sx, sy, fw, fh,         // src region
  drawX, drawY,           // dest top‐left
  drawW, drawH,           // dest size
  0                       // flags
);

    // Finally draw the necromancer on top
    Enemy::Draw();
}

