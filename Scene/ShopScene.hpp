// ShopScene.hpp
#ifndef SHOPSCENE_HPP
#define SHOPSCENE_HPP

#include <allegro5/allegro_audio.h>
#include <memory>
#include "Engine/IScene.hpp"

namespace Engine {
    class ImageButton;
    class Image;
    class Label;
}

class ShopScene final : public Engine::IScene {
public:
    void Initialize() override;
    void Update(float dt) override;
    void Draw()   const override;
    void Terminate() override;

    void BackOnClick(int);
    void PlayOnClick(int stage);
    void ScoreboardOnClick();

private:
    // buttons & cost
    int                           buyCost      = 1;
    Engine::ImageButton*          buyBtn       = nullptr;
    Engine::ImageButton*          backBtn      = nullptr;

    // audio
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;

    // UI pointers
    Engine::Label*                crystalCountLbl = nullptr;
    Engine::Image*                buyCostIcon     = nullptr;
    Engine::Label*                buyCostLabel    = nullptr;

    // ninja animation
    ALLEGRO_BITMAP*               idleBmp      = nullptr;
    int                           idleCols     = 0;
    int                           idleFrameW   = 0;
    int                           idleFrameH   = 0;
    float                         idleFrameTime= 0.2f;
    float                         idleTimer    = 0.0f;
    int                           idleCurFrame = 0;
    float                         idleX        = 0.0f;
    float                         idleY        = 0.0f;
};

#endif // SHOPSCENE_HPP
