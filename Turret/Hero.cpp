#include "Hero.hpp"
#include "Engine/Resources.hpp"
#include "Engine/GameEngine.hpp"
#include "Scene/PlayScene.hpp"
#include "Enemy/Enemy.hpp"
#include <limits>
#include <cmath>
#include <algorithm>
#include <allegro5/allegro_primitives.h>

Hero::Hero(float x, float y, float scale)
  : Engine::Sprite("play/yellowninja.png", x, y)
  , _scale(scale)
{
    Anchor = Engine::Point(0.5f, 0.5f);

    loadAnim(_walkAnim,   "play/yellowninjawalk.png",   10, 10.0f);
    loadAnim(_attackAnim, "play/yellowninjaattack.png", 20, 15.0f);
    loadAnim(_dieAnim,    "play/yellowninjadie.png",    14,  8.0f);

    _state      = Walking;
    _frameIndex = 0;
    _frameTime  = 0;
    Velocity    = Engine::Point(0.0f, 0.0f);
}

void Hero::loadAnim(Anim &anim,
                    const std::string &path,
                    int cols,
                    float fps)
{
    anim.sheet = Engine::Resources::GetInstance().GetBitmap(path);
    ALLEGRO_BITMAP* raw = anim.sheet.get();

    int W = al_get_bitmap_width(raw);
    int H = al_get_bitmap_height(raw);
    int fw = W / cols;
    int fh = H;

    anim.fps = fps;
    anim.frames.reserve(cols);
    for (int i = 0; i < cols; i++) {
        anim.frames.push_back(
          al_create_sub_bitmap(raw, i * fw, 0, fw, fh)
        );
    }
}

void Hero::Update(float deltaTime) {
    _elapsed += deltaTime;
    auto scene = dynamic_cast<PlayScene*>(
        Engine::GameEngine::GetInstance().GetActiveScene()
    );

    // 1) Lifetime expiry → start dying
    if (_elapsed >= _lifetime && _state != Dying) {
        _state      = Dying;
        _frameIndex = 0;
        _frameTime  = 0;
        Velocity    = Engine::Point(0.0f, 0.0f);
    }

    // 2) Dying animation
    if (_state == Dying) {
        _frameTime += deltaTime;
        float frameDur = 1.0f / _dieAnim.fps;
        if (_frameTime >= frameDur) {
            _frameTime -= frameDur;
            _frameIndex++;
            if (_frameIndex >= int(_dieAnim.frames.size())) {
                scene->HeroGroup->RemoveObject(objectIterator);
                return;
            }
        }
        return;
    }

    // 3) Seek nearest enemy
    Enemy* nearest = nullptr;
    float bestD2 = std::numeric_limits<float>::infinity();
    for (auto o : scene->EnemyGroup->GetObjects()) {
        auto e = dynamic_cast<Enemy*>(o);
        float dx = e->Position.x - Position.x;
        float dy = e->Position.y - Position.y;
        float d2 = dx*dx + dy*dy;
        if (d2 < bestD2) {
            bestD2 = d2;
            nearest = e;
        }
    }

    // 4) Chase only if beyond _stopDistance
    if (nearest) {
        Engine::Point delta = nearest->Position - Position;
        float dist = delta.Magnitude();
        if (dist > _stopDistance) {
            Engine::Point dir = delta.Normalize();
            Velocity = dir * _speed;
        } else {
            Velocity = Engine::Point(0.0f, 0.0f);
        }
    } else {
        Velocity = Engine::Point(0.0f, 0.0f);
    }

    // 5) Move
    Engine::Sprite::Update(deltaTime);

    // 6) Lock rotation to 0 & flip horizontally
    Rotation = 0.0f;
    {
        float w = std::fabs(Size.x);
        Size.x = (Velocity.x < 0.0f ? -w : w);
    }

    // 7) Damage & switch state
    bool inRange = false;
    for (auto o : scene->EnemyGroup->GetObjects()) {
        auto e = dynamic_cast<Enemy*>(o);
        float dx = e->Position.x - Position.x;
        float dy = e->Position.y - Position.y;
        if (dx*dx + dy*dy <= _range*_range) {
            e->Hit(_dps * deltaTime);
            inRange = true;
        }
    }
    State next = inRange ? Attacking : Walking;
    if (next != _state) {
        _state      = next;
        _frameIndex = 0;
        _frameTime  = 0;
    }

    // 8) Advance current animation
    Anim &A = (_state == Walking ? _walkAnim : _attackAnim);
    _frameTime += deltaTime;
    float fd = 1.0f / A.fps;
    if (_frameTime >= fd) {
        _frameTime -= fd;
        _frameIndex = (_frameIndex + 1) % int(A.frames.size());
    }
}

void Hero::Draw() const {
    const Anim* A = nullptr;
    switch (_state) {
      case Walking:   A = &_walkAnim;   break;
      case Attacking: A = &_attackAnim; break;
      case Dying:     A = &_dieAnim;    break;
    }
    int idx = std::clamp(_frameIndex, 0, int(A->frames.size()-1));
    ALLEGRO_BITMAP* bmp = A->frames[idx];
    int fw = al_get_bitmap_width(bmp);
    int fh = al_get_bitmap_height(bmp);

    float dw = fw * _scale;
    float dh = fh * _scale;
    float dx = Position.x - dw/2;
    float dy = Position.y - dh/2;

    // flip if moving left
    if (Velocity.x < 0.0f) {
        dx += dw;
        dw = -dw;
    }

    al_draw_tinted_scaled_bitmap(
        bmp,
        Tint,
        0, 0, fw, fh,
        dx, dy,
        dw, dh,
        0
    );
}
