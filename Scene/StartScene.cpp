#include "StartScene.h"
#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Scene/StartScene.h"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"

using namespace Engine;
// TODO HACKATHON-2 (1/3): You can imitate the 2 files: 'StartScene.hpp', 'StartScene.cpp' to implement your SettingsScene.
void StartScene::Initialize() {
    parallax.Load({
      "Resource/images/background/wl5.png",
      "Resource/images/background/wl4.png",
      "Resource/images/background/wl3.png",
      "Resource/images/background/wl2.png",
      "Resource/images/background/wl1.png"
    });
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    Engine::ImageButton *btn;

    AddNewObject(new Engine::Label(
        "Tower Defense", 
        "balatro.ttf", 
        120, 
        halfW, 
        halfH / 3 + 50, 
        10, 255, 255, 255, 0.5, 0.5
    ));

    // Play Button
    btn = new Engine::ImageButton(
        "stage-select/button1.png", 
        "stage-select/floor.png", 
        halfW - 200, halfH / 2 + 150, 
        400, 100
    );
    btn->SetOnClickCallback(std::bind(&StartScene::PlayOnClick, this, 1));
    btn->EnableBreathing(0.05f, 2.0f);
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label(
        "Play", 
        "balatro.ttf", 
        64, 
        halfW, 
        halfH / 2 + 200, 
        255, 255, 255, 255, 0.5, 0.5
    ));

    // Setting Button
    btn = new Engine::ImageButton(
        "stage-select/button1.png", 
        "stage-select/floor.png", 
        halfW - 200, 
        halfH * 3 / 2 - 100, 
        400, 100
    );
    btn->SetOnClickCallback(std::bind(&StartScene::SettingsOnClick, this, 2));
    btn->EnableBreathing();
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label(
        "Settings", 
        "balatro.ttf", 
        64, 
        halfW, 
        halfH * 3 / 2 - 50, 
        255, 255, 255, 255, 0.5, 0.5
    ));

    // Conditional Logout/Back Button
    bool fromOnline = (prevScene == "login-online");
    const char* text = fromOnline ? "Logout" : "Back";
    // Y = halfH*5/2 - 300  (same vertical spacing)
    auto* btnLogout = new Engine::ImageButton(
        "stage-select/button1.png",
        "stage-select/floor.png",
        halfW - 200, halfH * 5 / 2 - 350,
        400, 100
    );
    btnLogout->SetOnClickCallback([this, fromOnline](){
        prevScene.clear();
        if (fromOnline) {
            Engine::GameEngine::GetInstance().ChangeScene("login-online");
        }
        else {
            Engine::GameEngine::GetInstance().ChangeScene("login");
        }
    });
    btnLogout->EnableBreathing();
    AddNewControlObject(btnLogout);
    // label centered on that button
    AddNewObject(new Engine::Label(
        text,
        "balatro.ttf",
        64,
        halfW,
        halfH * 5 / 2 - 300,  // center of the button
        255, 255, 255, 255,
        0.5f, 0.5f
    ));
}
void StartScene::Terminate() {
    parallax.Unload();
    IScene::Terminate();
}
void StartScene::Draw() const {
    auto& eng = GameEngine::GetInstance();
    int  w   = eng.GetScreenSize().x,
         h   = eng.GetScreenSize().y;
    double t = al_get_time();

    // 1) Draw parallax background
    parallax.Draw(w, h, t);
    Group::Draw();
}
void StartScene::SetPreviousScene(const std::string& prev) {
    prevScene = prev;
}
void StartScene::PlayOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}
void StartScene::SettingsOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("settings");
}