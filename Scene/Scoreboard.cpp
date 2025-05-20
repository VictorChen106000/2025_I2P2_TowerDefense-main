#include "Scoreboard.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>    // for std::min

void Scoreboard::Initialize() {
    // 1) load all scores from file
    scores.clear();
    {
      std::ifstream ifs("Resource/scoreboard.txt");
      if (!ifs) {
        std::cerr << "[Scoreboard] cannot open Resource/scoreboard.txt\n";
      } else {
        std::string name;
        int pts;
        while (ifs >> name >> pts) {
          scores.emplace_back(name, pts);
        }
      }
    }

    // 2) layout constants
    int w     = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h     = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w/2;

    const int titleY        = 80;
    const int startY        = 160;
    const int lineSpacing   = 60;
    const int bottomY       = h - 100;

    // how many lines fit between startY and bottomY?
    itemsPerPage = (bottomY - startY) / lineSpacing;

    // clamp currentPage so it’s valid
    int maxPage = (scores.size() - 1) / itemsPerPage;
    currentPage = std::max(0, std::min(currentPage, maxPage));

    // 3) draw title
    AddNewObject(new Engine::Label(
      "SCOREBOARD","pirulen.ttf",64,
      halfW, titleY,
      0,255,0,255,  // green
      0.5f,0.5f
    ));

    // 4) draw just the slice for this page
    int first = currentPage * itemsPerPage;
    int last  = std::min<int>(first + itemsPerPage, scores.size());
    for (int idx = first, row = 0; idx < last; ++idx, ++row) {
      auto &p = scores[idx];
      std::string text = p.first + "   " + std::to_string(p.second);

      AddNewObject(new Engine::Label(
        text, "pirulen.ttf", 48,
        halfW, startY + row * lineSpacing,
        0,255,0,255,
        0.5f,0.5f
      ));
    }

    // 5) bottom three buttons (same spacing logic as before)
    const int btnW   = 200, btnH = 50, margin = 20;
    const int totalW = 3*btnW + 2*margin;
    const int startX = halfW - totalW/2;
    const int btnY   = bottomY;

    // Prev Page
    {
      int x = startX + 0*(btnW+margin);
      auto *b = new Engine::ImageButton(
        "stage-select/dirt.png","stage-select/floor.png",
        x, btnY, btnW, btnH
      );
      b->SetOnClickCallback(std::bind(&Scoreboard::PrevPageOnClick, this));
      AddNewControlObject(b);
      AddNewObject(new Engine::Label(
        "PREV PAGE","pirulen.ttf",24,
        x+btnW/2, btnY+btnH/2,
        0,0,0,255,
        0.5f,0.5f
      ));
    }

    // Back
    {
      int x = startX + 1*(btnW+margin);
      auto *b = new Engine::ImageButton(
        "stage-select/dirt.png","stage-select/floor.png",
        x, btnY, btnW, btnH
      );
      b->SetOnClickCallback(std::bind(&Scoreboard::BackOnClick, this));
      AddNewControlObject(b);
      AddNewObject(new Engine::Label(
        "BACK","pirulen.ttf",24,
        x+btnW/2, btnY+btnH/2,
        0,0,0,255,
        0.5f,0.5f
      ));
    }

    // Next Page
    {
      int x = startX + 2*(btnW+margin);
      auto *b = new Engine::ImageButton(
        "stage-select/dirt.png","stage-select/floor.png",
        x, btnY, btnW, btnH
      );
      b->SetOnClickCallback(std::bind(&Scoreboard::NextPageOnClick, this));
      AddNewControlObject(b);
      AddNewObject(new Engine::Label(
        "NEXT PAGE","pirulen.ttf",24,
        x+btnW/2, btnY+btnH/2,
        0,0,0,255,
        0.5f,0.5f
      ));
    }
}

void Scoreboard::Terminate() {
    IScene::Terminate();
}

void Scoreboard::PrevPageOnClick() {
    if (currentPage > 0) {
        --currentPage;
        // Re-enter the same scene so the engine will
        // clean up and re-initialize it for us, safely.
        Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
    }
}

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
