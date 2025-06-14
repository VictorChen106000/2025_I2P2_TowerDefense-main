// CaninaEnemy.cpp
#include "CaninaEnemy.hpp"
#include <algorithm>
#include <cmath>
#include <allegro5/allegro_primitives.h>
#include "Scene/PlayScene.hpp"
#include "Enemy.hpp"
#include "Engine/Resources.hpp"

// external list of all enemies in the scene
extern std::vector<Enemy*> g_enemies;

CaninaEnemy::CaninaEnemy(int x, int y)
    : Enemy(
        /*img=*/    "play/wolf.png",  
        /*x,y=*/    x, y,
        /*radius=*/ 20,
        /*speed=*/  100,
        /*hp=*/     30,
        /*money=*/  20
      ),
      originalHP(hp)
{
    SetAnimation(4, 2, 10.0f);
    SetFrameSequence({0, 1, 2, 3, 6, 7});

    float fw = GetBitmapWidth()  / 4.0f;  
    float fh = GetBitmapHeight() / 2.0f;  
    Size.x = fw * 1.7f;
    Size.y = fh * 1.7f;
}

void CaninaEnemy::Update(float deltaTime) {
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
    // 1) Animated shield sprite under everything
    {
        auto bmpPtr = Engine::Resources::GetInstance()
                          .GetBitmap("play/greenshield.png");
        ALLEGRO_BITMAP* bmp = bmpPtr.get();

        const int cols = 4, rows = 4;
        int fw = al_get_bitmap_width(bmp)  / cols;  // 512/4 =128
        int fh = al_get_bitmap_height(bmp) / rows;  // 512/4 =128

        static int frame = 0;
        frame = (frame + 1) % (cols * rows);

        int sx = (frame % cols) * fw;
        int sy = (frame / cols) * fh;

        float scale = 2.0f;  // match your wolf scaling
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

    // 3) Finally the wolf sprite
    Enemy::Draw();
}
