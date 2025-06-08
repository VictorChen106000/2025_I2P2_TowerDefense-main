#include <allegro5/base.h>
#include <cmath>
#include "Enemy/Enemy.hpp"
#include "RocketTurret.hpp"
#include "Bullet/RocketBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Point.hpp"
#include "Scene/PlayScene.hpp"
#include <vector>

const int RocketTurret::Price = 250;   

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

    // spawn two homing rockets with a little angular spread
    {
        // how far apart (in radians) the two rockets will fire
        const float spread = ALLEGRO_PI / 12;  // 15°
        // angles for each rocket: center ± half-spread
        float angles[2] = {
            rotation - spread * 0.5f,
            rotation + spread * 0.5f
        };
        for (float a : angles) {
            Engine::Point d = Engine::Point(std::cos(a), std::sin(a));
            Engine::Point n = d.Normalize();
            auto bullet = new RocketBullet(
                Position + n * 36,
                d,
                a,
                this
            );
            // keep homing target
            bullet->Target = this->Target;
            if (bullet->Target)
                bullet->Target->lockedBullets.push_back(bullet);
            getPlayScene()->BulletGroup->AddNewObject(bullet);
        }
    }

    // 1) Gather up to two distinct enemies in range
    std::vector<Enemy*> choices;
    for (auto &it : getPlayScene()->EnemyGroup->GetObjects()) {
        Enemy *e = dynamic_cast<Enemy*>(it);
        if (!e->Visible) 
            continue;
        // check distance² within turret’s range (800)
        float dx = e->Position.x - Position.x;
        float dy = e->Position.y - Position.y;
        if (dx*dx + dy*dy > 800.0f * 800.0f) 
            continue;
        choices.push_back(e);
        if (choices.size() == 2) 
            break;
    }
    // if only one enemy, duplicate it so both rockets spawn
    if (choices.empty()) 
        return;
    if (choices.size() == 1) 
        choices.push_back(choices[0]);

    // 2) Spawn one rocket per chosen enemy
    // for (Enemy *t : choices) {
    //     // direction vector toward this enemy
    //     Engine::Point diff = (t->Position - Position);
    //     Engine::Point norm = diff.Normalize();
    //     float rotation = atan2(diff.y, diff.x);
    //     auto bullet = new RocketBullet(
    //         Position + norm * 36,  // muzzle offset
    //         diff,                  // initial velocity dir
    //         rotation,              // base angle (ctor will +π/2)
    //         this
    //     );
    //     // assign its own homing target
    //     bullet->Target = t;
    //     t->lockedBullets.push_back(bullet);
    //     getPlayScene()->BulletGroup->AddNewObject(bullet);
    // }

    // Spawn two rockets which target enemy individually
    // {
    //     // how far left/right from center (tweak to taste)
    //     const float sideOffset = 12.0f;

    //     for (size_t i = 0; i < choices.size(); ++i) {
    //         Enemy *t = choices[i];

    //         // direction & rotation toward this target
    //         Engine::Point diff = t->Position - Position;
    //         Engine::Point norm = diff.Normalize();
    //         float rotation = atan2(diff.y, diff.x);

    //         // perpendicular for sideways spacing
    //         Engine::Point perp(-norm.y, norm.x);
    //         float sign = (i == 0 ? 1.0f : -1.0f);

    //         // spawn position: forward 36px, then left/right
    //         Engine::Point spawnPos =
    //             Position
    //           + norm * 36.0f
    //           + perp * sign * sideOffset;

    //         auto bullet = new RocketBullet(
    //             spawnPos,
    //             diff,
    //             rotation,
    //             this
    //         );
    //         // assign its own homing target
    //         bullet->Target = t;
    //         t->lockedBullets.push_back(bullet);
    //         getPlayScene()->BulletGroup->AddNewObject(bullet);
    //     }
    // }
    AudioHelper::PlayAudio("rocket.wav"); // pick a rocket SFX
}
    */
