// WolfEnemy.hpp
#ifndef WOLFENEMY_HPP
#define WOLFENEMY_HPP

#include "Enemy.hpp"

class WolfEnemy : public Enemy {
public:
    WolfEnemy(int x, int y);

    void Update(float deltaTime) override;
    void Draw()   const  override;
    void Hit(float damage) override;
    Type GetType() const override { return Type::Pawn2; }

private:
    // ─── Dash & Enrage ───────────────────────────────────
    float dashCooldown   = 5.0f;
    float dashTimer      = 0.0f;
    float dashDuration   = 0.5f;
    float dashRemaining  = 0.0f;

    float originalSpeed;
    float originalHP;
    bool  enraged        = false;

    // ─── Death animation ──────────────────────────────────
    bool   _isDying         = false;
    int    _deathFrame      = 0;      // 0…12
    float  _deathFrameTimer = 0.0f;
    static constexpr int   deathCols         = 13;
    static constexpr int   deathRowIndex     = 1;   // second row
    static constexpr int   deathFrames       = 13;
    static constexpr float deathFrameDuration= 0.08f; // seconds per frame
};

#endif // WOLFENEMY_HPP
