#include <functional>
#include <string>
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "PlayScene.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "Account/ScoreboardOnline.hpp"
#include "WinScene.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>

extern std::string CurrentUser;

void WinScene::Initialize() {
    ticks = 0;

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    AddNewObject(new Engine::Image("win/benjamin-sad.png", halfW, halfH, 0, 0, 0.5, 0.5));
    AddNewObject(new Engine::Label("You Win!", "pirulen.ttf", 48, halfW, halfH / 4 - 10, 255, 255, 255, 255, 0.5, 0.5));

    //remove input player name

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
    IScene::Draw();
    //no need because we have account
}



void WinScene::BackOnClick(int stage) {
    SaveScore();
    // Change to select scene.
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}

void WinScene::SaveScore() {
    // 1) only write if someone is logged in
    if (CurrentUser.empty()) return;

    // 2) grab elapsed time from PlayScene
    auto* ps = dynamic_cast<PlayScene*>(
        Engine::GameEngine::GetInstance().GetScene("play"));
    float t = (ps ? ps->GetElapsedTime() : 0.0f);

    // 3) compute a simple time‐bonus score
    int kills     = ps->GetKillCount();
    int timeBonus = static_cast<int>(10000.0f / (t + 1));
    int moneyLeft = ps->GetMoney();
    int score     = kills * 30 + timeBonus + moneyLeft * 2; 

    if (!ScoreboardOnline::idToken.empty()) {
          bool ok = ScoreboardOnline::UploadScore(CurrentUser, score);
             if (!ok) {
                 std::cerr << "[WinScene] UploadScore failed\n";
              }
              return;
      }
    // 4) build timestamp string
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm localTm;
#if defined(_WIN32)
    localtime_s(&localTm, &now_c);
#else
    localtime_r(&now_c, &localTm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S");
    std::string timeStr = oss.str();

    // 5) append "CurrentUser score timestamp" to scoreboard.txt
    std::ofstream ofs("../Resource/scoreboard.txt", std::ios::app);
    if (ofs.is_open()) {
        ofs << CurrentUser << " "
            << score << " "
            << timeStr << "\n";
    }
}
