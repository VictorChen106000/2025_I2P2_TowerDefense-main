// NecromancerEnemy.hpp
#ifndef NECROMANCER_ENEMY_HPP
#define NECROMANCER_ENEMY_HPP

#include "Enemy.hpp"

class NecromancerEnemy : public Enemy {
public:
    NecromancerEnemy(int x, int y);

    void Update(float deltaTime) override;
    void Draw() const override;

private:
    const float auraRadius = 120.0f;
    const float speedBuff  = 1.99f;   // 1.5× overall movement
};

#endif // NECROMANCER_ENEMY_HPP
