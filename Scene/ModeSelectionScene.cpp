#include "ModeSelectionScene.hpp"
#include <functional>
#include "Scene/PlayScene.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"

using namespace Engine;

void ModeSelectionScene::Initialize() {
    parallax.Load({
      "Resource/images/background/wl5.png",
      "Resource/images/background/wl4.png",
      "Resource/images/background/wl3.png",
      "Resource/images/background/wl2.png",
      "Resource/images/background/wl1.png"
    });
    // center layout
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // Title
    AddNewObject(new Engine::Label(
        "Select Mode",
        "balatro.ttf",
        120,
        halfW, halfH / 3 + 50,
        10, 255, 255, 255,
        0.5, 0.5
    ));

    // Normal Mode button
    auto *btnNormal = new Engine::ImageButton(
        "stage-select/button1.png",   // up image
        "stage-select/floor.png", // hover image
        halfW - 200, halfH / 2 + 150,
        400, 100
    );
    btnNormal->SetOnClickCallback(
        std::bind(&ModeSelectionScene::NormalModeOnClick, this, 1)
    );
    btnNormal->EnableBreathing(0.05f, 2.0f);
    btnNormal->EnableHoverScale(0.9f);
    AddNewControlObject(btnNormal);
    AddNewObject(new Engine::Label(
        "Normal",
        "balatro.ttf",
        64,
        halfW, halfH / 2 + 200,
        255, 255, 255, 255,
        0.5, 0.5
    ));

    // Survival Mode button
    auto *btnSurvival = new Engine::ImageButton(
        "stage-select/button1.png",
        "stage-select/floor.png",
        halfW - 200, halfH * 3 / 2 - 100,
        400, 100
    );
    btnSurvival->SetOnClickCallback(
        std::bind(&ModeSelectionScene::SurvivalModeOnClick, this, 2)
    );
    btnSurvival->EnableBreathing();
    btnSurvival->EnableHoverScale(0.9f);
    AddNewControlObject(btnSurvival);
    AddNewObject(new Engine::Label(
        "Survival",
        "balatro.ttf",
        64,
        halfW, halfH * 3 / 2 - 50,
        255, 255, 255, 255,
        0.5, 0.5
    ));

    // Back button
    auto* btnBack = new Engine::ImageButton(
        "stage-select/button1.png",         // up image
        "stage-select/floor.png",   // hover image
        halfW - 200,                    // same X as the other two
        halfH * 5 / 2 - 350,            // Y = survivalY + (survivalY - normalY)
        400, 100                        // same size
    );
    btnBack->SetOnClickCallback(
        std::bind(&ModeSelectionScene::BackOnClick, this, 0)
    );
    btnBack->EnableBreathing();
    btnBack->EnableHoverScale(0.9f);
    AddNewControlObject(btnBack);
    AddNewObject(new Engine::Label(
        "Back",
        "balatro.ttf",
        64,
        halfW,                          // center X
        halfH * 5 / 2 - 300,       // backY + btnH/2
        255, 255, 255, 255,
        0.5f, 0.5f                      // center‐center align
    ));
}

void ModeSelectionScene::Terminate() {
    parallax.Unload();
    IScene::Terminate();
}

void ModeSelectionScene::Draw() const {
    auto& eng = GameEngine::GetInstance();
    int  w   = eng.GetScreenSize().x,
         h   = eng.GetScreenSize().y;
    double t = al_get_time();

    // 1) Draw parallax background
    parallax.Draw(w, h, t);

    // 2) Draw this scene’s buttons / sprites / UI
    Group::Draw();
}

void ModeSelectionScene::BackOnClick(int /*unused*/) {
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}

void ModeSelectionScene::NormalModeOnClick(int mode) {
    auto &engine = Engine::GameEngine::GetInstance();
    // 1) grab your PlayScene instance
    auto *play = dynamic_cast<PlayScene*>(engine.GetScene("play"));
    // 2) tell it which map and which mode
    play->MapId   = nextStage;
    play->SetMode(PlayScene::Mode::Normal);
    // 3) go!
    engine.ChangeScene("play");
}

void ModeSelectionScene::SurvivalModeOnClick(int mode) {
    auto &engine = Engine::GameEngine::GetInstance();
    auto *play = dynamic_cast<PlayScene*>(engine.GetScene("play"));
    play->MapId   = nextStage;
    play->SetMode(PlayScene::Mode::Survival);
    engine.ChangeScene("play");
}
