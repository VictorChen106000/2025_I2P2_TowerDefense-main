// DemonEnemy.cpp
#include "DemonEnemy.hpp"
#include "Scene/PlayScene.hpp"
#include "Engine/GameEngine.hpp"
#include <cmath>

DemonEnemy::DemonEnemy(int x, int y)
  : Enemy("play/demonfly.png",
          x, y,
          /*radius=*/25,
          /*speed=*/50,
          /*hp=*/300,
          /*money=*/20)
{
    // flying animation: 4×1 @8 fps
    SetAnimation(4, 1, 8.0f);

    // draw at native frame size
    float fw = GetBitmapWidth()  / 4.0f;  // =324/4=81
    float fh = GetBitmapHeight() / 1.0f;  // =71/1=71
    Size.x = fw;
    Size.y = fh;
}

void DemonEnemy::Update(float deltaTime) {
    auto scene = getPlayScene();

    // only spawn bats while still alive
    if (hp > 0.0f) {
        spawnTimer += deltaTime;
        if (spawnTimer >= spawnCooldown) {
            spawnTimer -= spawnCooldown;

            auto* bat = new BatEnemy(
                static_cast<int>(Position.x),
                static_cast<int>(Position.y)
            );
            scene->EnemyGroup->AddNewObject(bat);
            bat->UpdatePath(scene->mapDistance);
        }
    }

    // movement & animation
    Enemy::Update(deltaTime);

    // face vertically or horizontally
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

void DemonEnemy::Hit(float damage) {
    bool dying = (hp - damage) <= 0.0f;

    if (dying) {
        // spawn 5 bats in a horizontal line, spaced ±30px
        auto scene = getPlayScene();
        const float spacing = 30.0f;
        for (int i = -2; i <= 2; ++i) {
            auto* bat = new BatEnemy(
                static_cast<int>(Position.x + i * spacing),
                static_cast<int>(Position.y)
            );
            scene->EnemyGroup->AddNewObject(bat);
            bat->UpdatePath(scene->mapDistance);
        }
    }

    // now let the base class handle hp reduction, kill count, and removal
    Enemy::Hit(damage);
}
