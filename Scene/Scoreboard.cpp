// Scoreboard.cpp
#include "Scoreboard.hpp"
#include "Account/ScoreboardOnline.hpp"            // for FirebaseService
#include "Engine/GameEngine.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Image.hpp" 
#include "UI/Component/Label.hpp"
#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>    // std::max, std::min, std::sort
#include <cctype>       // std::isspace

void Scoreboard::Initialize() {
    // bgBitmap = al_load_bitmap("Resource/images/background/sb-bg.png");
    // if (!bgBitmap) {
    //     printf("[Scoreboard] failed to load bg image\n");
    // } else printf("SB BG OK!\n");
    // 1) load all scores + timestamps (cloud if signed in, otherwise local file)
    scores.clear();
    if (!ScoreboardOnline::idToken.empty()) {
        // online mode
        auto entries = ScoreboardOnline::FetchTopScores(100);
        for (const auto &e : entries) {
            ScoreEntry se;
            se.name      = e.first;
            se.pts       = e.second;
            se.timestamp = "";  // no timestamp in online version
            scores.push_back(se);
        }
    } else {
        // local mode
        std::ifstream ifs("Resource/scoreboard.txt");
        if (!ifs) {
            std::cerr << "[Scoreboard] cannot open Resource/scoreboard.txt\n";
        } else {
            std::string line;
            while (std::getline(ifs, line)) {
                if (line.empty()) continue;
                std::istringstream iss(line);
                ScoreEntry e;
                iss >> e.name >> e.pts;
                std::getline(iss, e.timestamp);
                // trim leading whitespace
                if (!e.timestamp.empty() && std::isspace(e.timestamp.front())) {
                    e.timestamp.erase(0, e.timestamp.find_first_not_of(" \t"));
                }
                scores.push_back(e);
            }
        }
    }

    // 2) layout constants
    int w     = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h     = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;

     // 2a) full-screen background (replace “bg.png” with whatever you actually have)
    AddNewObject(new Engine::Image(
        "background/dirty_2.png",  // e.g. Resource/images/background/bg.png
        0, 0,
        w, h
    ));

    // 2b) scoreboard panel at 80% of screen size, centered
    float panelW = w * 0.8f;
    float panelH = h * 0.8f;
    AddNewObject(new Engine::Image(
        "background/sb-bg2.png",  // your decorative frame
        (w - panelW) * 0.5f,
        (h - panelH) * 0.5f,
        panelW,
        panelH
    ));

    constexpr float leftMarginPct  = 0.20f;  // 10% from left
    constexpr float rightMarginPct = 0.80f;  // 10% from right

    const float colNameX  = w * leftMarginPct;
    const float colScoreX = w * 0.50f;
    const float colTimeX  = w * rightMarginPct;

    const int titleY      = 60;
    const int startY      = 180;
    const int bottomY     = h - 100;
    const int lineSpacing = 40;

    // calculate pagination
    itemsPerPage = (bottomY - startY) / lineSpacing;
    int maxPage  = (scores.size() - 1) / itemsPerPage;
    currentPage  = std::max(0, std::min(currentPage, maxPage));

    // 2c) small background behind the title
    float titleBgW = w * 0.2f;              // half screen–wide
    float titleBgH = 90;                    // 60px tall
    float titleBgX = halfW - titleBgW/2;    // centered
    float titleBgY = titleY - titleBgH/2;   // vertically center on titleY

    AddNewObject(new Engine::Image(
        "background/sb-title-bg.png",                  // your little art file
        titleBgX, titleBgY + 5,
        titleBgW, titleBgH
    ));
    // 3) draw title
    AddNewObject(new Engine::Label(
        "SCOREBOARD",
        "balatro.ttf", 32,
        halfW, titleY,
        255, 255, 255, 255,  // green text
        0.5f, 0.5f       // centered
    ));

    // 4) draw each entry on this page
    for (int i = 0; i < itemsPerPage; ++i) {
        int idx = currentPage * itemsPerPage + i;
        if (idx >= scores.size()) break;
        const auto &e = scores[idx];
        float y = startY + i * lineSpacing;

        AddNewObject(new Engine::Label(
            e.name,
            "balatro.ttf", 32,
            colNameX, y,
            255, 255, 255, 255,  // white text
            0.f, 0.5f            // left-aligned
        ));

        AddNewObject(new Engine::Label(
            std::to_string(e.pts),
            "balatro.ttf", 32,
            colScoreX, y,
            255, 255, 255, 255,
            0.f, 0.5f
        ));

        AddNewObject(new Engine::Label(
            e.timestamp,
            "balatro.ttf", 32,
            colTimeX, y,
            255, 255, 255, 255,
            1.f, 0.5f            // right-aligned
        ));
    }

    // 5) bottom buttons: Prev, Back, Next
    const int btnW   = 200, btnH = 50, margin = 20;
    const int totalW = 3 * btnW + 2 * margin;
    const int startX = halfW - totalW / 2;
    const int btnY   = bottomY;

    // Prev Page
    {
        int x = startX + 0 * (btnW + margin);
        auto *b = new Engine::ImageButton(
            "stage-select/button1.png",
            "stage-select/floor.png",
            x - 180, btnY - 10, btnW + 150, btnH + 20
        );
        b->SetOnClickCallback(std::bind(&Scoreboard::PrevPageOnClick, this));
        b->EnableBreathing(0.05f, 2.0f);
        b->EnableHoverScale(0.9f);
        AddNewControlObject(b);
        AddNewObject(new Engine::Label(
            "PREV PAGE",
            "balatro.ttf", 32,
            x + btnW/2 - 100, btnY + btnH/2,
            255, 255, 255, 255,
            0.5f, 0.5f
        ));
    }

    // Back
    {
        int x = startX + 1 * (btnW + margin);
        auto *b = new Engine::ImageButton(
            "stage-select/button1.png",
            "stage-select/floor.png",
            x, btnY -10, btnW, btnH + 20
        );
        b->SetOnClickCallback(std::bind(&Scoreboard::BackOnClick, this));
        b->EnableBreathing();
        b->EnableHoverScale(0.9f);
        AddNewControlObject(b);
        AddNewObject(new Engine::Label(
            "BACK",
            "balatro.ttf", 32,
            x + btnW/2, btnY + btnH/2,
            255, 255, 255, 255,
            0.5f, 0.5f
        ));
    }

    // Next Page
    {
        int x = startX + 2 * (btnW + margin);
        auto *b = new Engine::ImageButton(
            "stage-select/button1.png",
            "stage-select/floor.png",
            x + 20, btnY - 10, btnW + 150, btnH + 20
        );
        b->SetOnClickCallback(std::bind(&Scoreboard::NextPageOnClick, this));
        b->EnableBreathing();
        b->EnableHoverScale(0.9f);
        AddNewControlObject(b);
        AddNewObject(new Engine::Label(
            "NEXT PAGE",
            "balatro.ttf", 32,
            x + btnW/2 + 100, btnY + btnH/2,
            255, 255, 255, 255,
            0.5f, 0.5f
        ));
    }
}

// void Scoreboard::Draw() const {
//     // clear
//     al_clear_to_color(al_map_rgb(0,0,0));

//     // draw full-screen bg
//     int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
//     int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
//     int bw = al_get_bitmap_width(bgBitmap);
//     int bh = al_get_bitmap_height(bgBitmap);
//     al_draw_scaled_bitmap(bgBitmap,
//                           0,0, bw,bh,
//                           0,0, w,h,
//                           0);

//     // then let IScene draw all your labels & buttons
//     Group::Draw();
// }

void Scoreboard::NextPageOnClick() {
    int maxPage = (scores.size() - 1) / itemsPerPage;
    if (currentPage < maxPage) {
        ++currentPage;
        Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
    }
}

void Scoreboard::BackOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}

void Scoreboard::PrevPageOnClick(){
  if(currentPage > 0){
    --currentPage;
    Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
  }
}

void Scoreboard::Terminate(){
    // if (bgBitmap) {
    //     al_destroy_bitmap(bgBitmap);
    //     bgBitmap = nullptr;
    // }
  IScene::Terminate();
}
