#include "FlyEnemy.hpp"
#include "Scene/PlayScene.hpp"         // for BlockSize
#include "UI/Animation/DirtyEffect.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Resources.hpp"
#include <cmath>
#include <allegro5/allegro_primitives.h>

// global list of enemies
extern std::vector<Enemy*> g_enemies;

FlyEnemy::FlyEnemy(int x, int y)
    : Enemy("play/flyy.png", x, y, /*radius=*/15, /*speed=*/120, /*hp=*/20, /*money=*/12),
      originalSpeed(speed)
{
    // 1) Slice 8×1 @10 fps
    SetAnimation(8, 1, 10.0f);

    // 2) Scale each 64×64 cell to native size
    float frameW = GetBitmapWidth()  / 8.0f;  // =64
    float frameH = GetBitmapHeight() / 1.0f;  // =64
    Size.x = frameW;
    Size.y = frameH;
}

void FlyEnemy::Hit(float damage) {
    // if already dying, ignore further hits
    if (_isDying) return;

    bool dying = (hp - damage) <= 0.0f;
    if (dying) {
        // start death animation; do not call base Hit yet
        _isDying         = true;
        _deathFrame      = 0;
        _deathFrameTimer = 0.0f;
        return;
    }

    // cancel any teleport buff when hit
    buffTimer = 0.0f;
    speed     = originalSpeed;

    // normal damage
    Enemy::Hit(damage);
}

void FlyEnemy::Update(float deltaTime) {
    auto scene = getPlayScene();

    if (_isDying) {
        // advance death animation
        _deathFrameTimer += deltaTime;
        if (_deathFrameTimer >= deathFrameDuration) {
            _deathFrameTimer -= deathFrameDuration;
            _deathFrame++;
            if (_deathFrame >= deathFrames) {
                // now actually die: trigger explosion & removal
                Enemy::Hit(hp);  // hp<=0 so base Hit will remove us
            }
        }
        return;  // skip movement/teleport while dying
    }

    // ─── Teleport logic ────────────────────────────────────
    teleportTimer += deltaTime;
    if (teleportTimer >= teleportCooldown && path.size() > teleportDistance + 1) {
        teleportTimer -= teleportCooldown;
        // skip forward along the path
        for (int i = 0; i < teleportDistance; i++)
            path.pop_back();
        // snap to that position
        auto target = path.back() * PlayScene::BlockSize
                      + Engine::Point(PlayScene::BlockSize / 2, PlayScene::BlockSize / 2);
        AudioHelper::PlayAudio("laser.wav");
        Position = target;
        // spawn a little dust effect
        getPlayScene()->GroundEffectGroup->AddNewObject(
            new DirtyEffect("play/dirty-1.png", 10, Position.x, Position.y)
        );
        // grant a temporary speed buff
        speed     = originalSpeed * buffMultiplier;
        buffTimer = buffDuration;
    }

    // ─── Buff timing ───────────────────────────────────────
    if (buffTimer > 0.0f) {
        buffTimer -= deltaTime;
        if (buffTimer <= 0.0f) {
            speed = originalSpeed;
        }
    }

    // ─── Move + animate ────────────────────────────────────
    Enemy::Update(deltaTime);
    // face only left/right
    Rotation = 0.0f;
    float w = std::fabs(Size.x);
    Size.x = (Velocity.x < 0 ? -w : w);
}

void FlyEnemy::Draw() const {
    if (_isDying) {
        // draw one frame from flydie.png
        auto bmpPtr = Engine::Resources::GetInstance()
                          .GetBitmap("play/flydie.png");
        ALLEGRO_BITMAP* bmp = bmpPtr.get();
        int fw = al_get_bitmap_width(bmp)  / deathFrames;
        int fh = al_get_bitmap_height(bmp) / 1;
        int sx = _deathFrame * fw;
        int sy = 0;
        // center it on Position, scale 1:1
        al_draw_scaled_bitmap(
            bmp,
            sx, sy, fw, fh,
            Position.x - fw/2.0f,
            Position.y - fh/2.0f,
            fw, fh,
            0
        );
    } else {
        // normal draw
        Enemy::Draw();
    }
}
