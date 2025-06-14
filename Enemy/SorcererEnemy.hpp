#ifndef SORCERERENEMY_HPP
#define SORCERERENEMY_HPP

#include "Enemy.hpp"
#include <vector>

class Turret;

class SorcererEnemy : public Enemy {
public:
    SorcererEnemy(int x, int y);

    void Hit(float damage)          override;
    void Update(float deltaTime)    override;
    void Draw()            const    override;
    Type GetType() const override { return Type::Boss8; }

private:
    // ─── Aura animation settings ─────────────────────────
    static constexpr int   auraCols     = 4;
    static constexpr int   auraRows     = 4;
    static constexpr float auraFrameDur = 0.1f;  // s per aura frame

    int   _auraFrame = 0;
    float _auraTimer = 0.0f;

    // ─── EMP radius ───────────────────────────────────────
    static constexpr float empRadius = 120.0f;
    std::vector<Turret*>   _disabled;

    // ─── Death‐animation using necromancer.png row 6 ──────
    bool   _isDying       = false;
    int    _deathFrame    = 0;                  // 0…8
    float  _deathTimer    = 0.0f;
    static constexpr int   deathFrames    = 9;   // frames 0–8
    static constexpr float deathFrameDur  = 0.1f;// s per death frame
};

#endif // SORCERERENEMY_HPP
