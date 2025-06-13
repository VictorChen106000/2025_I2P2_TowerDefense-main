#ifndef DEMONENEMY_HPP
#define DEMONENEMY_HPP

#include "Enemy.hpp"

class DemonEnemy : public Enemy {
public:
    DemonEnemy(int x, int y);

    void Update(float deltaTime) override;
    void Draw()   const override;
    void Hit(float damage) override;

private:
    // spawn bats while alive
    float spawnCooldown = 4.0f;
    float spawnTimer    = spawnCooldown;

    // ——— death animation state ———
    bool   _isDying         = false;
    int    _deathFrame      = 0;          // 0…6
    float  _deathFrameTimer = 0.0f;
    static constexpr int   deathFrames       = 7;
    static constexpr float deathFrameDuration= 0.1f;  // seconds per frame
};

#endif // DEMONENEMY_HPP
