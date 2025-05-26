#include <functional>
#include <string>
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "PlayScene.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "WinScene.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

void WinScene::Initialize() {
    ticks = 0;

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    AddNewObject(new Engine::Image("win/benjamin-sad.png", halfW, halfH, 0, 0, 0.5, 0.5));
    AddNewObject(new Engine::Label("You Win!", "pirulen.ttf", 48, halfW, halfH / 4 - 10, 255, 255, 255, 255, 0.5, 0.5));
    
    boxCenterX = halfW;
    boxCenterY = halfH / 3.0f + 30.0f;
    // the label that will display our typed name
    nameLabel = new Engine::Label("", "pirulen.ttf", 32,
                                  boxCenterX, boxCenterY,
                                  255,255,255,255, 0.5,0.5);
    AddNewObject(nameLabel);

    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("win/dirt.png", "win/floor.png", halfW - 200, halfH * 7 / 4 - 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&WinScene::BackOnClick, this, 2));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "pirulen.ttf", 48, halfW, halfH * 7 / 4, 0, 0, 0, 255, 0.5, 0.5));
    bgmId = AudioHelper::PlayAudio("win.wav");
}
void WinScene::Terminate() {
    IScene::Terminate();
    AudioHelper::StopBGM(bgmId);
}
void WinScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    ticks += deltaTime;
    if (ticks > 4 && ticks < 100 &&
        dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetScene("play"))->MapId == 2) {
        ticks = 100;
        bgmId = AudioHelper::PlayBGM("happy.ogg");
    }
}

void WinScene::Draw() const {
    // 1) draw everything else first
    IScene::Draw();

    // 2) draw our textbox at the center we stored in Initialize()
    const float boxW = 300.0f, boxH = 60.0f;
    const float cx   = boxCenterX;
    const float cy   = boxCenterY;
    const float x1   = cx - boxW * 0.5f;
    const float y1   = cy - boxH * 0.5f;
    const float x2   = cx + boxW * 0.5f;
    const float y2   = cy + boxH * 0.5f;

    // semi-transparent black, radius=10
   al_draw_filled_rounded_rectangle(
        x1, y1, x2, y2,
        10.0f, 10.0f,
        al_map_rgba(0, 0, 0, 0)   // white, 75% opacity
    );
    // optionally add a solid white border:
    al_draw_rounded_rectangle(
        x1, y1, x2, y2,
        10.0f, 10.0f,
        al_map_rgba(255, 255, 255, 255),  // white, 100% opacity
        3.0f                              // line thickness
    );

}


void WinScene::OnKeyChar(int unicode) {
    if (unicode == '\r') {
        BackOnClick(2);
        return;
    }
    else if (unicode == '\b') {
        if (!playerName.empty()) playerName.pop_back();
    }
    else if (unicode >= 32 && playerName.size() < 12) {
        playerName.push_back(char(unicode));
    }
    nameLabel->Text = playerName;
}

void WinScene::BackOnClick(int stage) {
    if (!playerName.empty()) {
        SaveScore();
        playerName.clear();
        nameLabel->Text.clear();
    }
    // Change to select scene.
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}

void WinScene::SaveScore() {
    if (playerName.empty()) return;
    // 1) grab elapsed time from the play scene
    auto* ps = dynamic_cast<PlayScene*>(
        Engine::GameEngine::GetInstance().GetScene("play"));
    float t = (ps ? ps->GetElapsedTime() : 0.0f);

    // 2) compute a simple time‐bonus score
    int kills     = ps->GetKillCount();
    int timeBonus = static_cast<int>(10000.0f / (t + 1));
    int moneyLeft = ps->GetMoney();
    int score     = kills * 30 + timeBonus * 1 + moneyLeft * 2; 

    // 3) append "Name Score" to Resource/scoreboard.txt
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm localTm;
    localtime_s(&localTm, &now_c);
    std::ostringstream oss;
    oss << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S");
    std::string timeStr = oss.str();
    std::ofstream ofs("../Resource/scoreboard.txt", std::ios::app);
    if (ofs.is_open()) {
        ofs << playerName << " "
            << score << " "
            << timeStr << "\n";
    }
}