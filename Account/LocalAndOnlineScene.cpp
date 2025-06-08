#include "LocalAndOnlineScene.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"

using namespace Engine;

LocalAndOnlineScene::LocalAndOnlineScene()
    : localButton(nullptr)
    , onlineButton(nullptr)
{ }

LocalAndOnlineScene::~LocalAndOnlineScene() { }

void LocalAndOnlineScene::Initialize() {
    // get screen dimensions
    int w = GameEngine::GetInstance().GetScreenSize().x;
    int h = GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // ── "Local" button ──────────────────────────────────────────────
    localButton = new ImageButton(
        "stage-select/dirt.png",  // up image
        "stage-select/floor.png", // down/hover image
        halfW - 200,              // x position (centered minus half width)
        halfH - 75,               // y position
        400,                      // width
        100                       // height
    );
    localButton->SetOnClickCallback(std::bind(&LocalAndOnlineScene::LocalOnClick, this));
    AddNewControlObject(localButton);

    AddNewObject(new Label(
        "Local",           // text
        "balatro.ttf",     // font
        80,                // size
        halfW,             // x (center of screen)
        halfH - 25,        // y (button center + offset)
        0, 0, 0, 255,      // color: black
        0.5f, 0.5f         // anchor: center
    ));

    // ── "Online" button ─────────────────────────────────────────────
    onlineButton = new ImageButton(
        "stage-select/dirt.png",  // up image
        "stage-select/floor.png", // down/hover image
        halfW - 200,              // x position
        halfH + 75,               // y position (below the Local button)
        400,                      // width
        100                       // height
    );
    onlineButton->SetOnClickCallback(std::bind(&LocalAndOnlineScene::OnlineOnClick, this));
    AddNewControlObject(onlineButton);

    AddNewObject(new Label(
        "Online",          // text
        "balatro.ttf",     // font
        80,                // size
        halfW,             // x (center of screen)
        halfH + 125,       // y (button center + offset)
        0, 0, 0, 255,      // color: black
        0.5f, 0.5f         // anchor: center
    ));
}

void LocalAndOnlineScene::Terminate() {
    IScene::Terminate();
    // Note: individual ImageButton/Label objects will be destroyed by IScene cleanup
}

void LocalAndOnlineScene::LocalOnClick() {
    // when "Local" is clicked, go to the existing LoginScene
    GameEngine::GetInstance().ChangeScene("login");
}

void LocalAndOnlineScene::OnlineOnClick() {
    // when "Online" is clicked, go to our new LoginOnlineScene
    GameEngine::GetInstance().ChangeScene("login-online");
}
