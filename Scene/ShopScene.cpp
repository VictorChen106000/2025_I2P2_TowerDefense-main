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
static float NINJA_ADJUST_X   =  460.0f;
static float NINJA_ADJUST_Y   =  -200.0f;
static float PODIUM_ADJUST_X  =  0.0f;
static float PODIUM_ADJUST_Y  = 130.0f;
static const float IDLE_SCALE = 7.0f;

void ShopScene::Initialize() {
    // 1) BGM
    bgmInstance = AudioHelper::PlaySample("levelup.ogg", true, AudioHelper::BGMVolume);

    // 2) Screen dims & center
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // 3) Background
    AddNewObject(new Engine::Image("background/dirty_2.png", 0, 0, w, h));

    // 4) Crystal icon + count
    const int ICON = 96, PAD = 20;
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

    // 5) Title
    AddNewObject(new Engine::Label(
        "WELCOME TO Lee Be SHOP", "Balatro.ttf", 48,
        halfW, PAD,
        0,255,0,255,
        0.5f, 0.0f
    ));

    // 6) Ninja‐idle animation
    idleBmp       = Engine::Resources::GetInstance()
                     .GetBitmap("play/yellowninjaidle.png").get();
    int sheetW    = al_get_bitmap_width(idleBmp);
    int sheetH    = al_get_bitmap_height(idleBmp);

    idleCols      = 2;
    idleFrameW    = sheetW / idleCols;
    idleFrameH    = sheetH;

    idleCurFrame  = 0;
    idleTimer     = 0.0f;
    idleFrameTime = 0.25f;

    float scaledW = idleFrameW * IDLE_SCALE;
    float scaledH = idleFrameH * IDLE_SCALE;
    idleX = (halfW - scaledW/2) + NINJA_ADJUST_X;
    idleY = (halfH - scaledH/2) + NINJA_ADJUST_Y;

    // 7) Podium
    const int PW = 300, PH = 400;
    float podiumX = (halfW - PW/2) + PODIUM_ADJUST_X;
    float podiumY = (halfH - PH/2) + PODIUM_ADJUST_Y;
    AddNewObject(new Engine::Image(
        "play/podium.png",
        static_cast<int>(podiumX),
        static_cast<int>(podiumY),
        PW, PH
    ));

    // 8) Buy button under podium
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
        AudioHelper::PlaySample("cancel.ogg", false, AudioHelper::SFXVolume);
        return;
    }
    // subtract the crystals, award the item, update UI
    p->EarnCoin(-buyCost);
    p->EarnMoney(100);
    crystalCountLbl->Text = std::to_string(p->GetGoldCoins());
});

// hover & breathing  
buyBtn->EnableBreathing(0.05f, 2.0f);
buyBtn->EnableHoverScale(0.9f);
AddNewControlObject(buyBtn);

// ——————————
// 8a) Add the “Buy” label centered in the button:
auto buyLabel = new Engine::Label(
    "Buy", "Balatro.ttf", 24,
    bx + BW/2, by + BH/2,
    255,255,255,255,
    0.5f, 0.5f
);
AddNewObject(buyLabel);

// 8b) Move your crystal icon + cost label to the right of “Buy”:
const int CI = 32;
int iconX = bx + BW + 10;                 // 10px to the right of the button
int iconY = by + (BH - CI)/2;
buyCostIcon = new Engine::Image("play/crystal.png", iconX, iconY, CI, CI);
buyCostLabel = new Engine::Label(
    std::to_string(buyCost), "Balatro.ttf", 28,
    iconX + CI + 4, iconY + CI/2,
    255,255,255,255,
    0.0f, 0.5f
);
AddNewObject(buyCostIcon);
AddNewObject(buyCostLabel);

    // 10) Back button (bottom-left)
    const int BW2 = 200, BH2 = 80, M = 20;
    backBtn = new Engine::ImageButton(
        "stage-select/button1.png","stage-select/floor.png",
        PAD, h - BH2 - M, BW2, BH2
    );
    backBtn->SetOnClickCallback(
      std::bind(&ShopScene::BackOnClick, this, 0)
    );
    // ← same hover & breathing
    backBtn->EnableBreathing(0.05f, 2.0f);
    backBtn->EnableHoverScale(0.9f);
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
