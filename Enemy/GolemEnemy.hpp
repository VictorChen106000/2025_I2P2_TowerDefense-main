#ifndef GOLEMENEMY_HPP
#define GOLEMENEMY_HPP

#include "Enemy.hpp"

/// <summary>
/// GolemEnemy: heavy slow mover. Shields every few seconds and enrages at low HP.
/// Plays a 10-frame death animation from play/golemdie.png before finally exploding.
/// </summary>
class GolemEnemy : public Enemy {
public:
    GolemEnemy(int x, int y);

    void Update(float deltaTime) override;
    void Hit(float damage) override;
    void Draw()   const    override;

private:
    // ─── Shielding ───────────────────────────────────────
    float shieldCooldown   = 6.0f;
    float shieldTimer      = 0.0f;
    float shieldDuration   = 1.5f;
    float shieldRemaining  = 0.0f;
    bool  shieldActive     = false;

    // ─── Enrage ───────────────────────────────────────────
    float originalSpeed;
    float originalHP;
    bool  enraged         = false;

    // ─── Death animation ──────────────────────────────────
    bool   _isDying           = false;
    int    _deathFrame        = 0;
    float  _deathFrameTimer   = 0.0f;
    static constexpr int   deathFrames        = 10;   // only use frames 0..9
    static constexpr float deathFrameDuration = 0.065f; // seconds per frame
};

#endif // GOLEMENEMY_HPP
