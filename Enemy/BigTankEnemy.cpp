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
    bool willDie = (hp - damage) <= 0;
    Enemy::Hit(damage);
    if (!willDie) return;

    auto scene = getPlayScene();
    const float cx = Position.x, cy = Position.y;

    const float boostTime = 0.6f;       // how long the extra speed lasts
    const float baseSpeed = 50.0f;      // must match your SoldierEnemy ctor
    const float spacing   = baseSpeed * boostTime;
    const float deltaSpd  = spacing / boostTime; // == baseSpeed

    for (int i = 0; i < 4; ++i) {
        auto *soldier = new SoldierEnemy((int)cx, (int)cy);
        scene->EnemyGroup->AddNewObject(soldier);

        soldier->UpdatePath(scene->mapDistance);

        // front gets 3×, then 2×, then 1×, last gets 0
        float bonus = (3 - i) * deltaSpd;
        if (bonus > 0) soldier->SetSpeedBoost(bonus, boostTime);
    }
}