#ifndef BOMBTURRET_HPP
#define BOMBTURRET_HPP

#include "Turret.hpp"

// A stationary bomb that detonates when any enemy enters its triggerRadius.
class BombTurret : public Turret {
public:
    static const int Price;

    BombTurret(float x, float y);

    // We never shoot bullets, so CreateBullet is a no-op
    void CreateBullet() override {}
   

    // Each frame, check if an enemy has stepped on us
    void Update(float deltaTime) override;

private:
    // Only explode once
    bool hasExploded = false;

    // How close an enemy must be to trigger us
    float triggerRadius;

    // How large the damage area is
    float explosionRadius;

    // Perform the explosion: effect, damage, then mark for removal
    void Explode();

    void applylevelstats()override;
    void setlevelimages() override;
};

#endif // BOMBTURRET_HPP
