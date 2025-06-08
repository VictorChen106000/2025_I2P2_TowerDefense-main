#include <allegro5/base.h>
#include <cmath>
#include "RocketTurret.hpp"
#include "Bullet/RocketBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Scene/PlayScene.hpp"
#include <vector>

const int RocketTurret::Price = 80;

RocketTurret::RocketTurret(float x, float y)
    : Turret(
        "play/tower-base.png",         // base image
        "play/blank.png",           // head image
        x, y,
        800,                            // wider range (vs. 200)
        Price,
        1.3f                           
      )
{
    // same downward shift so head aligns
    Anchor.y -= 1.0f / GetBitmapHeight();
    maxlevel =3;
    updrageprice ={ 75,125};
    setlevelimages();
    setBulletImages();
    applylevelstats();
}
void RocketTurret::setBulletImages() {
    bulletImages = {
        "play/bullet-4.png", // L1
        "play/bullet-4.png",// L2
        "play/bullet-3.png" // L3
    };
}

void RocketTurret::setlevelimages() {
    levelImages.push_back("play/roketcrop1.png");  // Level 1 image
    levelImages.push_back("play/roketcrop2.png");  // Level 2 image
    levelImages.push_back("play/roketcrop3.png");  // Level 3 image
}

void RocketTurret::applylevelstats() {
    switch (level) {
        case 1:
            coolDown = 1.5f;  // Initial cooldown
            CollisionRadius = 600.0f;  // Initial range
            break;
        case 2:
            coolDown = 0.5f;  // Faster reload for level 2
            CollisionRadius = 600.0f * 1.25f;  // Increase range for level 2
            break;
        case 3:
            coolDown = 0.1f;  // Even faster reload for level 3
            CollisionRadius = 600.0f * 1.5f;  // Further increase range for level 3
            break;
        default:
            break;
    }
    ChangeImage();
}

void RocketTurret::CreateBullet() {
    float baseAngle = Rotation - ALLEGRO_PI/2;
    int lvl = getlevel();

    // 1) Build your offset list
    std::vector<float> angleOffsets;
    if (lvl == 1) {
        angleOffsets = { 0.0f };
    }
    else if (lvl == 2) {
        float spread2 = ALLEGRO_PI/3; // 30°
        angleOffsets = { -spread2/2, +spread2/2 };
    }
    else { // lvl == 3
        float outerSpread = ALLEGRO_PI/3.5; 
        float innerSpread = ALLEGRO_PI/6; 
        angleOffsets = {
            -outerSpread,
            -innerSpread,
             innerSpread,
             outerSpread
        };
    }

    // 2) Spawn one rocket per offset
    for (float offset : angleOffsets) {
        float angle = baseAngle + offset;
        Engine::Point dir(std::cos(angle), std::sin(angle));
        Engine::Point norm = dir.Normalize();

        // still index the sprite by actual level
        const std::string &img = bulletImages[lvl - 1];

        auto b = new RocketBullet(
            img,
            Position + norm * 36,
            dir,
            angle,
            this
        );
        b->Target = this->Target;
        if (b->Target)
            b->Target->lockedBullets.push_back(b);
        getPlayScene()->BulletGroup->AddNewObject(b);
    }

    AudioHelper::PlayAudio("rocket.wav");
}


/*void RocketTurret::CreateBullet() {
    // same aim logic as MachineGunTurret
    Engine::Point diff = Engine::Point(
        cos(Rotation - ALLEGRO_PI/2),
        sin(Rotation - ALLEGRO_PI/2)
    );
    float rotation = atan2(diff.y, diff.x);
    Engine::Point norm = diff.Normalize();

    // pick back-most target
    Enemy* target = pickBackMost(scene);
    if (!target) return;

    Engine::Point dir = (target->Position - Position).Normalize();
    float rotation    = atan2(dir.y, dir.x);
    Engine::Point perp(-dir.y, dir.x);

    float spawnDist = 36, offset = 6;
    // left rocket
    scene->BulletGroup->AddNewObject(new RocketBullet(
        Position + dir*spawnDist - perp*offset,
        dir,
        rotation,
        this,
        target,               // pass it here
        "play/bullet-3.png"
    ));
    // right rocket
    scene->BulletGroup->AddNewObject(new RocketBullet(
        Position + dir*spawnDist + perp*offset,
        dir,
        rotation,
        this,
        target,
        "play/bullet-3.png"
    ));

    AudioHelper::PlayAudio("laser.wav");
}
    */
