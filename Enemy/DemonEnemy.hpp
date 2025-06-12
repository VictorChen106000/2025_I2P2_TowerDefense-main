// DemonEnemy.hpp
#ifndef DEMONENEMY_HPP
#define DEMONENEMY_HPP

#include <allegro5/allegro.h>
#include "Engine/Sprite.hpp"
#include "Enemy.hpp"
#include "BatEnemy.hpp"

/// DemonEnemy:
///  • Spawns a BatEnemy every 4s while HP > 0  
///  • On death: spawns 5 BatEnemies in a row, then dies.
class DemonEnemy : public Enemy {
public:
    DemonEnemy(int x, int y);

    void Update(float deltaTime) override;
    void Hit(float damage) override;  // override death to spawn 5 bats

private:
    float spawnCooldown = 4.0f;
    float spawnTimer    = spawnCooldown; // so first bat at ~4s
};

#endif // DEMONENEMY_HPP
