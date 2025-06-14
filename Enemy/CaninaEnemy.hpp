#ifndef CANINA_ENEMY_HPP
#define CANINA_ENEMY_HPP

#include "Enemy.hpp"
#include <vector>

class CaninaEnemy : public Enemy {
public:
    CaninaEnemy(int x, int y);

    void Hit(float damage) override;
    void Update(float deltaTime) override;
    void Draw()   const  override;

    // aura params
    float healTimer        = 0.0f;
    const float healInterval = 5.0f;
    const float healAmount   = 10.0f;
    const float healRadius   = 100.0f;
    float originalHP;

private:
    // death-animation state
    bool  _isDying         = false;
    int   _deathFrame      = 0;       // 0…7
    float _deathFrameTimer = 0.0f;

    static constexpr int   deathCols          = 4;
    static constexpr int   deathRows          = 2;
    static constexpr int   deathFrames        = deathCols * deathRows; // 8
    static constexpr float deathFrameDuration = 0.1f;                  // seconds/frame
    static constexpr float drawScale          = 1.7f;                  // same as wolf

    static constexpr int   shieldCols          = 4;
    static constexpr int   shieldRows          = 4;
    static constexpr float shieldFrameDuration = 0.1f;  // seconds per frame

    int   _shieldFrame = 0;    // 0 … shieldCols*shieldRows-1
    float _shieldTimer = 0.0f;
};

#endif // CANINA_ENEMY_HPP
