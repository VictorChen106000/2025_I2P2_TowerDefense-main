#include <allegro5/allegro_audio.h>
#include <allegro5/allegro.h>
#include <functional>
#include <memory>
#include <string>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "StageSelectScene.hpp"
#include "ShopScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"

static const float idleScale = 7.0f;

void ShopScene::Initialize() {
    // ------------------------------------------------
    // 1) background music
    bgmInstance = AudioHelper::PlaySample("levelup.ogg", true, AudioHelper::BGMVolume);


    // 2) screen size
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;

    // --- fetch current crystal count from PlayScene
    auto play = dynamic_cast<PlayScene*>(
      Engine::GameEngine::GetInstance().GetScene("play")
    );
    int crystals = play ? play->GetGoldCoins() : 0;

    // --- big crystal icon + count top-left ---
    const int iconSize = 64;
    const int padX     = 20;
    const int padY     = 20;

    // icon
    AddNewObject(new Engine::Image(
      "play/crystal.png",
      padX, padY,
      iconSize, iconSize
    ));
    // count label (store pointer so we can update later)
    crystalCountLbl = new Engine::Label(
      std::to_string(crystals),
      "Balatro.ttf",
      36,               
      padX + iconSize + 15,  
      padY + iconSize/2,    
      255,255,255,255,
      0.0f, 0.5f          
    );
    AddNewObject(crystalCountLbl);

    // --- podium graphic beneath ninja ---
    const int podiumW = 300;
    const int podiumH = 400;
    const int podiumX = 200;
    const int podiumY = 425;
    AddNewObject(new Engine::Image(
      "play/podium.png",
      podiumX, podiumY,
      podiumW, podiumH
    ));

    // ─── ninja‐idle setup ───────────────────────────────
    {
      idleBmp = Engine::Resources::GetInstance()
                  .GetBitmap("play/yellowninjaidle.png")
                  .get();
      int sheetW = al_get_bitmap_width(idleBmp);
      int sheetH = al_get_bitmap_height(idleBmp);
      idleFrameW = sheetW / idleCols;
      idleFrameH = sheetH;
      // center above podium
      idleX = podiumX + (podiumW - idleFrameW) * 0.5f;
      idleY = podiumY - idleFrameH - 20.0f;
    }

    // --- Buy button + cost UI just below podium ---
    const int btnW = 120, btnH = 50;
    int bx = podiumX + (podiumW - btnW)/2;
    int by = podiumY + podiumH + 20;

    buyBtn = new Engine::ImageButton(
      "play/crystal.png",
      "play/crystal.png",
      bx, by, btnW, btnH
    );
    // hook up callback
    buyBtn->SetOnClickCallback([this](){
      auto play = dynamic_cast<PlayScene*>(
        Engine::GameEngine::GetInstance().GetScene("play")
      );
      if (!play) return;
      if (play->GetGoldCoins() >= 1) {
        // subtract one crystal
        play->EarnCoin(-1);
        // update both ShopScene’s label and PlayScene’s saved coin count
        crystalCountLbl->Text = std::to_string(play->GetGoldCoins());
        buyCostLabel->Text    = "1";  
      } else {
        AudioHelper::PlaySample("cancel.ogg", false, AudioHelper::SFXVolume);
      }
    });
    AddNewControlObject(buyBtn);

    // cost icon & “1”
    const int costIconSize = 32;
    int ix = bx + 10;
    int iy = by + (btnH - costIconSize)/2;
    buyCostIcon = new Engine::Image(
      "play/crystal.png",
      ix, iy,
      costIconSize, costIconSize
    );
    
    AddNewObject(buyCostIcon);

    buyCostLabel = new Engine::Label(
      "1",
      "Balatro.ttf",
      28,
      ix + costIconSize + 8,
      iy + costIconSize/2,
      255,255,255,255,
      0.0f, 0.5f
    );
    AddNewObject(buyCostLabel);

    // --- existing “Back” button + title as before ---
    int halfW = w/2, halfH = h/2;
    const int menuW = 400, menuH = 100, gap = menuH + 50;
    int mx = halfW - menuW/2;
    int my = halfH/2 - menuH/2 + 3*gap;

    auto back = new Engine::ImageButton(
      "stage-select/button1.png",
      "stage-select/floor.png",
      mx, my, menuW, menuH
    );
    back->SetOnClickCallback(std::bind(&ShopScene::BackOnClick, this, 0));
    back->EnableBreathing(0.05f, 2.0f);
    back->EnableHoverScale(0.9f);
    AddNewControlObject(back);

    AddNewObject(new Engine::Label(
      "Back","Balatro.ttf",64,
      halfW, my + menuH/2,
      255,255,255,255, 0.5f,0.5f
    ));
    AddNewObject(new Engine::Label(
      "WELCOME TO Lee Be SHOP","Balatro.ttf",64,
      halfW,100,
      0,255,0,255, 0.5f,0.5f
    ));
}

void ShopScene::Terminate() {
    if (bgmInstance) {
        AudioHelper::StopSample(bgmInstance);
    }
    IScene::Terminate();
}

void ShopScene::BackOnClick(int) {
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}

void ShopScene::PlayOnClick(int stage) {
    auto scene = dynamic_cast<PlayScene*>(
      Engine::GameEngine::GetInstance().GetScene("play")
    );
    if (scene) scene->MapId = stage;
    Engine::GameEngine::GetInstance().ChangeScene("play");
}

void ShopScene::ScoreboardOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
}

void ShopScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    // advance ninja‐idle
    idleTimer += deltaTime;
    if (idleTimer >= idleFrameTime) {
      idleTimer    -= idleFrameTime;
      idleCurFrame = (idleCurFrame + 1) % idleCols;
    }
}

void ShopScene::Draw() const {
    IScene::Draw();
    // draw current ninja‐idle frame
    if (idleBmp) {
      float extraW = (idleFrameW*idleScale - idleFrameW)*0.5f;
      float extraH = (idleFrameH*idleScale - idleFrameH)*0.5f;
      al_draw_scaled_bitmap(
        idleBmp,
        idleCurFrame*idleFrameW, 0,
        idleFrameW, idleFrameH,
        idleX - extraW, idleY - extraH,
        idleFrameW*idleScale, idleFrameH*idleScale,
        0
      );
    }
}
