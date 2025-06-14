#include <functional>
#include <string>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "LoseScene.hpp"
#include "PlayScene.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"

void LoseScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    AddNewObject(new Engine::Image("lose/lose.png", halfW, halfH, 0, 0, 0.5, 0.5));
    AddNewObject(new Engine::Label("You Lose :(", "balatro.ttf", 64, halfW, halfH / 4 + 10, 255, 255, 255, 255, 0.5, 0.5));
    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("stage-select/button1.png", "win/floor.png", halfW - 200, halfH * 7 / 4 - 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&LoseScene::BackOnClick, this, 2));
    btn->EnableBreathing(0.05f, 2.0f);
    btn->EnableHoverScale(0.9f);
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "balatro.ttf", 48, halfW, halfH * 7 / 4, 255, 255, 255, 255, 0.5, 0.5));
    // bgmInstance = AudioHelper::PlaySample("lose-sound2.mp3", false, AudioHelper::BGMVolume, PlayScene::DangerTime);
    // play the losing sound from the very start, no countdown cue
    bgmInstance = AudioHelper::PlaySample(
        "lose-sound2.mp3",
        false,
        AudioHelper::BGMVolume,
        0.0f
    );
}
void LoseScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void LoseScene::BackOnClick(int stage) {
    // Change to select scene.
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}
