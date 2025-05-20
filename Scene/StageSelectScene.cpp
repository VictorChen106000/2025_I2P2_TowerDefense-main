#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "StageSelectScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"
#include <iostream>
using namespace std;

void StageSelectScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    Engine::ImageButton *btn;
     // --- layout constants ---
    const int btnW    = 400;
    const int btnH    = 100;
    const int startY  = halfH/2 - btnH/2;   // y of the topmost button
    const int gap     = btnH + 50;          // space between buttons
    int x = halfW - btnW/2;

    int y1 = startY + 0*gap;
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", x, y1, btnW, btnH);
    btn->SetOnClickCallback(std::bind(&StageSelectScene::PlayOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Stage 1", "pirulen.ttf", 48, halfW, y1 + btnH / 2, 0, 0, 0, 255, 0.5, 0.5));

    int y2 = startY + 1*gap;
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", x, y2, btnW, btnH);
    btn->SetOnClickCallback(std::bind(&StageSelectScene::PlayOnClick, this, 2));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Stage 2", "pirulen.ttf", 48, halfW, y2 + btnH / 2, 0, 0, 0, 255, 0.5, 0.5));

    int y3 = startY + 2*gap;
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", x, y3, btnW, btnH);
    btn->SetOnClickCallback(std::bind(&StageSelectScene::ScoreboardOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Scoreboard", "pirulen.ttf", 40, halfW, y3 + btnH / 2, 0, 0, 0, 255, 0.5, 0.5));

    int y4 = startY + 3*gap;
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", x, y4, btnW, btnH);
    btn->SetOnClickCallback(std::bind(&StageSelectScene::BackOnClick, this, 0));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "pirulen.ttf", 48, halfW, y4 + btnH / 2, 0, 0, 0, 255, 0.5, 0.5));
}
void StageSelectScene::Terminate() {
    IScene::Terminate();
}
void StageSelectScene::BackOnClick(int) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}
void StageSelectScene::PlayOnClick(int stage) {
    PlayScene *scene = dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetScene("play"));
    scene->MapId = stage;
    Engine::GameEngine::GetInstance().ChangeScene("play");
}
void StageSelectScene::ScoreboardOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
}