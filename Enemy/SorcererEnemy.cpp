    // SorcererEnemy.cpp
    #include "SorcererEnemy.hpp"
    #include "Scene/PlayScene.hpp"
    #include "Turret/Turret.hpp"            // need the Turret definition
    #include "Engine/LOG.hpp"              // optional, for debug logging
    #include <allegro5/allegro_primitives.h>
    #include <cmath>

    extern std::vector<Enemy*> g_enemies; // your global list

    SorcererEnemy::SorcererEnemy(int x, int y)
    : Enemy("play/sorcerer.png", x, y,
            /*radius=*/20, /*speed=*/70, /*hp=*/100, /*money=*/30)
    {
        // 10 cols × 1 row @6fps
        SetAnimation(10, 1, 6.0f);
        SetFrameSequence({0,1,2,3,4,5,6,7,8,9});

        // scale to taste
        float fw = GetBitmapWidth()  / 10.0f;
        float fh = GetBitmapHeight() /  1.0f;
        Size.x = fw * 0.7f;
        Size.y = fh * 0.7f;
    }

    void SorcererEnemy::Update(float deltaTime) {
        auto scene = getPlayScene();
        float now = scene->GetElapsedTime();

        //
        // 1) Re-enable any turrets whose disable-time has passed
        //
        /*  
        for (auto it = _disabled.begin(); it != _disabled.end();) {
            if (now >= it->reenableTime) {
                it->turret->Enabled = true;
                it = _disabled.erase(it);
            }
            else ++it;
        }
        */
        //
        // 2) Count down to next EMP pulse
        //
        empTimer += deltaTime;
        if (empTimer >= empInterval) {
            empTimer -= empInterval;

            Engine::LOG(Engine::INFO)
                << "Sorcerer EMP @ t=" << now;  // debug

            //
            // 3) Pulse: disable all turrets in range
            //
            for (auto obj : scene->TowerGroup->GetObjects()) {
                Turret* t = dynamic_cast<Turret*>(obj);
                if (!t) continue;

                float dx = t->Position.x - Position.x;
                float dy = t->Position.y - Position.y;
                if (dx*dx + dy*dy <= empRadius * empRadius) {
                    if (t->Enabled) {
                        t->Enabled = false;
                        t->Target  = nullptr;  // drop any current target
                        _disabled.push_back({ t, now + empDuration });

                        Engine::LOG(Engine::INFO)
                            << "  → disabling turret at ("
                            << t->Position.x << "," << t->Position.y
                            << ") until " << (now + empDuration);
                    }
                }
            }
        }

        //
        // 4) Normal movement, animation, etc.
        //
        Enemy::Update(deltaTime);
        float vx = Velocity.x, vy = Velocity.y;
    if (std::fabs(vy) > std::fabs(vx)) {
        Rotation = (vy > 0 ? ALLEGRO_PI/2 : -ALLEGRO_PI/2);
        Size.x   = std::fabs(Size.x);
    } else {
        Rotation = 0.0f;
        float w  = std::fabs(Size.x);
        Size.x   = (vx < 0 ? -w : w);
    }
    }

    void SorcererEnemy::Draw() const {
        // visualize the EMP radius
        al_draw_filled_circle(
        Position.x, Position.y,
        empRadius,
        al_map_rgba(0, 0, 255, 64)
        );
        Enemy::Draw();
    }
