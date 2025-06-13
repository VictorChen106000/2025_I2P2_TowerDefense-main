#include "DemonEnemy.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/Resources.hpp"
#include "Enemy/BatEnemy.hpp"
#include <allegro5/allegro_primitives.h>
#include <cmath>

// global list of enemies
extern std::vector<Enemy*> g_enemies;

DemonEnemy::DemonEnemy(int x, int y)
  : Enemy("play/demonfly.png", x, y, /*radius=*/25, /*speed=*/50, /*hp=*/300, /*money=*/20)
{
    // flying animation: 4×1 @8fps
    SetAnimation(4, 1, 8.0f);
    // native frame
    float fw = GetBitmapWidth()  / 4.0f;  
    float fh = GetBitmapHeight() / 1.0f;  
    Size.x = fw;
    Size.y = fh;
}

void DemonEnemy::Hit(float damage) {
    // if already playing death anim, ignore further hits
    if (_isDying) return;

    bool dying = (hp - damage) <= 0.0f;
    if (dying) {
        // 1) spawn the 5 bats
        auto scene = getPlayScene();
        const float spacing = 30.0f;
        for (int i = -2; i <= 2; ++i) {
            auto* bat = new BatEnemy(
                static_cast<int>(Position.x + i * spacing),
                static_cast<int>(Position.y)
            );
            scene->EnemyGroup->AddNewObject(bat);
            bat->UpdatePath(scene->mapDistance);
            g_enemies.push_back(bat);
        }
        // 2) kick off death-animation instead of instantly dying
        _isDying = true;
        _deathFrame = 0;
        _deathFrameTimer = 0;
        return; 
    }

    // otherwise normal damage
    Enemy::Hit(damage);
}

void DemonEnemy::Update(float deltaTime) {
    auto scene = getPlayScene();

    if (_isDying) {
        // advance death animation
        _deathFrameTimer += deltaTime;
        if (_deathFrameTimer >= deathFrameDuration) {
            _deathFrameTimer -= deathFrameDuration;
            _deathFrame++;
            if (_deathFrame >= deathFrames) {
                // animation finished → now really die
                Enemy::Hit(hp);  // hp <= 0 now triggers removal, explosion, etc.
            }
        }
        return;  // skip movement while dying
    }

    // still alive: spawn bats periodically
    spawnTimer += deltaTime;
    if (spawnTimer >= spawnCooldown) {
        spawnTimer -= spawnCooldown;
        auto* bat = new BatEnemy(
            static_cast<int>(Position.x),
            static_cast<int>(Position.y)
        );
        scene->EnemyGroup->AddNewObject(bat);
        bat->UpdatePath(scene->mapDistance);
        g_enemies.push_back(bat);
    }

    // movement & animation
    Enemy::Update(deltaTime);
    // face cardinally
    Rotation = 0.0f;
    float w = std::fabs(Size.x);
    if (Velocity.x < 0)
         Size.x = -w;   // face left
    else
        Size.x =  w;   // face right
}

void DemonEnemy::Draw() const {
    if (_isDying) {
        // draw death-sheet frame
        auto bmpPtr = Engine::Resources::GetInstance()
                          .GetBitmap("play/demondeath.png");
        ALLEGRO_BITMAP* bmp = bmpPtr.get();
        int cols = deathFrames, rows = 1;
        int fw = al_get_bitmap_width(bmp)  / cols;
        int fh = al_get_bitmap_height(bmp) / rows;
        int sx = _deathFrame * fw;
        int sy = 0;
        // center the frame on its Position
        al_draw_scaled_bitmap(
            bmp,
            sx, sy, fw, fh,
            Position.x - fw/2.0f,
            Position.y - fh/2.0f,
            fw, fh,
            0
        );
        return;
    }
    // otherwise draw normal demon
    Enemy::Draw();
}
