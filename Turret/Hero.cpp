#include "Hero.hpp"
#include "Engine/Resources.hpp"
#include "Engine/GameEngine.hpp"
#include "Scene/PlayScene.hpp"
#include "Enemy/Enemy.hpp"
#include <limits>
#include <cmath>
#include <algorithm>

Hero::Hero(float x, float y, float scale)
  : Engine::Sprite("play/crystal.png", x, y)
  , _scale(scale)
{
    Anchor = Engine::Point(0.5f, 0.5f);

    // load each sprite-sheet:  path, columns, fps
    loadAnim(_walkAnim,   "play/yellowninjawalk.png",   10, 10.0f);
    loadAnim(_attackAnim, "play/yellowninjaattack.png", 20, 15.0f);
    loadAnim(_dieAnim,    "play/yellowninjadie.png",    14,  8.0f);

    _state      = Walking;
    _frameIndex = 0;
    _frameTime  = 0;
    Velocity    = Engine::Point(0,0);
}

void Hero::loadAnim(Anim &anim,
                    const std::string &path,
                    int cols,
                    float fps)
{
    // grab the shared bitmap
    anim.sheet = Engine::Resources::GetInstance().GetBitmap(path);
    ALLEGRO_BITMAP* raw = anim.sheet.get();

    int W = al_get_bitmap_width(raw);
    int H = al_get_bitmap_height(raw);
    int fw = W / cols;
    int fh = H;

    anim.fps = fps;
    anim.frames.reserve(cols);
    for (int i = 0; i < cols; i++) {
        // slice out frame i
        anim.frames.push_back(
          al_create_sub_bitmap(raw, i*fw, 0, fw, fh)
        );
    }
}

void Hero::Update(float deltaTime) {
    _elapsed += deltaTime;
    auto scene = dynamic_cast<PlayScene*>(
        Engine::GameEngine::GetInstance().GetActiveScene()
    );

    // 1) If lifetime expired, switch to dying
    if (_elapsed >= _lifetime && _state != Dying) {
        _state      = Dying;
        _frameIndex = 0;
        _frameTime  = 0;
        Velocity    = Engine::Point(0,0);
    }

    // 2) Handle dying animation
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
    if (nearest) {
        auto dir = (nearest->Position - Position).Normalize();
        Velocity = dir * _speed;
        Rotation = std::atan2(dir.y, dir.x) + ALLEGRO_PI/2;
    } else {
        Velocity = Engine::Point(0,0);
    }

    // move
    Engine::Sprite::Update(deltaTime);

    // 4) Damage & decide walk vs attack
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
    State nextState = inRange ? Attacking : Walking;
    if (nextState != _state) {
        _state      = nextState;
        _frameIndex = 0;
        _frameTime  = 0;
    }

    // 5) Advance current animation
    Anim &A = (_state == Walking ? _walkAnim : _attackAnim);
    _frameTime += deltaTime;
    float frameDur = 1.0f / A.fps;
    if (_frameTime >= frameDur) {
        _frameTime -= frameDur;
        _frameIndex = (_frameIndex + 1) % int(A.frames.size());
    }
}

void Hero::Draw() const {
    // select anim sheet
    const Anim* A = nullptr;
    switch(_state) {
      case Walking:   A = &_walkAnim;   break;
      case Attacking: A = &_attackAnim; break;
      case Dying:     A = &_dieAnim;    break;
    }
    int idx = std::clamp(_frameIndex, 0, int(A->frames.size()-1));
    ALLEGRO_BITMAP* bmp = A->frames[idx];
    int fw = al_get_bitmap_width(bmp);
    int fh = al_get_bitmap_height(bmp);

    // draw centered, scaled, rotated
    al_draw_tinted_scaled_rotated_bitmap(
      bmp,
      Tint,
      fw/2, fh/2,
      Position.x, Position.y,
      _scale, _scale,
      Rotation,
      0
    );
}
