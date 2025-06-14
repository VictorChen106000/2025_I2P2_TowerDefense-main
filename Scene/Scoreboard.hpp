#ifndef SCOREBOARD_HPP
#define SCOREBOARD_HPP

#include <vector>
#include <string>
#include "Engine/IScene.hpp"
#include <allegro5/allegro.h>        // core Allegro types & drawing routines
#include <allegro5/allegro_image.h>  // al_load_bitmap, image‐addon init

class Scoreboard : public Engine::IScene {
public:
    Scoreboard() = default;
    void Initialize() override;
    void Terminate() override;

private:
    //std::vector<std::pair<std::string,int>> scores;
    struct ScoreEntry {
        std::string name;
        int         pts;
        std::string timestamp;
    };
    std::vector<ScoreEntry> scores;
    int currentPage = 0;      // ← which page we’re on
    int itemsPerPage = 0;     // ← how many items fit per page

    void PrevPageOnClick();
    void NextPageOnClick();
    void BackOnClick();
    void Draw() const override;
    ALLEGRO_BITMAP* bgBitmap = nullptr;
};

#endif // SCOREBOARDSCENE_HPP :contentReference[oaicite:0]{index=0}:contentReference[oaicite:1]{index=1}
