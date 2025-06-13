#ifndef STAGESELECTSCENE_HPP
#define STAGESELECTSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <memory>
#include "UI/Animation/ParallaxBackground.hpp"
#include "Engine/IScene.hpp"

class StageSelectScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    ParallaxBackground parallax;

public:
    explicit StageSelectScene() = default;
    void Initialize() override;
    void Terminate() override;
    void PlayOnClick(int stage);
    void ScoreboardOnClick();
    void ShopOnClick();
    void BackOnClick(int stage);
    void Draw() const override;
};

#endif   // STAGESELECTSCENE_HPP
