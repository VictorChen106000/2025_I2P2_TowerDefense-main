#ifndef SettingsScene_HPP
#define SettingsScene_HPP
#include <memory>

#include "Engine/IScene.hpp"
#include "UI/Animation/ParallaxBackground.hpp"
#include <allegro5/allegro_audio.h>

class SettingsScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    ParallaxBackground parallax;

public:
    explicit SettingsScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
    void BGMSlideOnValueChanged(float value);
    void SFXSlideOnValueChanged(float value);
    void Draw() const override;
};

#endif   // SettingsScene_HPP
