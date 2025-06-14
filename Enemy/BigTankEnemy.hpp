#ifndef BIGTANKENEMY_HPP
#define BIGTANKENEMY_HPP

#include "Enemy.hpp"

class BigTankEnemy : public Enemy {
public:
    BigTankEnemy(int x, int y);
    void Hit(float damage) override;
    Type GetType() const override { return Type::bigtank; }
};

#endif // BIGTANKENEMY_HPP
