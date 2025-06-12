// WolfEnemy.hpp
#ifndef WOLFENEMY_HPP
#define WOLFENEMY_HPP

#include "Enemy.hpp"

/// <summary>
/// WolfEnemy dashes in short bursts and permanently enrages at low HP.
/// Sprite sheet: resources/images/play/wolf.png (192×64px, 6×2 frames).
/// </summary>
class WolfEnemy : public Enemy {
public:
    /// <param name="x,y">Spawn position in pixels (world coords)</param>
    WolfEnemy(int x, int y);

    /// Handles dash/enrage and then movement/animation.
    void Update(float deltaTime) override;

    /// Applies damage; if below 50% HP → enrage.
    void Hit(float damage) override;

private:
    // Dash logic
    float dashCooldown   = 5.0f;  // seconds between dashes
    float dashTimer      = 0.0f;
    float dashDuration   = 0.5f;  // dash lasts 0.5s
    float dashRemaining  = 0.0f;

    // Enrage logic
    float originalSpeed;          // stored at spawn
    float originalHP;             // stored at spawn
    bool  enraged         = false;
};

#endif // WOLFENEMY_HPP
