// SlimeEnemy.hpp
#ifndef SLIMEENEMY_HPP
#define SLIMEENEMY_HPP

#include "Enemy.hpp"

class SlimeEnemy : public Enemy {
public:
    SlimeEnemy(int x, int y);

    void Update(float deltaTime) override;
    void Draw()   const  override;
    void Hit(float damage) override;

private:
    // ─── regeneration ────────────────────────
    float regenCooldown = 2.0f;
    float regenTimer    = 0.0f;
    float regenAmount   = 5.0f;
    float originalHP;

    bool enraged = false;

    // ─── death animation ──────────────────────
    bool   _isDying         = false;
    int    _deathFrame      = 0;      // 0…5
    float  _deathFrameTimer = 0.0f;
    static constexpr int   deathCols         = 4;
    static constexpr int   deathRows         = 2;
    static constexpr int   deathFrames       = 6;
    static constexpr float deathFrameDuration= 0.08f; // seconds per frame
};

#endif // SLIMEENEMY_HPP
