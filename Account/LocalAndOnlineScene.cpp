#include "LocalAndOnlineScene.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include <allegro5/allegro_image.h>
#include <cmath>

using namespace Engine;
// scroll speeds in pixels/sec
namespace {
    constexpr float L1_SPEED = 300.0f; // front layer (fastest)
    constexpr float L2_SPEED = 150.0f; // middle
    constexpr float L3_SPEED =  75.0f; // third
    constexpr float L4_SPEED =  37.5f; // back (just in front of static)
}

LocalAndOnlineScene::LocalAndOnlineScene()
    : background(nullptr)
    , layer4(nullptr)
    , layer3(nullptr)
    , layer2(nullptr)
    , layer1(nullptr)
    , localButton(nullptr)
    , onlineButton(nullptr)
{ }

LocalAndOnlineScene::~LocalAndOnlineScene() { }

void LocalAndOnlineScene::Initialize() {
    // load depth-4 (static) + depths 3–1 for scrolling
    // background = al_load_bitmap("Resource/images/background/Day/d4.png");
    // layer3     = al_load_bitmap("Resource/images/background/Day/d3.png");
    // layer2     = al_load_bitmap("Resource/images/background/Day/d2.png");
    // layer1     = al_load_bitmap("Resource/images/background/Day/d1.png");

    background = al_load_bitmap("Resource/images/background/wl5.png");
    layer4     = al_load_bitmap("Resource/images/background/wl4.png");
    layer3     = al_load_bitmap("Resource/images/background/wl3.png");
    layer2     = al_load_bitmap("Resource/images/background/wl2.png");
    layer1     = al_load_bitmap("Resource/images/background/wl1.png");

    if (!background || !layer4 || !layer3 || !layer2 || !layer1) {
        printf("Failed to load one of the background layers\n");
    }

    // al_convert_mask_to_alpha(layer1, al_map_rgb(255,255,255));
    // al_convert_mask_to_alpha(layer2, al_map_rgb(255,255,255));
    // al_convert_mask_to_alpha(layer3, al_map_rgb(255,255,255));

    // get screen dimensions
    int w = GameEngine::GetInstance().GetScreenSize().x;
    int h = GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // ── "Local" button ──────────────────────────────────────────────
    localButton = new ImageButton(
        "stage-select/button1.png",  // up image
        "stage-select/floor.png", // down/hover image
        halfW - 200,              // x position (centered minus half width)
        halfH - 75,               // y position
        400,                      // width
        100                       // height
    );
    localButton->SetOnClickCallback(std::bind(&LocalAndOnlineScene::LocalOnClick, this));
    localButton->EnableBreathing(0.05f, 2.0f);
    AddNewControlObject(localButton);

    AddNewObject(new Label(
        "Local",           // text
        "balatro.ttf",     // font
        80,                // size
        halfW,             // x (center of screen)
        halfH - 25,        // y (button center + offset)
        255,255,255,255,      // color: black
        0.5f, 0.5f         // anchor: center
    ));

    // ── "Online" button ─────────────────────────────────────────────
    onlineButton = new ImageButton(
        "stage-select/button1.png",  // up image
        "stage-select/floor.png", // down/hover image
        halfW - 200,              // x position
        halfH + 75,               // y position (below the Local button)
        400,                      // width
        100                       // height
    );
    onlineButton->SetOnClickCallback(std::bind(&LocalAndOnlineScene::OnlineOnClick, this));
    onlineButton->EnableBreathing();
    AddNewControlObject(onlineButton);

    AddNewObject(new Label(
        "Online",          // text
        "balatro.ttf",     // font
        80,                // size
        halfW,             // x (center of screen)
        halfH + 125,       // y (button center + offset)
        255,255,255,255,      // color: black
        0.5f, 0.5f         // anchor: center
    ));
}

void LocalAndOnlineScene::Draw() const {
    // 1) clear once
    al_clear_to_color(al_map_rgb(0,0,0));

    // 2) screen dims & time
    int w = GameEngine::GetInstance().GetScreenSize().x;
    int h = GameEngine::GetInstance().GetScreenSize().y;
    double t = al_get_time();

    // 3) static depth-4 background
    int bgW = al_get_bitmap_width(background),
        bgH = al_get_bitmap_height(background);
    al_draw_scaled_bitmap(
        background, 0,0, bgW,bgH,
        0,0, w,h,
        0
    );

    // 4) helper to draw each parallax layer
    auto drawLayer = [&](ALLEGRO_BITMAP* bmp, float speed){
    int bw = al_get_bitmap_width(bmp),
        bh = al_get_bitmap_height(bmp);
    // 1) how much we scale the bitmap to fill screen height
    float scale = float(h) / float(bh);
    // 2) its scaled width
    float sw = bw * scale;
    // 3) how far to shift (wrap at sw, not screen-width)
    float xoff = fmodf(t * speed, sw);
    // 4) draw two copies side by side at sw
    al_draw_scaled_bitmap(bmp, 0,0, bw,bh,
                          -xoff,    0, sw,h, 0);
    al_draw_scaled_bitmap(bmp, 0,0, bw,bh,
                          -xoff+sw, 0, sw,h, 0);
    };

    // 5) draw scrolling layers in back→front order
    drawLayer(layer4, L4_SPEED);
    drawLayer(layer3, L3_SPEED);
    drawLayer(layer2, L2_SPEED);
    drawLayer(layer1, L1_SPEED);

    // 6) finally draw your buttons & labels **once**
    Group::Draw();
}

void LocalAndOnlineScene::Terminate() {
    // destroy all 5 layers
    for (ALLEGRO_BITMAP** bmp : { &background, &layer4, &layer3, &layer2, &layer1 }) {
        if (*bmp) { al_destroy_bitmap(*bmp); *bmp = nullptr; }
    }
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
