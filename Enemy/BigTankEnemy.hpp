#ifndef BIGTANKENEMY_HPP
#define BIGTANKENEMY_HPP

#include "Enemy.hpp"

class BigTankEnemy : public Enemy {
public:
    BigTankEnemy(int x, int y);
    void Hit(float damage) override;
};

#endif // BIGTANKENEMY_HPP
