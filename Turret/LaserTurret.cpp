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
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    float rotation = atan2(diff.y, diff.x);
    Engine::Point normalized = diff.Normalize();
    Engine::Point normal = Engine::Point(-normalized.y, normalized.x);
    // Change bullet position to the front of the gun barrel.
    getPlayScene()->BulletGroup->AddNewObject(new LaserBullet(Position + normalized * 36 - normal * 6, diff, rotation, this));
    getPlayScene()->BulletGroup->AddNewObject(new LaserBullet(Position + normalized * 36 + normal * 6, diff, rotation, this));
    AudioHelper::PlayAudio("laser.wav");
}
