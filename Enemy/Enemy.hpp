#ifndef ENEMY_HPP
#define ENEMY_HPP
#include <list>
#include <string>
#include <vector>

#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"

class Bullet;
class PlayScene;
class Turret;

class Enemy : public Engine::Sprite {
protected:
    std::vector<Engine::Point> path;
    float speed;
    float hp;
    int money;
    PlayScene *getPlayScene();
    virtual void OnExplode();

public:
     // ── NEW: a strongly-typed ID for each enemy kind ────────────────
   enum class Type : int {
     Pawn1 = 1, Pawn2 = 2, Pawn3 = 3,
     Strong4 = 4, Strong5 = 5, Strong6 = 6,
     Boss7  = 7, Boss8  = 8, soldier = 9, bigtank = 10, bat = 11
   };

   // ask any Enemy what kind it is
   virtual Type GetType() const = 0;
    float reachEndTime;
    std::list<Turret *> lockedTurrets;
    std::list<Bullet *> lockedBullets;
    Enemy(std::string img, float x, float y, float radius, float speed, float hp, int money);
    virtual void Hit(float damage);
    void UpdatePath(const std::vector<std::vector<int>> &mapDistance);
    void Update(float deltaTime) override;
    void Draw() const override;
    float GetHP()  const { return hp; }
    void  SetHP(float v) { hp = v; }
};
#endif   // ENEMY_HPP
