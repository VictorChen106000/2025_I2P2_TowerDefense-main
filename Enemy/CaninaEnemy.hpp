#ifndef CANINA_ENEMY_HPP
#define CANINA_ENEMY_HPP

#include "Enemy.hpp"
#include <vector>

class CaninaEnemy : public Enemy {
public:
    // ctor: spawn at (x,y)
    CaninaEnemy(int x, int y);

    // override to add healing aura
    void Update(float deltaTime) override;
    void Draw() const override;


    float healTimer      = 0.0f;        // accumulates deltaTime
    const float healInterval = 5.0f;     // seconds between heal pulses
    const float healAmount   = 10.0f;    // HP per pulse
    const float healRadius   = 100.0f;   // px radius of aura
    float originalHP;                    // max HP cap
};

#endif // CANINA_ENEMY_HPP
