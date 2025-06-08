#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "Bullet/FireBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "MachineGunTurret.hpp"
#include "Scene/PlayScene.hpp"

const int MachineGunTurret::Price = 50;
MachineGunTurret::MachineGunTurret(float x, float y)
    : Turret("play/tower-base.png", "play/blank.png", x, y, 200, Price, 0.5) {
    // Move center downward, since we the turret head is slightly biased upward.

    maxlevel =3;
    updrageprice ={ 75,125};

    Anchor.y += 8.0f / GetBitmapHeight();
    setlevelimages();
    applylevelstats();
}
void MachineGunTurret::setlevelimages() {
    levelImages.push_back("play/roketred1.png");  // Level 1 image
    levelImages.push_back("play/roketred2.png");  // Level 2 image
    levelImages.push_back("play/roketred3.png");  // Level 3 image
}
void MachineGunTurret::applylevelstats() {
    switch (level) {
        case 1:
            // Level 1 stats (already partly set by base constructor)
            coolDown = 0.75f;         // 0.5 seconds per shot
            CollisionRadius = 200.0f; // range
            // If you have a “damage” member inside FireBullet, you could pass 10 to it.
            // But in your original code, FireBullet took (position, direction, rotation, this).
            // So you might store a damage field in the turret and pass it in CreateBullet().
            break;

        case 2:
            // BUMP: fire 10% faster and range + 20%
            coolDown = 0.35f;         // slightly faster
            CollisionRadius = 200.0f * 1.20f; // 20% more range
            break;

        case 3:
            // BUMP: fire 10% faster AGAIN and range + another 20%
            coolDown = 0.10f;
            CollisionRadius = 200.0f * 1.40f;
            break;

        default:
            break;
    }
    ChangeImage();
}
void MachineGunTurret::CreateBullet() {
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    float rotation = atan2(diff.y, diff.x);
    Engine::Point normalized = diff.Normalize();
    // Change bullet position to the front of the gun barrel.
    getPlayScene()->BulletGroup->AddNewObject(new FireBullet(Position + normalized * 36, diff, rotation, this));
    AudioHelper::PlayAudio("gun.wav");
}


