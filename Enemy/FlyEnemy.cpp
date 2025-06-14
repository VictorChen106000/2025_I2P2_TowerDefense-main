#include "FlyEnemy.hpp"
#include "Scene/PlayScene.hpp"         // for BlockSize
#include "UI/Animation/DirtyEffect.hpp"
#include "Engine/AudioHelper.hpp"      // << added for PlayAudio
#include <cmath>
#include <allegro5/allegro.h>          // for ALLEGRO_PI

FlyEnemy::FlyEnemy(int x, int y)
    : Enemy("play/flyy.png",  // 512×64 under resources/images/play/
            x, y,
            /*radius=*/15,
            /*speed=*/120,
            /*hp=*/20,
            /*money=*/12),
      originalSpeed(speed)
{
    // 1) Slice 8×1 @10 fps
    SetAnimation(8, 1, 10.0f);

    // 2) Scale each 64×64 cell to 60% → ~38×38 on-screen
    float frameW = GetBitmapWidth()  / 8.0f;  // =512/8 =64
    float frameH = GetBitmapHeight() / 1.0f;  // = 64/1 =64
    Size.x = frameW * 1.0f;
    Size.y = frameH * 1.0f;
}

void FlyEnemy::Update(float deltaTime) {
    // ─── Teleport logic ────────────────────────────────────
    teleportTimer += deltaTime;
    if (teleportTimer >= teleportCooldown && path.size() > teleportDistance + 1) {
        teleportTimer -= teleportCooldown;
        // skip forward along the path
        for (int i = 0; i < teleportDistance; i++)
            path.pop_back();
        // snap to that position
        auto target = path.back() * PlayScene::BlockSize
                      + Engine::Point(PlayScene::BlockSize/2, PlayScene::BlockSize/2);
         AudioHelper::PlayAudio("laser.wav");
        Position = target;
        // spawn a little dust effect
        getPlayScene()->GroundEffectGroup->AddNewObject(
            new DirtyEffect("play/dirty-1.png", 10, Position.x, Position.y)
        );
        // grant a temporary speed buff
        speed      = originalSpeed * buffMultiplier;
        buffTimer  = buffDuration;
    }

    // ─── Buff timing ───────────────────────────────────────
    if (buffTimer > 0.0f) {
        buffTimer -= deltaTime;
        if (buffTimer <= 0.0f) {
            speed = originalSpeed;
        }
    }

    // ─── Move + animate once ───────────────────────────────
    Enemy::Update(deltaTime);

    // ─── Orientation (vertical vs horizontal) ──────────────
    float vx = Velocity.x, vy = Velocity.y;
    if (std::fabs(vy) > std::fabs(vx)) {
        // mostly vertical → rotate ±90°
        Rotation = (vy > 0 ? ALLEGRO_PI/2 : -ALLEGRO_PI/2);
        Size.x = std::fabs(Size.x);
    } else {
        // mostly horizontal → upright + mirror
        Rotation = 0.0f;
        float w = std::fabs(Size.x);
        Size.x = (vx < 0 ? -w : w);
    }
}

void FlyEnemy::Hit(float damage) {
    // cancel any teleport buff when hit
    buffTimer = 0.0f;
    speed     = originalSpeed;
    Enemy::Hit(damage);
}
