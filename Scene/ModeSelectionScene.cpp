#include "ModeSelectionScene.hpp"
#include <functional>
#include "Scene/PlayScene.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"

void ModeSelectionScene::Initialize() {
    // center layout
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // Title
    AddNewObject(new Engine::Label(
        "Select Mode",
        "pirulen.ttf",
        120,
        halfW, halfH / 3 + 50,
        10, 255, 255, 255,
        0.5, 0.5
    ));

    // Normal Mode button
    auto *btnNormal = new Engine::ImageButton(
        "stage-select/dirt.png",   // up image
        "stage-select/floor.png", // hover image
        halfW - 200, halfH / 2 + 150,
        400, 100
    );
    btnNormal->SetOnClickCallback(
        std::bind(&ModeSelectionScene::NormalModeOnClick, this, 1)
    );
    AddNewControlObject(btnNormal);
    AddNewObject(new Engine::Label(
        "Normal",
        "pirulen.ttf",
        48,
        halfW, halfH / 2 + 200,
        0, 0, 0, 255,
        0.5, 0.5
    ));

    // Survival Mode button
    auto *btnSurvival = new Engine::ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW - 200, halfH * 3 / 2 - 100,
        400, 100
    );
    btnSurvival->SetOnClickCallback(
        std::bind(&ModeSelectionScene::SurvivalModeOnClick, this, 2)
    );
    AddNewControlObject(btnSurvival);
    AddNewObject(new Engine::Label(
        "Survival",
        "pirulen.ttf",
        48,
        halfW, halfH * 3 / 2 - 50,
        0, 0, 0, 255,
        0.5, 0.5
    ));

    // Back button
    auto* btnBack = new Engine::ImageButton(
        "stage-select/dirt.png",         // up image
        "stage-select/floor.png",   // hover image
        halfW - 200,                    // same X as the other two
        halfH * 5 / 2 - 350,            // Y = survivalY + (survivalY - normalY)
        400, 100                        // same size
    );
    btnBack->SetOnClickCallback(
        std::bind(&ModeSelectionScene::BackOnClick, this, 0)
    );
    AddNewControlObject(btnBack);
    AddNewObject(new Engine::Label(
        "Back",
        "pirulen.ttf",
        48,
        halfW,                          // center X
        halfH * 5 / 2 - 300,       // backY + btnH/2
        0, 0, 0, 255,
        0.5f, 0.5f                      // center‐center align
    ));
}

void ModeSelectionScene::Terminate() {
    IScene::Terminate();
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
