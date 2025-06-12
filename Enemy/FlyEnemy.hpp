// FlyEnemy.hpp
#ifndef FLYENEMY_HPP
#define FLYENEMY_HPP

#include "Enemy.hpp"

/// <summary>
/// FlyEnemy: every few seconds it teleports forward along the path
/// and gains a short speed burst. Uses an 8-frame, 512×64 sprite.
/// </summary>
class FlyEnemy : public Enemy {
public:
    FlyEnemy(int x, int y);
    void Update(float deltaTime) override;
    void Hit(float damage) override;

private:
    // teleport ability
    float teleportCooldown   = 7.0f;    // seconds between teleports
    float teleportTimer      = 0.0f;
    int   teleportDistance   = 2;       // how many path‐steps to skip

    // speed burst after teleport
    float buffDuration       = 2.0f;    // seconds
    float buffTimer          = 0.0f;
    float buffMultiplier     = 1.5f;

    // store original speed
    float originalSpeed;
};

#endif // FLYENEMY_HPP
