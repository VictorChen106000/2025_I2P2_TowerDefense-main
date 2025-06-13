#ifndef FLYENEMY_HPP
#define FLYENEMY_HPP

#include "Enemy.hpp"

/// <summary>
/// FlyEnemy: every few seconds it teleports forward along the path
/// and gains a short speed burst. On death it plays a 17-frame
/// animation from play/flydie.png before actually dying.
/// </summary>
class FlyEnemy : public Enemy {
public:
    FlyEnemy(int x, int y);
    void Update(float deltaTime) override;
    void Hit(float damage) override;
    void Draw() const override;

private:
    // --- teleport ability ---
    float teleportCooldown   = 7.0f;    // seconds between teleports
    float teleportTimer      = 0.0f;
    int   teleportDistance   = 2;       // how many path steps to skip

    // --- speed burst after teleport ---
    float buffDuration       = 2.0f;    // seconds
    float buffTimer          = 0.0f;
    float buffMultiplier     = 1.5f;

    // store original speed
    float originalSpeed;

    // --- death animation state ---
    bool   _isDying          = false;
    int    _deathFrame       = 0;          // 0…deathFrames-1
    float  _deathFrameTimer  = 0.0f;
    static constexpr int   deathFrames        = 17;   // columns in flydie.png
    static constexpr float deathFrameDuration = 0.07f; // seconds per frame
};

#endif // FLYENEMY_HPP
