// BatEnemy.hpp
#ifndef BATENEMY_HPP
#define BATENEMY_HPP

#include "Enemy.hpp"

/// <summary>
/// BatEnemy: a simple moving enemy with a 4-frame flap animation.
/// </summary>
class BatEnemy : public Enemy {
public:
    /// <param name="x,y">Spawn position in world pixels.</param>
    explicit BatEnemy(int x, int y);
};

#endif // BATENEMY_HPP
