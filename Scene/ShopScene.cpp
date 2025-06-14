// ShopScene.cpp
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro.h>
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Resources.hpp"
#include "Scene/PlayScene.hpp"
#include "Scene/StageSelectScene.hpp"
#include "Scene/ShopScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/Label.hpp"

// ─── Adjustable Offsets ────────────────────────────────────────────────
// These are *added* to the screen-center when placing each element.
static float NINJA_ADJUST_X   =  0.0f; // shift ninja left/right
static float NINJA_ADJUST_Y   =  0.0f; // shift ninja up/down
static float PODIUM_ADJUST_X  =  0.0f; // shift podium left/right
static float PODIUM_ADJUST_Y  = 130.0f; // shift podium up/down
static const float IDLE_SCALE = 7.0f;  // how much to scale up ninja frames

void ShopScene::Initialize() {
    // 1) BGM
    bgmInstance = AudioHelper::PlaySample("levelup.ogg", true, AudioHelper::BGMVolume);

    // 2) Screen dims & center
    int w     = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h     = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // 3) Background
    AddNewObject(new Engine::Image("background/dirty_2.png", 0, 0, w, h));

    // 4) Crystal icon + count (top-left)
    const int ICON = 64, PAD = 20;
    auto play    = dynamic_cast<PlayScene*>(
                     Engine::GameEngine::GetInstance().GetScene("play"));
    int crystals = play ? play->GetGoldCoins() : 0;
    AddNewObject(new Engine::Image("play/crystal.png", PAD, PAD, ICON, ICON));
    crystalCountLbl = new Engine::Label(
        std::to_string(crystals), "Balatro.ttf", 36,
        PAD + ICON + 5, PAD + ICON/2,
        255,255,255,255, 0.0f, 0.5f
    );
    AddNewObject(crystalCountLbl);

    // 5) Title (top-center)
    AddNewObject(new Engine::Label(
        "WELCOME TO Lee Be SHOP", "Balatro.ttf", 48,
        halfW, PAD,
        0,255,0,255,
        0.5f, 0.0f
    ));

    // 6) Ninja‐idle animation setup
    idleBmp       = Engine::Resources::GetInstance()
                     .GetBitmap("play/yellowninjaidle.png").get();
    int sheetW    = al_get_bitmap_width(idleBmp);
    int sheetH    = al_get_bitmap_height(idleBmp);
    idleCols      = 4;
    idleFrameW    = sheetW / idleCols;
    idleFrameH    = sheetH;
    idleCurFrame  = 0;
    idleTimer     = 0.0f;

    // 7) Compute ninja center + adjust
    float scaledW = idleFrameW * IDLE_SCALE;
    float scaledH = idleFrameH * IDLE_SCALE;
    idleX = (halfW - scaledW/2) + NINJA_ADJUST_X;
    idleY = (halfH - scaledH/2) + NINJA_ADJUST_Y;

    // 8) Podium IMAGE at center + adjust
    const int PW = 300, PH = 400;
    float podiumX = (halfW - PW/2) + PODIUM_ADJUST_X;
    float podiumY = (halfH - PH/2) + PODIUM_ADJUST_Y;
    AddNewObject(new Engine::Image(
        "play/podium.png",
        static_cast<int>(podiumX),
        static_cast<int>(podiumY),
        PW, PH
    ));

    // 9) Buy button under podium (y based on podiumY+PH+20)
    const int BW = 120, BH = 50;
    int bx = halfW - BW/2;
    int by = static_cast<int>(podiumY) + PH + 20;
    buyBtn = new Engine::ImageButton(
        "stage-select/button1.png","stage-select/floor.png",
        bx, by, BW, BH
    );
    buyBtn->SetOnClickCallback([this](){
        auto p = dynamic_cast<PlayScene*>(
                   Engine::GameEngine::GetInstance().GetScene("play"));
        if (!p || p->GetGoldCoins() < buyCost) {
            AudioHelper::PlaySample(
              "cancel.ogg", false, AudioHelper::SFXVolume
            );
            return;
        }
        p->EarnCoin(-buyCost);
        p->EarnMoney(100);
        crystalCountLbl->Text = std::to_string(p->GetGoldCoins());
    });
    AddNewControlObject(buyBtn);

    // 10) Cost icon + label
    const int CI = 32;
    int ix = bx + 10;
    int iy = by + (BH - CI)/2;
    buyCostIcon  = new Engine::Image(
        "play/crystal.png", ix, iy, CI, CI
    );
    buyCostLabel = new Engine::Label(
        std::to_string(buyCost), "Balatro.ttf", 28,
        ix + CI + 8, iy + CI/2,
        255,255,255,255, 0.0f, 0.5f
    );
    AddNewObject(buyCostIcon);
    AddNewObject(buyCostLabel);

    // 11) Back button (bottom-left corner)
    const int BW2 = 200, BH2 = 80, M = 20;
    backBtn = new Engine::ImageButton(
        "stage-select/button1.png","stage-select/floor.png",
        PAD, h - BH2 - M, BW2, BH2
    );
    backBtn->SetOnClickCallback(
      std::bind(&ShopScene::BackOnClick, this, 0)
    );
    AddNewControlObject(backBtn);
    AddNewObject(new Engine::Label(
        "Back","Balatro.ttf",32,
        PAD + BW2/2, h - BH2/2 - M,
        255,255,255,255, 0.5f, 0.5f
    ));
}

void ShopScene::Update(float dt) {
    IScene::Update(dt);
    bool ok = false;
    if (auto p = dynamic_cast<PlayScene*>(
          Engine::GameEngine::GetInstance().GetScene("play")))
      ok = (p->GetGoldCoins() >= buyCost);
    buyBtn->Enabled = ok;

    idleTimer += dt;
    if (idleTimer >= idleFrameTime) {
        idleTimer   -= idleFrameTime;
        idleCurFrame = (idleCurFrame + 1) % idleCols;
    }
}

void ShopScene::Draw() const {
    IScene::Draw();
    // draw single ninja sprite at idleX, idleY
    if (idleBmp) {
        al_draw_scaled_bitmap(
            idleBmp,
            idleCurFrame * idleFrameW, 0,
            idleFrameW, idleFrameH,
            static_cast<int>(idleX),
            static_cast<int>(idleY),
            static_cast<int>(idleFrameW * IDLE_SCALE),
            static_cast<int>(idleFrameH * IDLE_SCALE),
            0
        );
    }
}

void ShopScene::Terminate() {
    if (bgmInstance) {
        AudioHelper::StopSample(bgmInstance);
        bgmInstance.reset();
    }
    IScene::Terminate();
}

void ShopScene::BackOnClick(int) {
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}

void ShopScene::PlayOnClick(int stage) {
    auto s = dynamic_cast<PlayScene*>(
      Engine::GameEngine::GetInstance().GetScene("play")
    );
    if (s) s->MapId = stage;
    Engine::GameEngine::GetInstance().ChangeScene("play");
}

void ShopScene::ScoreboardOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
}
