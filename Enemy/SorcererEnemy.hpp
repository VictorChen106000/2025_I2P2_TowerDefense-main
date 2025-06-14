// SorcererEnemy.hpp
#ifndef SORCERERENEMY_HPP
#define SORCERERENEMY_HPP

#include "Enemy.hpp"
#include <vector>

class Turret;

class SorcererEnemy : public Enemy {
public:
    SorcererEnemy(int x, int y);

    void Update(float deltaTime) override;
    void Draw()   const  override;

private:
    // EMP pulse settings
    float empTimer       = 0.0f;
    static constexpr float empInterval = 8.0f;    // every 8s
    static constexpr float empDuration = 3.0f;    // disable for 3s
    static constexpr float empRadius   = 150.0f;  // px

    struct DisabledRecord {
        Turret*   turret;
        float     reenableTime;
    };
    std::vector<DisabledRecord> _disabled;
};

#endif // SORCERERENEMY_HPP
