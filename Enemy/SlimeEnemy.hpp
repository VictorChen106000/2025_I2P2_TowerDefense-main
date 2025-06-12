// SlimeEnemy.hpp
#ifndef SLIMEENEMY_HPP
#define SLIMEENEMY_HPP

#include "Enemy.hpp"

/// <summary>
/// SlimeEnemy slowly regenerates HP every few seconds,
/// and when first dropping below 50% HP it doubles its regen rate.
/// Sprite sheet: resources/images/play/slime.png (256×64px, 4×1 frames).
/// </summary>
class SlimeEnemy : public Enemy {
public:
    /// <param name="x,y">spawn coords in pixels</param>
    SlimeEnemy(int x, int y);

    /// Regeneration + movement/animation + orientation
    void Update(float deltaTime) override;

    /// Enrage on low HP → boost regen
    void Hit(float damage) override;

private:
    // regen logic
    float regenCooldown = 2.0f;   // seconds between heals
    float regenTimer    = 0.0f;
    float regenAmount   = 5.0f;   // HP per tick

    float originalHP;             // to clamp/max and check 50%
    bool  enraged      = false;   // only enrage once
};

#endif // SLIMEENEMY_HPP
