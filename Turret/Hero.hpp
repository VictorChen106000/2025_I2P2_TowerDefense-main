// Hero.hpp
#ifndef HERO_HPP
#define HERO_HPP

#include "Engine/Sprite.hpp"
#include <vector>
#include <string>
#include <memory>
#include <allegro5/allegro.h>
#include "Scene/PlayScene.hpp"

class Hero : public Engine::Sprite {
public:
    /// @param x,y     center position
    /// @param scale   uniform draw scale (1.0 = original size)
    Hero(float x, float y, float scale = 1.6f);
    void Update(float deltaTime) override;
    void Draw()   const override;

private:
    enum State { Walking, Attacking, Dying };
    State _state = Walking;

    // behavior
    float _lifetime     = 15.0f;   // seconds before forced death
    float _elapsed      =  0.0f;
    float _attackRange  =64.0f;   // start attacking at this distance (px)
    float _stopDistance = 10.0f;  // hero will stop this far from enemy
    float _dps          = 30.0f;   // damage per second
    float _speed        =120.0f;   // chase speed (px/sec)
    float _scale        =   1.0f;  // uniform draw scale

    // track which way to face
    bool _faceLeft = false;

    struct Anim {
        std::shared_ptr<ALLEGRO_BITMAP> sheet;    // keep ownership alive
        std::vector<ALLEGRO_BITMAP*>    frames;   // raw sub-bitmaps
        float                            fps = 8.0f;
    };

    Anim _walkAnim, _attackAnim, _dieAnim;

    // animation state
    float _frameTime  = 0.0f;  // accumulated dt for frame advance
    int   _frameIndex = 0;     // current frame

    // helper to slice up a sheet into sub-bitmaps
    void loadAnim(Anim &anim,
                  const std::string &path,
                  int cols,
                  float fps);
};

#endif // HERO_HPP
