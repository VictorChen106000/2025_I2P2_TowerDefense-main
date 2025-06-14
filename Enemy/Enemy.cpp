#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
#include <random>
#include <string>
#include <vector>

#include "Bullet/Bullet.hpp"
#include "Enemy.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/IScene.hpp"
#include "Engine/LOG.hpp"
#include "Scene/PlayScene.hpp"
#include "Turret/Turret.hpp"
#include "UI/Animation/DirtyEffect.hpp"
#include "UI/Animation/ExplosionEffect.hpp"

#include "Enemy/SoldierEnemy.hpp"
#include "Enemy/TankEnemy.hpp"
#include "Enemy/BigTankEnemy.hpp"
#include "Enemy/CaninaEnemy.hpp"
#include "Enemy/DemonEnemy.hpp"
#include "Enemy/NecromancerEnemy.hpp"
#include "Enemy/FlyEnemy.hpp"
#include "Enemy/GolemEnemy.hpp"
#include "Enemy/PlaneEnemy.hpp"
#include "Enemy/SlimeEnemy.hpp"
#include "Enemy/WolfEnemy.hpp"
#include "Enemy/BatEnemy.hpp"
#include "Enemy/SorcererEnemy.hpp"


std::vector<Enemy*> g_enemies;

PlayScene *Enemy::getPlayScene() {
    return dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetActiveScene());
}
void Enemy::OnExplode() {
    getPlayScene()->EffectGroup->AddNewObject(new ExplosionEffect(Position.x, Position.y));
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distId(1, 3);
    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 20);
    for (int i = 0; i < 10; i++) {
        // Random add 10 dirty effects.
        getPlayScene()->GroundEffectGroup->AddNewObject(new DirtyEffect("play/dirty-" + std::to_string(distId(rng)) + ".png", dist(rng), Position.x, Position.y));
    }
}
Enemy::Enemy(std::string img, float x, float y, float radius, float speed, float hp, int money) : Engine::Sprite(img, x, y), speed(speed), hp(hp), money(money) {
    CollisionRadius = radius;
    reachEndTime = 0;
}
void Enemy::Hit(float damage) {

    if (damage > 0 && !dynamic_cast<CaninaEnemy*>(this)) {
        auto scene = getPlayScene();
        for (auto obj : scene->EnemyGroup->GetObjects()) {
            auto *c = dynamic_cast<CaninaEnemy*>(obj);
            if (!c || c == this)     // skip non-Caninas and skip self
                continue;

            float dx = Position.x - c->Position.x;
            float dy = Position.y - c->Position.y;
            if (dx*dx + dy*dy <= c->healRadius * c->healRadius)
                return;  // inside someone else's Canina aura → ignore damage
        }
    }

    hp -= damage;
    if (hp <= 0) {
        // 1) Explosion
        

        // 2) Scene bookkeeping
        auto scene = getPlayScene();
        scene->AddKill();
        scene->EarnMoney(money);

        // 3) Coin logic
        if (auto s = dynamic_cast<SlimeEnemy*>(this)) {
    // only advance the mission while we're below the goal
    if (scene->soldierkillcount < PlayScene::KILLS_PER_COIN) {
        scene->soldierkillcount++;
        scene->UpdateKillBar();
        // only give the one coin, when you *just* reached 3
        if (scene->soldierkillcount == PlayScene::KILLS_PER_COIN) {
            scene->EarnCoin(1);
        }
    }
}
        else if (auto w = dynamic_cast<WolfEnemy*>(this)) {
        if (scene->wolfKillCount < PlayScene::KILLS_PER_COIN) {
            scene->wolfKillCount++;
            scene->UpdateWolfBar();
            if (scene->wolfKillCount == PlayScene::KILLS_PER_COIN) {
            scene->EarnCoin(1);
        }
        }
    }

        // 4) Cleanup
        for (auto &it : lockedTurrets) it->Target = nullptr;
        for (auto &it : lockedBullets) it->Target = nullptr;
    
        scene->EnemyGroup->RemoveObject(objectIterator);
        AudioHelper::PlayAudio("explosion.wav");
    }
}

void Enemy::UpdatePath(const std::vector<std::vector<int>> &mapDistance) {
    int x = static_cast<int>(floor(Position.x / PlayScene::BlockSize));
    int y = static_cast<int>(floor(Position.y / PlayScene::BlockSize));
    if (x < 0) x = 0;
    if (x >= PlayScene::MapWidth) x = PlayScene::MapWidth - 1;
    if (y < 0) y = 0;
    if (y >= PlayScene::MapHeight) y = PlayScene::MapHeight - 1;
    Engine::Point pos(x, y);
    int num = mapDistance[y][x];
    if (num == -1) {
        num = 0;
        Engine::LOG(Engine::ERROR) << "Enemy path finding error";
    }
    path = std::vector<Engine::Point>(num + 1);
    while (num != 0) {
        std::vector<Engine::Point> nextHops;
        for (auto &dir : PlayScene::directions) {
            int x = pos.x + dir.x;
            int y = pos.y + dir.y;
            if (x < 0 || x >= PlayScene::MapWidth || y < 0 || y >= PlayScene::MapHeight || mapDistance[y][x] != num - 1)
                continue;
            nextHops.emplace_back(x, y);
        }
        // Choose arbitrary one.
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist(0, nextHops.size() - 1);
        pos = nextHops[dist(rng)];
        path[num] = pos;
        num--;
    }
    path[0] = getPlayScene()->EndGridPoints.front();

}
void Enemy::Update(float deltaTime) {
    // Pre-calculate the velocity.
    float remainSpeed = speed * deltaTime;
    while (remainSpeed != 0) {
        if (path.empty()) {
            // Reach end point.
            Hit(hp);
            getPlayScene()->Hit();
            reachEndTime = 0;
            return;
        }
        Engine::Point target = path.back() * PlayScene::BlockSize + Engine::Point(PlayScene::BlockSize / 2, PlayScene::BlockSize / 2);
        Engine::Point vec = target - Position;
        // Add up the distances:
        // 1. to path.back()
        // 2. path.back() to border
        // 3. All intermediate block size
        // 4. to end point
        reachEndTime = (vec.Magnitude() + (path.size() - 1) * PlayScene::BlockSize - remainSpeed) / speed;
        Engine::Point normalized = vec.Normalize();
        if (remainSpeed - vec.Magnitude() > 0) {
            Position = target;
            path.pop_back();
            remainSpeed -= vec.Magnitude();
        } else {
            Velocity = normalized * remainSpeed / deltaTime;
            remainSpeed = 0;
        }
    }
    Rotation = atan2(Velocity.y, Velocity.x);
    Sprite::Update(deltaTime);
}
void Enemy::Draw() const {
    Sprite::Draw();
    if (PlayScene::DebugMode) {
        // Draw collision radius.
        al_draw_circle(Position.x, Position.y, CollisionRadius, al_map_rgb(255, 0, 0), 2);
    }
}
