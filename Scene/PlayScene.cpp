#include <algorithm>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <cmath>
#include <fstream>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <cstdlib>

#include "Enemy/Enemy.hpp"
#include "Enemy/SoldierEnemy.hpp"
#include "Enemy/TankEnemy.hpp"
#include "Enemy/PlaneEnemy.hpp"
#include "Enemy/BigTankEnemy.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "Turret/LaserTurret.hpp"
#include "Turret/MachineGunTurret.hpp"
#include "Turret/RocketTurret.hpp"
#include "Turret/TurretButton.hpp"
#include "Turret/BowTurret.hpp"
#include "Shovel/ShovelButton.hpp"
#include "Turret/BallistaTurret.hpp"
#include "UI/Animation/DirtyEffect.hpp"
#include "UI/Animation/Plane.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"
#include "Turret/BombTurret.hpp"

// TODO HACKATHON-4 (1/3): Trace how the game handles keyboard input.
// TODO HACKATHON-4 (2/3): Find the cheat code sequence in this file.
// TODO HACKATHON-4 (3/3): When the cheat code is entered, a plane should be spawned and added to the scene.
// TODO HACKATHON-5 (1/4): There's a bug in this file, which crashes the game when you win. Try to find it.
// TODO HACKATHON-5 (2/4): The "LIFE" label are not updated when you lose a life. Try to fix it.

bool PlayScene::IsWalkable(TileType t)const{
    return t == TILE_WHITE_FLOOR;
}

bool PlayScene::HasTurretAt(int row, int col) const {
    // scan your TowerGroup for any turret whose grid‐coords match
    for (auto obj : TowerGroup->GetObjects()) {
        auto turret = dynamic_cast<Turret*>(obj);
        if (!turret) continue;
        int tx = int(turret->Position.x) / BlockSize;
        int ty = int(turret->Position.y) / BlockSize;
        if (ty == row && tx == col)
            return true;
    }
    return false;
}

bool PlayScene::CanPlaceTurretHere(int row, int col) const {
    // (If you switched to dynamic width/height, replace MapWidth/MapHeight
    // with your locals.  Otherwise this guards the bounds.)
    if (row < 0 || row >= MapHeight || col < 0 || col >= MapWidth)
        return false;

    return mapState[row][col] == TILE_WHITE_FLOOR
        && !HasTurretAt(row, col);  
    // —or test mapState[row][col] != TILE_OCCUPIED if that’s your flag.
}

bool PlayScene::DebugMode = false;
const std::vector<Engine::Point> PlayScene::directions = { Engine::Point(-1, 0), Engine::Point(0, -1), Engine::Point(1, 0), Engine::Point(0, 1) };
const int PlayScene::MapWidth = 20, PlayScene::MapHeight = 13;
const int PlayScene::BlockSize = 64;
const float PlayScene::DangerTime = 7.61;
const Engine::Point PlayScene::SpawnGridPoint = Engine::Point(-1, 0);
const Engine::Point PlayScene::EndGridPoint = Engine::Point(MapWidth, MapHeight - 1);
const std::vector<int> PlayScene::code = {
    ALLEGRO_KEY_LEFT, ALLEGRO_KEY_UP, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_DOWN
    // ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN, ALLEGRO_KEY_DOWN,
    // ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT,
    // ALLEGRO_KEY_B, ALLEGRO_KEY_A, ALLEGRO_KEY_ENTER
};
Engine::Point PlayScene::GetClientSize() {
    return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
}
void PlayScene::Initialize() {
    mapState.clear();
    keyStrokes.clear();
    ticks = 0;
    deathCountDown = -1;
    coins =0; // tambahan
    soldierkillcount=0; // tambahan 
    lives = 10;
    money = 2000;
    killCount = 0;
    isPaused = false;
    SpeedMult = 1;
    savedSpeedMult = 1;
    pausePanel   = nullptr;
    quitBtn      = nullptr;
    quitLabel    = nullptr;
    sliderBGM    = nullptr;
    sliderSFX    = nullptr;
    labelBGM     = nullptr;
    labelSFX     = nullptr;
    elapsedTime = 0.0f;
    nextAdaptiveWait = 1.0f;
    lastGroundSpawnTime = 0.0f;
    // Add groups from bottom to top.
    AddNewObject(TileMapGroup = new Group());
    AddNewObject(GroundEffectGroup = new Group());
    AddNewObject(DebugIndicatorGroup = new Group());
    AddNewObject(TowerGroup = new Group());
    AddNewObject(EnemyGroup = new Group());
    AddNewObject(BulletGroup = new Group());
    AddNewObject(EffectGroup = new Group());
    // Should support buttons.
    AddNewControlObject(UIGroup = new Group());
    ReadMap();
    ReadEnemyWave();
    mapDistance = CalculateBFSDistance();
    ConstructUI();
    imgTarget = new Engine::Image("play/target.png", 0, 0);
    imgTarget->Visible = false;
    preview = nullptr;
    UIGroup->AddNewObject(imgTarget);
    // Preload Lose Scene
    deathBGMInstance = Engine::Resources::GetInstance().GetSampleInstance("astronomia.ogg");
    Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    // Start BGM.
    bgmInstance = AudioHelper::PlaySample("play.ogg", true, AudioHelper::BGMVolume);
}
void PlayScene::Terminate() {
    Engine::LOG(Engine::INFO)<<"Terminating play scene";
    AudioHelper::StopSample(bgmInstance);
    AudioHelper::StopSample(deathBGMInstance);
    deathBGMInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void PlayScene::Update(float deltaTime) {
    elapsedTime += deltaTime;
    // If we use deltaTime directly, then we might have Bullet-through-paper problem.
    // Reference: Bullet-Through-Paper
    if (SpeedMult == 0)
        deathCountDown = -1;
    else if (deathCountDown != -1)
        SpeedMult = 1;

    // Calculate danger zone.
    std::vector<float> reachEndTimes;
    for (auto &it : EnemyGroup->GetObjects()) {
        reachEndTimes.push_back(dynamic_cast<Enemy *>(it)->reachEndTime);
    }
    // Can use Heap / Priority-Queue instead. But since we won't have too many enemies, sorting is fast enough.
    std::sort(reachEndTimes.begin(), reachEndTimes.end());
    float newDeathCountDown = -1;
    int danger = lives;
    for (auto &it : reachEndTimes) {
        if (it <= DangerTime) {
            danger--;
            if (danger <= 0) {
                // Death Countdown
                float pos = DangerTime - it;
                if (it > deathCountDown) {
                    // Restart Death Count Down BGM.
                    AudioHelper::StopSample(deathBGMInstance);
                    if (SpeedMult != 0)
                        deathBGMInstance = AudioHelper::PlaySample("astronomia.ogg", false, AudioHelper::BGMVolume, pos);
                }
                float alpha = pos / DangerTime;
                alpha = std::max(0, std::min(255, static_cast<int>(alpha * alpha * 255)));
                dangerIndicator->Tint = al_map_rgba(255, 255, 255, alpha);
                newDeathCountDown = it;
                break;
            }
        }
    }

    deathCountDown = newDeathCountDown;
    if (SpeedMult == 0)
        AudioHelper::StopSample(deathBGMInstance);
    if (deathCountDown == -1 && lives > 0) {
        AudioHelper::StopSample(deathBGMInstance);
        dangerIndicator->Tint.a = 0;
    }
    if (SpeedMult == 0)
        deathCountDown = -1;
    for (int i = 0; i < SpeedMult; i++) {
        IScene::Update(deltaTime);
        if (shovelMode && shovelPreview) {
            auto mpos = Engine::GameEngine::GetInstance().GetMousePosition();
            shovelPreview->Position = mpos;
            shovelPreview->Update(deltaTime);
        }
        
        // Check if we should create new enemy.
        ticks += deltaTime;
        if (!enemyWaveData.empty()) {
            auto current = enemyWaveData.front();
            if (ticks >= current.second) {
                ticks -= current.second;
                enemyWaveData.pop_front();
                SpawnEnemyOfType(current.first, ticks);
            }
        } else {
            if (ticks >= nextAdaptiveWait) {
                ticks -= nextAdaptiveWait;

                // === BEGIN DEBUG PRINT ===
                //  1) Recompute power/time/D/wait in local variables so we can see them:
                int dbgCount   = static_cast<int>(TowerGroup->GetObjects().size());       // number of towers
                float dbgDPS   = CalculatePlayerPower();                                  // sum DPS
                const float α  = 0.5f;             // tuner for tower count
                const float β  = 1.0f;             // tuner for total DPS
                const float γ  = 0.2f;             // tuner for time
                float dbgD     = α * float(dbgCount) + β * dbgDPS + γ * elapsedTime;      // new difficulty score
                auto [dbgType, dbgWait] = GenerateAdaptiveEnemy();

                printf(
                    "[ADAPTIVE DEBUG] count=%d  totalDPS=%.2f  time=%.1f  D=%.2f  → type=%d  wait=%.2f  alive=%zu\n",
                    dbgCount,
                    dbgDPS,
                    elapsedTime,
                    dbgD,
                    dbgType,
                    dbgWait,
                    EnemyGroup->GetObjects().size()
                );
                // === END DEBUG PRINT ===
                
                bool isGround = (dbgType == 1 || dbgType == 3 || dbgType == 4);
                float timeSinceLast = elapsedTime - lastGroundSpawnTime;
                if (isGround && timeSinceLast < minGroundGap) {
                    float leftover = (minGroundGap - timeSinceLast);
                    nextAdaptiveWait = dbgWait + leftover;
                } else {
                    if (isGround) lastGroundSpawnTime = elapsedTime;
                    nextAdaptiveWait = dbgWait;
                    SpawnEnemyOfType(dbgType, ticks);
                }
            }
        }

        if (enemyWaveData.empty() && EnemyGroup->GetObjects().empty()) {
            Engine::GameEngine::GetInstance().ChangeScene("win");
        }
        if (preview) {
            preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
            // To keep responding when paused.
            preview->Update(deltaTime);
        }
    }
}
void PlayScene::Draw() const {
    IScene::Draw();
    if (DebugMode) {
        // Draw reverse BFS distance on all reachable blocks.
        for (int i = 0; i < MapHeight; i++) {
            for (int j = 0; j < MapWidth; j++) {
                if (mapDistance[i][j] != -1) {
                    // Not elegant nor efficient, but it's quite enough for debugging.
                    Engine::Label label(std::to_string(mapDistance[i][j]), "pirulen.ttf", 32, (j + 0.5) * BlockSize, (i + 0.5) * BlockSize);
                    label.Anchor = Engine::Point(0.5, 0.5);
                    label.Draw();
                }
            }
        }
    }
}
void PlayScene::OnMouseDown(int button, int mx, int my) {
    if (isPaused) {
        // still forward to IScene so your sliders & quitBtn get the click…
        IScene::OnMouseDown(button, mx, my);
        return;   // …but skip all the rest (turret/shovel logic)
    }
    // Calculate whether the click is on the map area:
    const int mapW = MapWidth * BlockSize;
    const int mapH = MapHeight * BlockSize;
    bool clickOnMap = mx >= 0 && mx < mapW && my >= 0 && my < mapH;
    //aaaaaaaaaaaaa
    if ((button & 1) && !isPaused) {
        for (auto obj : TowerGroup->GetObjects()) {
            auto bt = dynamic_cast<BowTurret*>(obj);
            if (!bt) continue;
            float dx = mx - bt->Position.x;
            float dy = my - bt->Position.y;
            const float pickRadius = 32; 
            if (dx*dx + dy*dy < pickRadius*pickRadius) {
                isAiming = true;
                aimingTurret = bt;
                return;   // consume the click
            }
        }
    }
    //aaaaaaaaaaaaaa
    // If it’s a left‐click outside the map *or* any right‐click, cancel both turret preview and shovel
    if (((button & 1) && !clickOnMap) || (button & 2)) {
        // 1) Cancel turret preview
        if (preview) {
            UIGroup->RemoveObject(preview->GetObjectIterator());
            preview = nullptr;
        }
        // 2) Cancel shovel mode
        if (shovelMode) {
            if (shovelPreview) {
                UIGroup->RemoveObject(shovelPreview->GetObjectIterator());
                shovelPreview = nullptr;
            }
            shovelMode       = false;
            imgTarget->Visible = false;  // hide the grid highlight
        }
    }
    // Always forward to base for button callbacks (so your TurretButton / ShovelButton still work)
    IScene::OnMouseDown(button, mx, my);
}
void PlayScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
    //aaaaaaa
    if (isAiming && aimingTurret) {
        float dx = mx - aimingTurret->Position.x;
        float dy = my - aimingTurret->Position.y;
        // +PI/2 if your sprite is “up” at 0 radians
        aimingTurret->Rotation = std::atan2(dy, dx) + ALLEGRO_PI/2;
        return;
    }
    //aaaaaaaaa
    if (!preview || x < 0 || x >= MapWidth || y < 0 || y >= MapHeight) {
        imgTarget->Visible = false;
        return;
    }
    imgTarget->Visible = true;
    imgTarget->Position.x = x * BlockSize;
    imgTarget->Position.y = y * BlockSize;
}
void PlayScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
    if (isPaused) return; 
    const int mapPixelWidth  = MapWidth  * BlockSize;  // 20 * 64 = 1280
    const int mapPixelHeight = MapHeight * BlockSize;  // 13 * 64 =  832
    if (isAiming) {
        isAiming = false;
        aimingTurret = nullptr;
        return;  // prevent passing through to “place turret” logic
    }

    // If the right mouse button is clicked (for upgrade)
    if (button & 2) {  // Check if it's a right-click
        const int x = mx / BlockSize;
        const int y = my / BlockSize;

        // Check if the clicked tile has a turret (i.e., is occupied)
        if (mapState[y][x] == TILE_OCCUPIED) {
            // Iterate over the TowerGroup to find the turret clicked
            for (auto obj : TowerGroup->GetObjects()) {
                Turret* turret = dynamic_cast<Turret*>(obj);
                if (!turret) continue;

                // Check if the mouse click was on the turret
                int tx = int(turret->Position.x) / BlockSize;
                int ty = int(turret->Position.y) / BlockSize;

                if (tx == x && ty == y) {
                    // If the turret can be upgraded and the player has enough money
                    if (turret->canupdrage()) {
                        int upgradeCost = turret->getupdragecost();
                        if (money >= upgradeCost) {
                            // Subtract money and upgrade the turret
                            EarnMoney(-upgradeCost);
                            turret->updrage();
                            AudioHelper::PlayAudio("explosion.wav");  // Play upgrade sound

                            // Optionally: show a visual upgrade effect or change sprite, etc.
                        } else {
                            // If the player doesn't have enough money
                            AudioHelper::PlayAudio("explosion.wav");  // Play sound or show a message
                        }
                    } else {
                        // Optionally: Play a sound or show a message if turret is already at max level
                        AudioHelper::PlayAudio("explosion.wav");
                    }
                    break; // Exit loop after upgrading the turret
                }
            }
        }
    }


    if (shovelMode && (button & 1) && mx >= 0 && mx <  mapPixelWidth && my >= 0 && my <  mapPixelHeight)
    {
        // 1) grid coords
        int gx = mx / BlockSize;
        int gy = my / BlockSize;

        // 2) find the turret in that cell
        for (auto obj : TowerGroup->GetObjects()) {
            auto turret = dynamic_cast<Turret*>(obj);
            if (!turret) continue;
            int tx = int(turret->Position.x) / BlockSize;
            int ty = int(turret->Position.y) / BlockSize;
            if (tx == gx && ty == gy) {
                // 3a) refund 85%
                int refund = int(turret->GetPrice() * 0.85f);
                EarnMoney(refund);

                // 3b) remove turret & clear tile
                turret->GetObjectIterator()->first = false;
                TowerGroup->RemoveObject(turret->GetObjectIterator());
                mapState[gy][gx] = TILE_DIRT;
                break;
            }
        }

        // 4) tear down the preview
        UIGroup->RemoveObject(shovelPreview->GetObjectIterator());
        shovelPreview    = nullptr;
        shovelMode       = false;
        imgTarget->Visible = false;
        return;
    }
    if (!imgTarget->Visible)
        return;
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
    if (button & 1) {
        if (mapState[y][x] != TILE_OCCUPIED) {
            if (!preview)
                return;
            bool isBomb = dynamic_cast<BombTurret *>(preview) != nullptr;
            if (!CanPlaceTurretHere(y, x)) {
                // all-in-one invalid effect
                GroundEffectGroup->AddNewObject(
                  new DirtyEffect("play/target-invalid.png", 1,
                                  x*BlockSize+BlockSize/2,
                                  y*BlockSize+BlockSize/2)
                );
                return;
            }
            
            // Purchase
            EarnMoney(-preview->GetPrice());
            // Remove preview
            preview->GetObjectIterator()->first = false;
            UIGroup->RemoveObject(preview->GetObjectIterator());
            // Construct real turret
            preview->Position.x = x * BlockSize + BlockSize / 2;
            preview->Position.y = y * BlockSize + BlockSize / 2;
            preview->Enabled = true;
            preview->Preview = false;
            preview->Tint = al_map_rgba(255, 255, 255, 255);
            TowerGroup->AddNewObject(preview);
            // To keep responding when paused
            preview->Update(0);
            // For non‐bomb turrets, mark the grid occupied
            if (!isBomb) {
                mapState[y][x] = TILE_OCCUPIED;
            }
            // Reset preview pointer
            preview = nullptr;
            // Update target indicator
            OnMouseMove(mx, my);
        }
    }
}
void PlayScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    if (keyCode == ALLEGRO_KEY_TAB) {
        DebugMode = !DebugMode;
    } else {
        keyStrokes.push_back(keyCode);

        if (keyStrokes.size() > code.size()) keyStrokes.pop_front();

        if (keyStrokes.size() == code.size() && std::equal(code.begin(), code.end(), keyStrokes.begin())) {
            // Cheat activated: Spawn a Rocket and add 10,000 money
            // const Engine::Point SpawnCoordinate = Engine::Point(SpawnGridPoint.x * BlockSize + BlockSize / 2, SpawnGridPoint.y * BlockSize + BlockSize / 2);
            // PlaneEnemy* cheatPlane = new PlaneEnemy(SpawnCoordinate.x, SpawnCoordinate.y);
            // EnemyGroup->AddNewObject(cheatPlane);
            // cheatPlane->UpdatePath(mapDistance);
            EffectGroup->AddNewObject(new Plane());
            EarnMoney(10000);
        }        
    }
    if (keyCode == ALLEGRO_KEY_Q && !isPaused) {
        // Hotkey for MachineGunTurret.
        UIBtnClicked(0);
    } else if (keyCode == ALLEGRO_KEY_W && !isPaused) {
        // Hotkey for LaserTurret.
        UIBtnClicked(1);
    } else if (keyCode == ALLEGRO_KEY_E && !isPaused) {
        UIBtnClicked(2);  // hotkey E for rocket
    }
    else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
        // Hotkey for Speed up.
        int newSpeed = keyCode - ALLEGRO_KEY_0;
        if (isPaused) savedSpeedMult = newSpeed;
        else SpeedMult = newSpeed;
    }
}
void PlayScene::Hit() {
    lives--;
    UILives->Text = std::string("Life ") + std::to_string(lives);
    if (lives <= 0) {
        Engine::GameEngine::GetInstance().ChangeScene("lose");
    }
}
int PlayScene::GetMoney() const {
    return money;
}
void PlayScene::EarnCoin(int c) {
  coins += c;
  UICoins->Text = std::to_string(coins);
}
void PlayScene::EarnMoney(int money) {
    this->money += money;
    UIMoney->Text = std::string("$") + std::to_string(this->money);
}
void PlayScene::UpdateKillBar() {
    // make sure count never exceeds the goal
    int count = std::min(soldierkillcount, KILLS_PER_COIN);
    float frac = float(count) / float(KILLS_PER_COIN);
    killBarFill->w = frac * killBarBg->w;
    killBarLabel->Text = std::to_string(count) + "/" + std::to_string(KILLS_PER_COIN);
}



void PlayScene::ReadMap() {
    std::string filename = std::string("Resource/map") + std::to_string(MapId) + ".txt";
    // Read map file.
    char c;
    std::vector<int> mapData;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case '0': mapData.push_back(0); break;  // dirt
            case '1': mapData.push_back(1); break;  // white-floor
            // ─── Add these cases ───────────────────
            case '2': mapData.push_back(2);  break; // blue-floor
            case '3': mapData.push_back(3);  break; // corner1
            case '4': mapData.push_back(4);  break; // corner2
            case '5': mapData.push_back(5);  break; // corner3
            case '6': mapData.push_back(6);  break; // corner4
            case '7': mapData.push_back(7);  break; // platform
            case '8': mapData.push_back(8);  break; // wall1
            case '9': mapData.push_back(9);  break; // wall2
            case 'A': mapData.push_back(10); break; // wall3 (use 'A' or another symbol)
            case '\n': case '\r':
                if ((int)mapData.size() % MapWidth != 0)
                    throw std::ios_base::failure("Map data is corrupted.");
                break;
            default:
                throw std::ios_base::failure("Unknown tile code in map.");
        }
    }
    fin.close();
    // Validate map data.
    if (static_cast<int>(mapData.size()) != MapWidth * MapHeight)
        throw std::ios_base::failure("Map data is corrupted.");
    // Store map in 2d array.
    mapState = std::vector<std::vector<TileType>>(MapHeight, std::vector<TileType>(MapWidth));
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++) {
            const int num = mapData[i * MapWidth + j];
            int code = mapData[i * MapWidth + j];
            TileType t;
            std::string path;
            switch (code) {
                case 0:  t = TILE_DIRT;        path = "play/dirt.png";         break;
                case 1:  t = TILE_WHITE_FLOOR; path = "tile/white-floor.png";  break;
                case 2:  t = TILE_BLUE_FLOOR;  path = "tile/blue-floor.png";   break;
                case 3:  t = TILE_CORNER1;     path = "tile/corner1.png";      break;
                case 4:  t = TILE_CORNER2;     path = "tile/corner2.png";      break;
                case 5:  t = TILE_CORNER3;     path = "tile/corner3.png";      break;
                case 6:  t = TILE_CORNER4;     path = "tile/corner4.png";      break;
                case 7:  t = TILE_PLATFORM;    path = "tile/platform.png";     break;
                case 8:  t = TILE_WALL1;       path = "tile/wall1.png";        break;
                case 9:  t = TILE_WALL2;       path = "tile/wall2.png";        break;
                case 10: t = TILE_WALL3;       path = "tile/wall3.png";        break;
                default: throw std::ios_base::failure("Invalid tile code");
            }
            mapState[i][j] = t;
            TileMapGroup->AddNewObject(
                new Engine::Image(path, j * BlockSize, i * BlockSize, BlockSize, BlockSize)
            );

        }
    }
}
void PlayScene::ReadEnemyWave() {
    std::string filename = std::string("Resource/enemy") + std::to_string(MapId) + ".txt";
    // Read enemy file.
    float type, wait, repeat;
    enemyWaveData.clear();
    std::ifstream fin(filename);
    while (fin >> type && fin >> wait && fin >> repeat) {
        for (int i = 0; i < repeat; i++)
            enemyWaveData.emplace_back(type, wait);
    }
    fin.close();
}
void PlayScene::ConstructUI() {
    // Background
    UIGroup->AddNewObject(new Engine::Image("play/sand.png", 1280, 0, 320, 832));
    // Text
    UIGroup->AddNewObject(new Engine::Label(std::string("Stage ") + std::to_string(MapId), "pirulen.ttf", 32, 1294, 0));
    UIGroup->AddNewObject(UIMoney = new Engine::Label(std::string("$") + std::to_string(money), "pirulen.ttf", 24, 1294, 48));
    UIGroup->AddNewObject(UILives = new Engine::Label(std::string("Life ") + std::to_string(lives), "pirulen.ttf", 24, 1294, 88));
    //coin
    UIGroup->AddNewObject(UICoinIcon = new Engine::Image("play/goldcoin3.png",1294, 350,48, 48));
    UIGroup->AddNewObject(UICoins = new Engine::Label(std::to_string(coins),"pirulen.ttf",24,1355,360));
    UIGroup->AddNewObject(UICoinCount = new Engine::Label("Kill 3 Enemy Soldiers","pirulen.ttf",16,1294,480));
    // Pause Button (toggles SpeedMult between 0 and 1)
    pauseBtn = new Engine::ImageButton(
        "play/pause.png",       // out
        "play/pause.png", // in
        1560, 8, 32, 32, 0, 0
    );
    pauseBtn->SetOnClickCallback([this](){
        // flip state
        isPaused = !isPaused;
        // swap the two icons
        if (isPaused) {
            savedSpeedMult = SpeedMult;
            SpeedMult      = 0;
            // show ▶️ (play)
            pauseBtn->SetImage("play/play.png", "play/play.png");
            ShowPauseMenu();
        } else {
            SpeedMult      = savedSpeedMult;
            // show ⏸️ (pause)
            pauseBtn->SetImage("play/pause.png", "play/pause.png");
            HidePauseMenu();
        }
    });
    UIGroup->AddNewControlObject(pauseBtn);

    TurretButton *btn;
    // Button 1
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1294, 136, 0, 0, 0, 0),
                           Engine::Sprite("play/roketred1.png", 1305, 122, 0, 0, 0, 0), 1294, 136, MachineGunTurret::Price);
    // Reference: Class Member Function Pointer and std::bind.
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 0));
    UIGroup->AddNewControlObject(btn);
    // Button 2
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1370, 136, 0, 0, 0, 0),
                           Engine::Sprite("play/tankblue1.png", 1382, 121, 0, 0, 0, 0), 1370, 136, LaserTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 1));
    UIGroup->AddNewControlObject(btn);
    
    btn = new TurretButton(
        "play/blank1.png", "play/shovel-base.png",
        Engine::Sprite("play/bomb.png", 1520, 240, 0, 0, 0, 0),
        Engine::Sprite("play/bomb.png", 1520, 240, 0, 0, 0, 0),
        1520, 240, BombTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 4));
    UIGroup->AddNewControlObject(btn);

    btn = new TurretButton(
        "play/floor.png", "play/dirt.png",
        Engine::Sprite("play/tower-base.png", 1520, 136, 0, 0, 0, 0),
        Engine::Sprite("play/tankrrr1profile.png", 1530, 134, 0, 0, 0, 0),
        1520, 136, BombTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 5));
    UIGroup->AddNewControlObject(btn);

     btn = new TurretButton(
        "play/floor.png", "play/dirt.png",
        Engine::Sprite("play/tower-base.png", 1446, 240, 0, 0, 0, 0),
        Engine::Sprite("play/Ballista2.png", 1446, 240, 0, 0, 0, 0),
        1446, 240, BombTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 6));
    UIGroup->AddNewControlObject(btn);

    // ShovelButton
    auto shovelBtn = new ShovelButton(1294, 210);
    shovelBtn->SetOnClickCallback([this](){
        if (isPaused) return;
        // toggle shovel mode on each *click release*
        if (shovelMode) {
            UIGroup->RemoveObject(shovelPreview->GetObjectIterator());
            shovelMode = false;
            shovelPreview = nullptr;
        } else {
            shovelMode = true;
            shovelPreview = new Engine::Sprite("play/shovel.png", 0, 0);
            shovelPreview->Tint = al_map_rgba(255,255,255,200);
            UIGroup->AddNewObject(shovelPreview);
        }
    });
    UIGroup->AddNewControlObject(shovelBtn);

    // RocketTurret Button
    btn = new TurretButton(
    "play/floor.png", "play/dirt.png",
    Engine::Sprite("play/tower-base.png", 1446, 136, 0, 0, 0, 0),
    Engine::Sprite("play/roketcrop1.png",     1453, 138, 0, 0, 0, 0),
    1446, 136,
    RocketTurret::Price
    );
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 2));
    UIGroup->AddNewControlObject(btn);

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int shift = 135 + 25;
    dangerIndicator = new Engine::Sprite("play/benjamin.png", w - shift, h - shift);
    dangerIndicator->Tint.a = 0;
    UIGroup->AddNewObject(dangerIndicator);

    //BARRRRRRRRRRRRRRRRRRRRRR
    const int BAR_W = 200, BAR_H = 20;
    const int BAR_X = 1294, BAR_Y = 500;  // just below your coins label
    // background (grey)
    UIGroup->AddNewObject(killBarBg   = new PanelRect(BAR_X, BAR_Y, BAR_W, BAR_H, al_map_rgba(100,100,100,255)));
    // fill (green, start at 0 width)
    UIGroup->AddNewObject(killBarFill = new PanelRect(BAR_X, BAR_Y, 0,     BAR_H, al_map_rgba(0,200,0,255)));
    // text "0/3"
    UIGroup->AddNewObject(killBarLabel = new Engine::Label("0/3", "pirulen.ttf", 20, BAR_X + BAR_W/2, BAR_Y + BAR_H/2));
    killBarLabel->Anchor = Engine::Point(0.5f, 0.5f); 
}

void PlayScene::UIBtnClicked(int id) {
    if (isPaused) return;
    // 1) Determine if we can actually preview this turret
    Turret* newPreview = nullptr;
    if (id == 0 && money >= MachineGunTurret::Price) {
        newPreview = new MachineGunTurret(0, 0);
    }
    else if (id == 1 && money >= LaserTurret::Price) {
        newPreview = new LaserTurret(0, 0);
    }
    else if (id == 2 && money >= RocketTurret::Price) {
        newPreview = new RocketTurret(0,0);
    } else if (id == 4 && money >= BombTurret::Price) {
        newPreview = new BombTurret(0, 0);
    }else if (id == 5 && money >= BallistaTurret::Price) {
        newPreview = new BallistaTurret(0, 0);
    }else if (id == 6 && money >= BowTurret::Price) {
        newPreview = new BowTurret(0, 0);
    }else return;

    // 2) Remove any existing preview
    if (preview) {
        UIGroup->RemoveObject(preview->GetObjectIterator());
    }

    // 3) Install the new preview
    preview = newPreview;
    preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
    preview->Tint     = al_map_rgba(255, 255, 255, 200);
    preview->Enabled  = false;
    preview->Preview  = true;
    UIGroup->AddNewObject(preview);

    // Ensure the target‐tile indicator stays in sync
    OnMouseMove(
      Engine::GameEngine::GetInstance().GetMousePosition().x,
      Engine::GameEngine::GetInstance().GetMousePosition().y
    );
}


bool PlayScene::CheckSpaceValid(int x, int y) {
    if (x < 0 || x >= MapWidth || y < 0 || y >= MapHeight)
        return false;
    auto map00 = mapState[y][x];
    mapState[y][x] = TILE_OCCUPIED;
    std::vector<std::vector<int>> map = CalculateBFSDistance();
    mapState[y][x] = map00;
    if (map[0][0] == -1)
        return false;
    for (auto &it : EnemyGroup->GetObjects()) {
        Engine::Point pnt;
        pnt.x = floor(it->Position.x / BlockSize);
        pnt.y = floor(it->Position.y / BlockSize);
        if (pnt.x < 0) pnt.x = 0;
        if (pnt.x >= MapWidth) pnt.x = MapWidth - 1;
        if (pnt.y < 0) pnt.y = 0;
        if (pnt.y >= MapHeight) pnt.y = MapHeight - 1;
        if (map[pnt.y][pnt.x] == -1)
            return false;
    }
    // All enemy have path to exit.
    mapState[y][x] = TILE_OCCUPIED;
    mapDistance = map;
    for (auto &it : EnemyGroup->GetObjects())
        dynamic_cast<Enemy *>(it)->UpdatePath(mapDistance);
    return true;
}
std::vector<std::vector<int>> PlayScene::CalculateBFSDistance() {
    // Reverse BFS to find path.
    std::vector<std::vector<int>> map(MapHeight, std::vector<int>(std::vector<int>(MapWidth, -1)));
    std::queue<Engine::Point> que;
    // Push end point.
    // BFS from end point.
    if (mapState[MapHeight - 1][MapWidth - 1] != TILE_DIRT)
        return map;
    que.push(Engine::Point(MapWidth - 1, MapHeight - 1));
    map[MapHeight - 1][MapWidth - 1] = 0;
    while (!que.empty()) {
        Engine::Point p = que.front();
        que.pop();
        // TODO PROJECT-1 (1/1): Implement a BFS starting from the most right-bottom block in the map.
                    //   For each step you should assign the corresponding distance to the most right-bottom block.
                    //   mapState[y][x] is TILE_DIRT if it is empty.
        for (const auto& dir : directions) {
            int nx = p.x + dir.x;
            int ny = p.y + dir.y;
            if (nx < 0 || nx >= MapWidth || ny < 0 || ny >= MapHeight)
                continue;
                if (!IsWalkable(mapState[ny][nx]) || map[ny][nx] != -1)
                continue;
            map[ny][nx] = map[p.y][p.x] + 1;
            que.push(Engine::Point(nx, ny));
        }
    }
    return map;
}


void PlayScene::BGMSlideOnValueChanged(float value) {
    AudioHelper::BGMVolume = value;
    AudioHelper::ChangeSampleVolume(bgmInstance, value);
}

void PlayScene::SFXSlideOnValueChanged(float value) {
    AudioHelper::SFXVolume = value;
}

void PlayScene::ShowPauseMenu() {
    if (preview) {
        UIGroup->RemoveObject(preview->GetObjectIterator());
        preview = nullptr;
    }
    if (shovelMode) {
        shovelMode = false;
        if (shovelPreview) {
            UIGroup->RemoveObject(shovelPreview->GetObjectIterator());
            shovelPreview = nullptr;
        }
        imgTarget->Visible = false;
    }
    // center a box
    int W = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int H = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int FW = 400, FH = 250;
    int FX = (W - FW) / 2, FY = (H - FH) / 2;

    // a semi-transparent black backdrop
    pausePanel = new PanelRect(FX, FY, FW, FH, al_map_rgba(0, 0, 0, 200));
    UIGroup->AddNewObject(pausePanel);

    // BGM label + slider
    int labelY = FY + 30;
    labelBGM = new Engine::Label("BGM:", "pirulen.ttf", 24, FX + 20, labelY, 255,255,255,255);
    UIGroup->AddNewObject(labelBGM);
    sliderBGM = new Slider(FX + 130, labelY + 12, FW - 160, 4);
    sliderBGM->SetValue(AudioHelper::BGMVolume);
    sliderBGM->SetOnValueChangedCallback(
        std::bind(&PlayScene::BGMSlideOnValueChanged, this, std::placeholders::_1)
    );
    UIGroup->AddNewControlObject(sliderBGM);

    // SFX label + slider
    int labelY2 = FY + 80;
    labelSFX = new Engine::Label("SFX:", "pirulen.ttf", 24, FX + 20, labelY2, 255,255,255,255);
    UIGroup->AddNewObject(labelSFX);
    sliderSFX = new Slider(FX + 130, labelY2 + 12, FW - 160, 4);
    sliderSFX->SetValue(AudioHelper::SFXVolume);
    sliderSFX->SetOnValueChangedCallback(
        std::bind(&PlayScene::SFXSlideOnValueChanged, this, std::placeholders::_1)
    );
    UIGroup->AddNewControlObject(sliderSFX);

    // Quit‐to‐StageSelect button
    quitBtn = new Engine::ImageButton(
        "stage-select/dirt.png",  // up
        "stage-select/floor.png", // down
        FX + FW - 140,            // x
        FY + FH - 60,             // y
        120, 40                   // w, h
    );
    quitBtn->SetOnClickCallback([this](){
        SpeedMult = savedSpeedMult;
        isPaused = false;
        pauseBtn->SetImage("play/pause.png", "play/pause.png");
        HidePauseMenu();
        Engine::GameEngine::GetInstance().ChangeScene("stage-select");
    });
    UIGroup->AddNewControlObject(quitBtn);

    quitLabel = new Engine::Label(
        "Quit",              // text
        "pirulen.ttf",       // font
        24,                  // size
        FX + FW - 140 + 120/2, // center x (button x + w/2)
        FY + FH - 60  + 40/2  // center y (button y + h/2)
    );
    quitLabel->Anchor = Engine::Point(0.5f, 0.5f);  // pivot at its center
    UIGroup->AddNewObject(quitLabel);
}

void PlayScene::HidePauseMenu() {
    if (pausePanel) { UIGroup->RemoveObject(pausePanel->GetObjectIterator()); pausePanel = nullptr; }
    if (labelBGM) { UIGroup->RemoveObject(labelBGM->GetObjectIterator());   labelBGM   = nullptr; }
    if (sliderBGM) { UIGroup->RemoveObject(sliderBGM->GetObjectIterator());  sliderBGM  = nullptr; }
    if (labelSFX) { UIGroup->RemoveObject(labelSFX->GetObjectIterator());   labelSFX   = nullptr; }
    if (sliderSFX) { UIGroup->RemoveObject(sliderSFX->GetObjectIterator());  sliderSFX  = nullptr; }
    if (quitBtn) { UIGroup->RemoveObject(quitBtn->GetObjectIterator());    quitBtn    = nullptr; }
    if (quitLabel) { UIGroup->RemoveObject(quitLabel->GetObjectIterator()); quitLabel = nullptr;}
}

float PlayScene::CalculatePlayerPower() {
    float totalDPS = 0.0f;
    for (auto &obj : TowerGroup->GetObjects()) {
        Turret *t = dynamic_cast<Turret *>(obj);
        if (!t) continue;
        totalDPS += t->GetDPS();
    }
    return totalDPS;
}

std::pair<int, float> PlayScene::GenerateAdaptiveEnemy() {
    // 1) Count how many towers are placed:
    int towerCount = static_cast<int>(TowerGroup->GetObjects().size());

    // 2) Sum up all turret DPS:
    float totalDPS = CalculatePlayerPower();

    // 3) Combine (towerCount, totalDPS, elapsedTime) into a single difficulty score D:
    //
    //   – Let α tune “importance of tower‐count” (e.g. each tower → 0.5 difficulty points)
    //   – Let β tune “importance of total DPS”    (e.g. each 1 DPS → 1 difficulty point)
    //   – Let γ tune “importance of time”         (to slowly ramp even if no turrets)
    //
    const float α = 0.5f;       // each tower adds 0.5 difficulty
    const float β = 1.0f;       // each DPS point adds 1 difficulty
    const float γ = 0.2f;       // each second adds 0.2 difficulty
    float D = α * float(towerCount) + β * totalDPS + γ * elapsedTime;

    int   type;
    float wait;

    // ─── Phase 1: D < 5 → only Soldiers, spawn every 2.0 → 1.5 s ─────────────
    if (D < 5.0f) {
        type = 1; // Soldier
        wait = 2.0f - 0.1f * D;        // linearly 2.0 → 1.5 as D goes 0→5
        wait = std::clamp(wait, 1.5f, 2.0f);
    }
    // ─── Phase 2: 5 ≤ D < 12 → mix Soldiers & Tanks, spawn 1.5 → 1.0 s ─────────
    else if (D < 12.0f) {
        // As D climbs 5→12, tankChance goes 20%→60%
        int tankChance = static_cast<int>(20 + (D - 5.0f) * (40.0f / 7.0f));
        if ((rand() % 100) < tankChance) {
            type = 3; // Tank
        } else {
            type = 1; // Soldier
        }
        wait = 1.5f - 0.0714286f * (D - 5.0f);  // 1.5 → 1.0 as D goes 5→12
        wait = std::clamp(wait, 1.0f, 1.5f);
    }
    // ─── Phase 3: D ≥ 12 → Tanks/Planes/BigTanks, spawn 1.0 → 0.6 s ────────────
    else {
        static float lastBigTankTime = 0.0f;
        int r = rand() % 100;
        if (r < 40) {
            type = 3; // Tank (40%)
        }
        else if (r < 65) {
            type = 2; // Plane (25%)
        }
        else {
            // 35% chance for BigTank, but only once every 20 seconds
            if ((elapsedTime - lastBigTankTime) >= 20.0f) {
                type = 4; // BigTank
                lastBigTankTime = elapsedTime;
            } else {
                type = 3; // fallback to Tank
            }
        }
        float rawWait = 1.0f - 0.025f * (D - 12.0f);  // 1.0 → 0.6 as D goes 12→28
        wait = std::clamp(rawWait, 0.6f, 1.0f);
    }

    // ─── Prevent overcrowding: if ≥ 8 enemies alive, add +1s delay ────────────
    if ((int)EnemyGroup->GetObjects().size() >= 8) {
        wait += 1.0f;
    }

    return { type, wait };
}


void PlayScene::SpawnEnemyOfType(int type, float extraTicks) {
    const Engine::Point SpawnCoordinate =
        Engine::Point(SpawnGridPoint.x * BlockSize + BlockSize / 2,
                      SpawnGridPoint.y * BlockSize + BlockSize / 2);

    Enemy *enemy = nullptr;
    switch (type) {
        case 1:
            EnemyGroup->AddNewObject(enemy = new SoldierEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
            break;
        case 2:
            EnemyGroup->AddNewObject(enemy = new PlaneEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
            break;
        case 3:
            EnemyGroup->AddNewObject(enemy = new TankEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
            break;
        case 4:
            EnemyGroup->AddNewObject(enemy = new BigTankEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
            break;
        default:
            return;
    }
    enemy->UpdatePath(mapDistance);
    enemy->Update(extraTicks);
}