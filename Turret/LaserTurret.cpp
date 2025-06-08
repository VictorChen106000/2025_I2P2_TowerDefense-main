#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "Bullet/LaserBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "LaserTurret.hpp"
#include "Scene/PlayScene.hpp"

const int LaserTurret::Price = 200;
LaserTurret::LaserTurret(float x, float y) : Turret("play/tower-base.png", "play/blank.png", x, y, 300, Price, 0.5) {
    // Move center downward, since we the turret head is slightly biased upward.
    Anchor.y += 8.0f / GetBitmapHeight();
    maxlevel =3;
    updrageprice ={ 75,125};

    
    setlevelimages();
    applylevelstats();
}
void LaserTurret::setlevelimages() {
    levelImages.push_back("play/tankblue1.png");  // Level 1 image
    levelImages.push_back("play/tankblue2.png");  // Level 2 image
    levelImages.push_back("play/tankblue3.png");  // Level 3 image
}
void LaserTurret::applylevelstats() {
    switch (level) {
        case 1:
            coolDown = 1.5f;  // Initial cooldown
            CollisionRadius = 160.0f;  // Initial range
            break;
        case 2:
            coolDown = 1.3f;  // Faster reload for level 2
            CollisionRadius = 160.0f * 1.25f;  // Increase range for level 2
            break;
        case 3:
            coolDown = 1.1f;  // Even faster reload for level 3
            CollisionRadius = 160.0f * 1.5f;  // Further increase range for level 3
            break;
        default:
            break;
    }
    ChangeImage();
}
void LaserTurret::CreateBullet() {
    // 1) Compute base firing angle & forward vector
    float baseAngle = Rotation - ALLEGRO_PI/2;
    Engine::Point forward(
        std::cos(baseAngle),
        std::sin(baseAngle)
    );

    // 2) Decide how wide the fan is at each level
    //    Level 1: no spread  (0°)
    //    Level 2: tight fan (e.g. 10° total)
    //    Level 3: medium fan (e.g. 20° total)
    float spreads[] = {
        0.0f,
        ALLEGRO_PI * (10.0f/180.0f),  // 10° in radians
        ALLEGRO_PI * (20.0f/180.0f)   // 20° in radians
    };

    int lvl = getlevel();         // 1, 2 or 3
    float totalSpread = spreads[lvl-1];
    int   count       = lvl;      // # of bullets to fire

    // 3) Spawn one bullet per evenly spaced offset
    for (int i = 0; i < count; i++) {
        // evenly from –totalSpread/2  …  +totalSpread/2
        float offset = (count > 1)
            ? -totalSpread * 0.5f + (totalSpread * i) / (count - 1)
            : 0.0f;

        float angle = baseAngle + offset;
        Engine::Point dir  (std::cos(angle), std::sin(angle));
        Engine::Point norm = dir.Normalize();

        Engine::Point spawnPos = Position + norm * 36;
        getPlayScene()->BulletGroup->AddNewObject(
            new LaserBullet(spawnPos, dir, angle, this)
        );
    }

    AudioHelper::PlayAudio("laser.wav");
}


