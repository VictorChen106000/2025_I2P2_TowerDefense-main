#include <string>

#include "SoldierEnemy.hpp"

// TODO HACKATHON-3 (1/3): You can imitate the 2 files: 'SoldierEnemy.hpp', 'SoldierEnemy.cpp' to create a new enemy.
SoldierEnemy::SoldierEnemy(int x, int y) : Enemy("play/enemy-1.png", x, y, 10, 70, 5, 5) {}
void SoldierEnemy::SetSpeedBoost(float bonus, float duration) {
    if (bonus > 0 && duration > 0) {
        speed           += bonus;
        _pendingBonus    = bonus;
        _boostRemaining  = duration;
    }
}
void SoldierEnemy::Update(float deltaTime) {
    // 1) if boost is active, count it down
    if (_boostRemaining > 0.0f) {
        _boostRemaining -= deltaTime;
        if (_boostRemaining <= 0.0f) {
            // boost expired ⇒ remove it
            speed         -= _pendingBonus;
            _pendingBonus  = 0.0f;
        }
    }
    // 2) then do the usual path-walking
    Enemy::Update(deltaTime);
}
