#ifndef NECROMANCERENEMY_HPP
#define NECROMANCERENEMY_HPP

#include "Enemy.hpp"

class NecromancerEnemy : public Enemy {
public:
    NecromancerEnemy(int x, int y);

    void Hit(float damage) override;       // intercept dying
    void Update(float deltaTime) override;
    void Draw()   const override;

private:
    // ─── Aura animation ────────────────────────────────
    static constexpr float auraRadius   = 90.0f;
    static constexpr float speedBuff    = 1.99f;
    static constexpr int   auraCols     = 5;
    static constexpr int   auraRows     = 3;
    static constexpr float auraFrameDur = 0.15f;   // seconds per aura frame
    int   _auraFrame = 0;
    float _auraTimer = 0.0f;

    // ─── Death animation ───────────────────────────────
    bool   _isDying         = false;
    int    _deathFrame      = 0;          // 0..8
    float  _deathTimer      = 0.0f;
    static constexpr int   deathFrames       = 9;    // first 9 frames of row 6
    static constexpr float deathFrameDur     = 0.053f; // seconds per death frame
};

#endif // NECROMANCERENEMY_HPP
