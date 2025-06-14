#ifndef WINSCENE_HPP
#define WINSCENE_HPP
#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>
#include <string>

namespace Engine {class Label; class Image;}

class WinScene final : public Engine::IScene {
private:
    float ticks;
    ALLEGRO_SAMPLE_ID bgmId;

    std::string playerName;
    Engine::Label* nameLabel = nullptr;
    float boxCenterX = 0.0f;
    float boxCenterY = 0.0f;
    void SaveScore();

public:
    explicit WinScene() = default;
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void BackOnClick(int stage);

protected:
    
    void Draw() const override;
};

#endif   // WINSCENE_HPP
