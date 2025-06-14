#ifndef BATENEMY_HPP
#define BATENEMY_HPP

#include "Enemy.hpp"

/// BatEnemy: a simple moving enemy with a 4-frame flap animation that
/// plays a 4×3 “batdeath.png” death animation once, scaled 2×.
class BatEnemy : public Enemy {
public:
    explicit BatEnemy(int x, int y);

    void Hit(float damage) override;
    void Update(float deltaTime) override;
    void Draw()   const override;
    Type GetType() const override { return Type::bat; }

private:
    // ——— death-animation state ———
    bool  _isDying         = false;
    int   _deathFrame      = 0;
    float _deathFrameTimer = 0.0f;

    static constexpr int   deathCols          = 4;
    static constexpr int   deathRows          = 3;
    static constexpr int   deathFrames        = deathCols * deathRows;
    static constexpr float deathFrameDuration = 0.035f;  // seconds per frame

    // draw scale (2× native)
    static constexpr float drawScale = 1.5f;
};

#endif // BATENEMY_HPP
