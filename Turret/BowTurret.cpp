#include <allegro5/base.h>
#include <cmath>
#include "Enemy/Enemy.hpp"
#include "BowTurret.hpp"
#include "Bullet/RocketBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Point.hpp"
#include "Scene/PlayScene.hpp"
#include <vector>

const int BowTurret::Price = 250;   

BowTurret::BowTurret(float x, float y)
    : Turret(
        "play/tower-base.png",         // base image
        "play/Ballista2.png",           // head image
        x, y,
        800,                            // wider range (vs. 200)
        Price,
        1.3f                           
      )
{
    // same downward shift so head aligns
    Anchor.y -= 1.0f / GetBitmapHeight();
    maxlevel =1;
    updrageprice ={ 75,125};
    setlevelimages();
    setBulletImages();
    applylevelstats();
}
void BowTurret::setBulletImages() {
   bulletImages = {
        "play/arrow13232.png", // L1
        "play/peluruhitam.png",// L2
        "play/peluruhitam.png" // L3
    };
}

void BowTurret::setlevelimages() {
    levelImages.push_back("play/blank.png");  // Level 1 image
    levelImages.push_back("play/Ballista2.png");  // Level 2 image
    levelImages.push_back("play/Ballista2.png");  // Level 3 image
}

void BowTurret::applylevelstats() {
   switch (level) {
        case 1:
            coolDown = 0.1f;  // Initial cooldown
            CollisionRadius = 600.0f;  // Initial range
            break;
        case 2:
            coolDown = 0.1f;  // Faster reload for level 2
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

void BowTurret::CreateBullet() {
    float baseAngle = Rotation - ALLEGRO_PI/2;
    int lvl = getlevel();

    // 1) Build your offset list
    std::vector<float> angleOffsets;
    if (lvl == 1) {
        angleOffsets = { 0.0f };
    }
    else if (lvl == 2) {
        float spread2 = ALLEGRO_PI/6; // 
        angleOffsets = { -spread2/2, +spread2/2 };
    }
    else { // lvl == 3

        float spread2 = ALLEGRO_PI/6; // 
        angleOffsets = { -spread2/2, +spread2/2 };

       // float outerSpread = ALLEGRO_PI/3.5; 
       // float innerSpread = ALLEGRO_PI/6; 
      //  angleOffsets = {
        //    -outerSpread,
       //     -innerSpread,
        //     innerSpread,
        //     outerSpread
        //};
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
void BowTurret::Update(float dt) {
    // 1) Tick down the cooldown
    coolDown -= dt;

    // 2) If it’s time, fire (this uses your CreateBullet())
    if (coolDown <= 0) {
        CreateBullet();
        applylevelstats();    // resets coolDown for the next shot
    }

    // 3) Still run the Sprite logic (if you need animations, etc.)
    Sprite::Update(dt);
}

