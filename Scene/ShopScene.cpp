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
#include "ShopScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"
#include <iostream>
using namespace std;

void ShopScene::Initialize() {
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

    int y4 = startY + 3*gap;
    btn = new Engine::ImageButton("stage-select/button1.png", "stage-select/floor.png", x, y4, btnW, btnH);
    btn->SetOnClickCallback(std::bind(&ShopScene::BackOnClick, this, 0));
    btn->EnableBreathing(0.05f, 2.0f);
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "balatro.ttf", 64, halfW, y4 + btnH / 2, 255, 255, 255, 255, 0.5, 0.5));

    AddNewObject(new Engine::Label("WELCOME TO Lee Be SHOP", "balatro.ttf", 64, halfW, 100, 0, 255, 0, 255, 0.5, 0.5));
}
void ShopScene::Terminate() {
    IScene::Terminate();
}
void ShopScene::BackOnClick(int) {
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}
void ShopScene::PlayOnClick(int stage) {
    PlayScene *scene = dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetScene("play"));
    scene->MapId = stage;
    Engine::GameEngine::GetInstance().ChangeScene("play");
}
void ShopScene::ScoreboardOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
}