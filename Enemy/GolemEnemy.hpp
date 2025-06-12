// GolemEnemy.hpp
#ifndef GOLEMENEMY_HPP
#define GOLEMENEMY_HPP

#include "Enemy.hpp"

/// <summary>
/// GolemEnemy: heavy slow mover. Shields every few seconds and enrages at low HP.
/// Sprite sheet: resources/images/play/golemattack.png (256×192px, 4×3 grid, 11 frames).
/// </summary>
class GolemEnemy : public Enemy {
public:
    /// <param name="x,y">Spawn position in world pixels</param>
    GolemEnemy(int x, int y);

    /// Shield + movement/animation + orientation
    void Update(float deltaTime) override;

    /// Apply shielding, then enrage if threshold hit
    void Hit(float damage) override;

private:
    // ─── Shielding ───────────────────────────────────────
    float shieldCooldown   = 6.0f;   // s between shields
    float shieldTimer      = 0.0f;
    float shieldDuration   = 1.5f;   // s of shield active
    float shieldRemaining  = 0.0f;
    bool  shieldActive     = false;

    // ─── Enrage ───────────────────────────────────────────
    float originalSpeed;            // stored at spawn
    float originalHP;               // stored at spawn
    bool  enraged         = false; 

    // ─── Scaling ─────────────────────────────────────────
    
};

#endif // GOLEMENEMY_HPP
