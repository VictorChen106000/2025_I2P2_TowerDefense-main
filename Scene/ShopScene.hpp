#ifndef SHOPSCENE_HPP
#define SHOPSCENE_HPP

#include <allegro5/allegro_audio.h>
#include <allegro5/allegro.h>
#include <memory>
#include "Engine/IScene.hpp"

namespace Engine {
    class ImageButton;
    class Image;
    class Label;
}

class ShopScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;

    // ── ninja‐idle animation (8×1 @ 8 FPS) ─────
    ALLEGRO_BITMAP* idleBmp       = nullptr;
    int             idleCols      = 8;
    int             idleFrameW    = 0;
    int             idleFrameH    = 0;
    int             idleCurFrame  = 0;
    float           idleTimer     = 0.0f;
    float           idleFrameTime = 1.0f / 8.0f;
    float           idleX         = 0.0f;
    float           idleY         = 0.0f;

    // UI pointers
    Engine::Label*       crystalCountLbl = nullptr;
    Engine::ImageButton* buyBtn          = nullptr;
    Engine::Image*       buyCostIcon     = nullptr;
    Engine::Label*       buyCostLabel    = nullptr;

public:
    void Initialize() override;
    void Terminate() override;
    void PlayOnClick(int stage);
    void ScoreboardOnClick();
    void BackOnClick(int stage);

    // drive our ninja sprite
    void Update(float deltaTime) override;
    void Draw()   const override;
};

#endif // SHOPSCENE_HPP
