#include <string>
#include <array>
#include <utility>
#include <random>
#include "BigTankEnemy.hpp"
#include "Enemy/SoldierEnemy.hpp"
#include "Scene/PlayScene.hpp"

BigTankEnemy::BigTankEnemy(int x, int y)
    : Enemy("play/enemy-6.png", x, y, 40, 40, 200, 30) {}

// Override the base Hit to inject our spawn logic
void BigTankEnemy::Hit(float damage) {
    // 1) Will this hit kill it?
    bool willDie = (hp - damage) <= 0;

    // 2) First, apply base logic (subtract hp, explode, remove, etc.)
    Enemy::Hit(damage);  // hp -= damage; if (hp<=0) { … } :contentReference[oaicite:0]{index=0}

    // 3) If it did die, spawn 4 Soldiers at its last position
    if (willDie) {
        auto scene = getPlayScene();
        const float cx = Position.x, cy = Position.y;
        // pick four offsets in a little square around the center
        const float d = PlayScene::BlockSize * 0.3f; // tweak radius as you like
        std::array<std::pair<float,float>,4> offs = {{
            {-d, -d},
            {+d, -d},
            {+d, +d},
            {-d, +d},
        }};

        // how much time to stagger each spawn (in seconds)
        const float timeStep = 0.2f;

        for (int i = 0; i < 4; ++i) {
            // 1) spawn in a little square
            float sx = cx + offs[i].first;
            float sy = cy + offs[i].second;
            auto *soldier = new SoldierEnemy(int(sx), int(sy));
            scene->EnemyGroup->AddNewObject(soldier);

            // 2) give them the correct path to the exit
            soldier->UpdatePath(scene->mapDistance);

            // 3) advance each one by i*timeStep seconds
            soldier->Update(i * timeStep);
        }
    }
}