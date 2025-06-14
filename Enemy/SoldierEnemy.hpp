#ifndef SOLDIERENEMY_HPP
#define SOLDIERENEMY_HPP
#include "Enemy.hpp"

class SoldierEnemy : public Enemy {
public:
    SoldierEnemy(int x, int y);
    void SetSpeedBoost(float bonus, float duration);
    void Update(float deltaTime) override;
    private:
    float _boostRemaining  = 0.0f;  // seconds left on the boost
    float _pendingBonus    = 0.0f;  // how much we've added to 'speed'
    Type GetType() const override { return Type::soldier; }
};
#endif   // SOLDIERENEMY_HPP
