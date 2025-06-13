#ifndef PLAYSCENE_HPP
#define PLAYSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <list>
#include <memory>
#include <utility>
#include <vector>

#include <allegro5/allegro_primitives.h>  
#include <allegro5/color.h>

#include "UI/Component/Slider.hpp"
#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"
#include "UI/Component/ImageButton.hpp"



class Turret;
namespace Engine {
    class Group;
    class Image;
    class Label;
    class Sprite;
}   // namespace Engine

struct PanelRect : public Engine::IObject {
  float x, y, w, h;
  ALLEGRO_COLOR color;
  PanelRect(float _x, float _y, float _w, float _h, ALLEGRO_COLOR c)
    : x(_x), y(_y), w(_w), h(_h), color(c) {}
  void Draw() const override {
    al_draw_filled_rectangle(x, y, x + w, y + h, color);
  }
};

class PlayScene final : public Engine::IScene {
private:
    enum TileType {
      TILE_DIRT           = 0,  // '0' or ' ' (space)
      TILE_WHITE_FLOOR    = 1,  // '1'
      TILE_BLUE_FLOOR,         // 'D' / 'd'

      // Big corners
      TILE_CORNER_BOT_LEFT,    // 'C'
      TILE_CORNER_TOP_LEFT,    // '3'
      TILE_CORNER_TOP_RIGHT,   // '4'
      TILE_CORNER1,            // '5'
      TILE_CORNER2,            // '6'
      TILE_CORNER_4,
      TILE_CORNER_3,

      // Small corners (add these 4)
      TILE_CORNER_SMALL_1,     // e.g. 's' → corner-small-1.png
      TILE_CORNER_SMALL_2,     // 't' → corner-small-2.png
      TILE_CORNER_SMALL_3,     // 'u' → corner-small-3.png
      TILE_CORNER_SMALL_4,     // 'v' → corner-small-4.png

      // Platforms & special tiles
      TILE_PLATFORM,           // '7'
      TILE_TILE011,   
      
      TILE_S,// '8'

      // Walls
      TILE_WALL1,              // '#' or '9'
      TILE_WALL2,              // 'A' / 'a'
      TILE_WALL3,              // 'B' / 'b'

      TILE_OCCUPIED            // your existing occupied flag
    };
    std::vector<Engine::Point> spawnPoints;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> deathBGMInstance;
    float  elapsedTime = 0.0f;
    bool shovelMode = false;
    Engine::Sprite* shovelPreview = nullptr;
    bool isPaused = false;
    int savedSpeedMult = 1;
    Engine::ImageButton* quitBtn = nullptr;
    Engine::Label* quitLabel = nullptr;
    Slider* sliderBGM = nullptr;
    Slider* sliderSFX = nullptr;
    Engine::Label* labelBGM = nullptr;
    Engine::Label* labelSFX = nullptr;
    Engine::IObject* pausePanel = nullptr;
    float lastGroundSpawnTime = 0.0f;
    const float minGroundGap = 0.8f;
    void ShowPauseMenu();
    void HidePauseMenu();
    void BGMSlideOnValueChanged(float value);
    void SFXSlideOnValueChanged(float value);
    void SpawnEnemyOfType(int type, float extraTicks);
    //++++

    bool   isAiming = false;
    Turret* aimingTurret = nullptr;

protected:
    int lives;
    int money;
    int SpeedMult;
    int killCount;
    //++
    Engine::Label* UICoins;
    Engine::Label* UIrandom;
    
    


public:
    static bool DebugMode;
    static const std::vector<Engine::Point> directions;
    static const int MapWidth, MapHeight;
    static const int BlockSize;
    static const float DangerTime;
    static std::vector<Engine::Point> SpawnGridPoints;
    static std::vector<Engine::Point> EndGridPoints;

    static const std::vector<int> code;
    int MapId;
    float ticks;
    float deathCountDown;
    float GetElapsedTime() const { return elapsedTime; }
    float nextAdaptiveWait = 0.0f;
    // Map tiles.
    Group *TileMapGroup;
    Group *GroundEffectGroup;
    Group *DebugIndicatorGroup;
    Group *BulletGroup;
    Group *TowerGroup;
    Group *EnemyGroup;
    Group *EffectGroup;
    Group *UIGroup;
    Engine::Label *UIMoney;
    Engine::Label *UILives;
    Engine::Image *imgTarget;
    Engine::Sprite *dangerIndicator;
    Turret *preview;
    Engine::ImageButton* pauseBtn;
    std::vector<std::vector<TileType>> mapState;
    std::vector<std::vector<int>> mapDistance;
    std::list<std::pair<int, float>> enemyWaveData;
    std::list<int> keyStrokes;
    static Engine::Point GetClientSize();
    explicit PlayScene() = default;
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void Draw() const override;
    void OnMouseDown(int button, int mx, int my) override;
    void OnMouseMove(int mx, int my) override;
    void OnMouseUp(int button, int mx, int my) override;
    void OnKeyDown(int keyCode) override;
    void Hit();
    int GetMoney() const;
    void EarnMoney(int money);
    void ReadMap();
    void ReadEnemyWave();
    void ConstructUI();
    void UIBtnClicked(int id);
    bool CheckSpaceValid(int x, int y);
    std::vector<std::vector<int>> CalculateBFSDistance();
    float CalculatePlayerPower();
    std::pair<int, float> GenerateAdaptiveEnemy();
    void AddKill() {++killCount;}
    int GetKillCount() const {return killCount;}
    // void ModifyReadMapTiles();

    //earcoin
    void EarnCoin(int c);

    int coins;
    int soldierkillcount;

    static constexpr int KILLS_PER_COIN = 3;

    // UI elements:
    PanelRect*      killBarBg     = nullptr;
    PanelRect*      killBarFill   = nullptr;
    Engine::Label*  killBarLabel  = nullptr;

    // helper to refresh the bar
    void UpdateKillBar();



    Engine::Image* UICoinIcon   = nullptr;
    Engine::Label* UICoinCount  = nullptr;
};
#endif   // PLAYSCENE_HPP
