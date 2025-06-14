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
#include "Enemy/WolfEnemy.hpp"
#include "Enemy/PlaneEnemy.hpp"
#include "Enemy/BigTankEnemy.hpp"
#include "Enemy/SlimeEnemy.hpp"
#include "Enemy/GolemEnemy.hpp"
#include "Enemy/FlyEnemy.hpp"
#include "Enemy/BatEnemy.hpp"
#include "Enemy/DemonEnemy.hpp"
#include "Enemy/CaninaEnemy.hpp"
#include "Enemy/NecromancerEnemy.hpp"
#include "Enemy/SorcererEnemy.hpp"

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
#include "Turret/Hero.hpp"
#include "Shovel/ShovelButton.hpp"
#include "Turret/BallistaTurret.hpp"
#include "UI/Animation/DirtyEffect.hpp"
#include "UI/Animation/Plane.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"
#include "Turret/BombTurret.hpp"
#include "Tetris/TetrisBlock.hpp"
#include "UI/Component/ImageButton.hpp"

// TODO HACKATHON-4 (1/3): Trace how the game handles keyboard input.
// TODO HACKATHON-4 (2/3): Find the cheat code sequence in this file.
// TODO HACKATHON-4 (3/3): When the cheat code is entered, a plane should be spawned and added to the scene.
// TODO HACKATHON-5 (1/4): There's a bug in this file, which crashes the game when you win. Try to find it.
// TODO HACKATHON-5 (2/4): The "LIFE" label are not updated when you lose a life. Try to fix it.
        // for Enemy*
extern std::vector<Enemy*> g_enemies;
bool PlayScene::DebugMode = false;
const std::vector<Engine::Point> PlayScene::directions = { Engine::Point(-1, 0), Engine::Point(0, -1), Engine::Point(1, 0), Engine::Point(0, 1) };
const int PlayScene::MapWidth = 20, PlayScene::MapHeight = 13;
const int PlayScene::BlockSize = 64;
const float PlayScene::DangerTime = 7.61;
std::vector<Engine::Point> PlayScene::SpawnGridPoints;
std::vector<Engine::Point> PlayScene::EndGridPoints;



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
    AddNewObject(HeroGroup= new Group());
    // Should support buttons.
    AddNewControlObject(UIGroup = new Group());
    ReadMap();
    ReadEnemyWave();
    staticWaveCount = int(enemyWaveData.size());
    currentWave = 1;
    adaptiveSpawnCount = 0;
    if (GetMode() == Mode::Normal) maxWaves = 3;
    else maxWaves = INT_MAX;
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

        // ─── 1) Static file spawns (Wave 1) ──────────────────────────────────
        if (!enemyWaveData.empty()) {
            auto [type, wait] = enemyWaveData.front();
            if (ticks >= wait) {
                ticks -= wait;
                enemyWaveData.pop_front();
                SpawnEnemyOfType(type, ticks);

                // count toward adaptive‐burst when on Wave 2+:
                if (currentWave >= 2)
                    ++adaptiveSpawnCount;
            }
        }
        else {
            // ─── 2) Transition off static Wave 1 ──────────────────────────────
            if (currentWave == 1) {
                currentWave        = 2;
                adaptiveSpawnCount = 0;
                ticks              = 0;
                nextAdaptiveWait   = 0;
            }

            // ─── 3a) NORMAL MODE: exactly 3 waves ─────────────────────────────
            if (GetMode() == Mode::Normal) {
                // 3a.i) if still under spawn‐limit this wave, do adaptive spawn
                if (currentWave <= maxWaves && adaptiveSpawnCount < staticWaveCount) {
                    if (ticks >= nextAdaptiveWait) {
                        ticks -= nextAdaptiveWait;

                        // === BEGIN DEBUG + SEPARATION from old logic ===
                        int dbgCount = static_cast<int>(TowerGroup->GetObjects().size());
                        float dbgDPS = CalculatePlayerPower();
                        const float α = 0.5f, β = 1.0f, γ = 0.2f;
                        float dbgD   = α*dbgCount + β*dbgDPS + γ*elapsedTime;
                        auto [dbgType, dbgWait] = GenerateAdaptiveEnemy();

                        printf(
                        "[ADAPTIVE DEBUG] count=%d  totalDPS=%.2f  time=%.1f  D=%.2f  type=%d  wait=%.2f  alive=%zu  wave=%d\n",
                        dbgCount, dbgDPS, elapsedTime, dbgD, dbgType, dbgWait,
                        EnemyGroup->GetObjects().size(), currentWave
                        );

                        bool isGround       = (dbgType==1||dbgType==3||dbgType==4);
                        float timeSinceLast = elapsedTime - lastGroundSpawnTime;
                        if (isGround && timeSinceLast < minGroundGap) {
                            float leftover = (minGroundGap - timeSinceLast);
                            nextAdaptiveWait = dbgWait + leftover;
                        } else {
                            if (isGround) lastGroundSpawnTime = elapsedTime;
                            nextAdaptiveWait = dbgWait;
                            SpawnEnemyOfType(dbgType, ticks);
                        }
                        // === END DEBUG + SEPARATION from old logic ===

                        ++adaptiveSpawnCount;
                    }
                }
                // 3a.ii) finished this wave’s spawns → wait for all to die, then bump wave
                else if (adaptiveSpawnCount >= staticWaveCount
                        && EnemyGroup->GetObjects().empty())
                {
                    ++currentWave;
                    adaptiveSpawnCount = 0;
                    ticks              = 0;
                    nextAdaptiveWait   = 0;
                }
            }

            // ─── 3b) SURVIVAL MODE: endless adaptive ────────────────────────────
            else {
                // always spawn, identical logic to above debug+separation
                if (ticks >= nextAdaptiveWait) {
                    ticks -= nextAdaptiveWait;

                    // === BEGIN DEBUG + SEPARATION from old logic ===
                    int dbgCount = static_cast<int>(TowerGroup->GetObjects().size());
                    float dbgDPS = CalculatePlayerPower();
                    const float α = 0.5f, β = 1.0f, γ = 0.2f;
                    float dbgD   = α*dbgCount + β*dbgDPS + γ*elapsedTime;
                    auto [dbgType, dbgWait] = GenerateAdaptiveEnemy();

                    printf(
                    "[ADAPTIVE DEBUG] count=%d  totalDPS=%.2f  time=%.1f  D=%.2f  type=%d  wait=%.2f  alive=%zu  wave=%d\n",
                    dbgCount, dbgDPS, elapsedTime, dbgD, dbgType, dbgWait,
                    EnemyGroup->GetObjects().size(), currentWave
                    );

                    bool isGround       = (dbgType==1||dbgType==3||dbgType==4);
                    float timeSinceLast = elapsedTime - lastGroundSpawnTime;
                    if (isGround && timeSinceLast < minGroundGap) {
                        float leftover = (minGroundGap - timeSinceLast);
                        nextAdaptiveWait = dbgWait + leftover;
                    } else {
                        if (isGround) lastGroundSpawnTime = elapsedTime;
                        nextAdaptiveWait = dbgWait;
                        SpawnEnemyOfType(dbgType, ticks);
                    }
                    // === END DEBUG + SEPARATION from old logic ===
                }
            }
        }

        // ─── 4) WIN check (Normal only after 3 waves) ─────────────────────────
        if (GetMode() == Mode::Normal
            && currentWave > maxWaves
            && enemyWaveData.empty()
            && EnemyGroup->GetObjects().empty())
        {
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
    if (placing && previewBlock) {
        // Left-click: either place if over map, or cancel if outside
        if (button & 1) {
            int gx = mx / BlockSize;
            int gy = my / BlockSize;
            if (clickOnMap) {
                // attempt placement
                bool ok = true;
                for (auto [dx,dy] : previewBlock->GetCells()) {
                    int xx = gx + dx, yy = gy + dy;
                    if (xx < 0 || xx >= MapWidth || yy < 0 || yy >= MapHeight
                     || mapState[yy][xx] != TILE_WHITE_FLOOR) {
                        ok = false;
                        break;
                    }
                }
                if (ok && !CanPlaceTetrisAt(gx, gy, previewBlock->GetCells()))
                    ok = false;
                if (ok) {
                    // commit placement
                    for (auto* cell : previewBlock->GetSprites()) {
                        cell->GetObjectIterator()->first = false;
                        UIGroup->RemoveObject(cell->GetObjectIterator());
                        GroundEffectGroup->AddNewObject(cell);
                    }
                    previewBlock->SetPosition(gx * BlockSize, gy * BlockSize);
                    for (auto [dx,dy] : previewBlock->GetCells())
                        mapState[gy+dy][gx+dx] = TILE_TETRIS;
                    mapDistance = CalculateBFSDistance();
                    for (auto& obj : EnemyGroup->GetObjects())
                        dynamic_cast<Enemy*>(obj)->UpdatePath(mapDistance);
                    delete previewBlock;
                    previewBlock = nullptr;
                    placing = false;
                } else {
                    // invalid placement feedback
                    Engine::Sprite* spr = new DirtyEffect(
                        "play/target-invalid.png", 1,
                        gx * BlockSize + BlockSize/2,
                        gy * BlockSize + BlockSize/2
                    );
                    GroundEffectGroup->AddNewObject(spr);
                    spr->Rotation = 0;
                    // keep placing = true for retry
                }
            } else {
                // clicked outside map: cancel drag
                for (auto* cellSprite : previewBlock->GetSprites())
                    UIGroup->RemoveObject(cellSprite->GetObjectIterator());
                delete previewBlock;
                previewBlock = nullptr;
                placing = false;
            }
            return; // consume click
        }
    }
    if ((button & 2) && placing && previewBlock) {
        // rotate the shape
        previewBlock->Rotate();

        // re-snap to grid under the mouse
        int gx = mx / BlockSize;
        int gy = my / BlockSize;
        previewBlock->SetPosition(gx * BlockSize, gy * BlockSize);

        // swallow the click so it doesn’t also cancel your drag:
        return;
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
    // 0) click on one of our panel icons?
// 0) click on one of our panel icons?
// 0) click on one of our panel icons?
if ((button & 1) && !placing) {
    for (auto &tpl : _tetrisIcons) {
      TetrominoType type;
      int x,y,w,h;
      std::tie(type,x,y,w,h) = tpl;
      if (mx >= x && mx < x + w
       && my >= y && my < y + h) {
        
        // ——— clean up any old preview ———
        if (previewBlock) {
          for (auto* oldCell : previewBlock->GetSprites())
            UIGroup->RemoveObject(oldCell->GetObjectIterator());
          previewBlock = nullptr;
          placing = false;
        }
        
        // ——— start a new drag ———
        previewBlock = new TetrisBlock(type, BlockSize);
        placing      = true;
  
        // snap it into place before drawing
        int gx = mx / BlockSize;
        int gy = my / BlockSize;
        previewBlock->SetPosition(gx * BlockSize, gy * BlockSize);
  
        // add its sprites into UIGroup
        for (auto* cell : previewBlock->GetSprites()) {
          cell->Visible = true;
          UIGroup->AddNewObject(cell);
        }
        return;  // consume the click
      }
    }
}

  
  
    // Always forward to base for button callbacks (so your TurretButton / ShovelButton still work)
    IScene::OnMouseDown(button, mx, my);
}
void PlayScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);

    // grid coords
    const int gx = mx / BlockSize;
    const int gy = my / BlockSize;
    if (heroMode && heroPreview) {
        int gx = mx / BlockSize, gy = my / BlockSize;
        heroPreview->Position = Engine::Point(
          gx * BlockSize + BlockSize/2,
          gy * BlockSize + BlockSize/2
        );
    }

    // Aiming turret early-out
    if (isAiming && aimingTurret) {
        float dx = mx - aimingTurret->Position.x;
        float dy = my - aimingTurret->Position.y;
        aimingTurret->Rotation = std::atan2(dy, dx) + ALLEGRO_PI/2;
        return;
    }

    // hide target if off-map
    if (gx < 0 || gx >= MapWidth || gy < 0 || gy >= MapHeight) {
        imgTarget->Visible = false;
        return;
    }

    // snap your tetromino preview
    if (placing && previewBlock) {
        int gx = mx/BlockSize, gy = my/BlockSize;
        previewBlock->SetPosition(gx*BlockSize, gy*BlockSize);
      }

    // update your mouse-over highlight
    imgTarget->Visible   = true;
    imgTarget->Position.x = gx * BlockSize;
    imgTarget->Position.y = gy * BlockSize;
}

void PlayScene::OnMouseUp(int button, int mx, int my) {

    
    IScene::OnMouseUp(button, mx, my);

    if (placing && previewBlock) {
        // Optional: if you want release-outside to cancel:
        const int mapW = MapWidth * BlockSize;
        const int mapH = MapHeight * BlockSize;
        bool overMap = mx >= 0 && mx < mapW && my >= 0 && my < mapH;
        if (!overMap) {
            // cancel drag
            for (auto* cellSprite : previewBlock->GetSprites())
                UIGroup->RemoveObject(cellSprite->GetObjectIterator());
            delete previewBlock;
            previewBlock = nullptr;
            placing = false;
        }
        // If released over map, do nothing: placement was on mouse-down
    }

    if (isPaused) return; 
    const int mapPixelWidth  = MapWidth  * BlockSize;  // 20 * 64 = 1280
    const int mapPixelHeight = MapHeight * BlockSize;  // 13 * 64 =  832
    if (isAiming) {
        isAiming = false;
        aimingTurret = nullptr;
        return;  // prevent passing through to “place turret” logic
    }
    if (heroMode && heroPreview && (button & 1)) {
        int gx = mx / BlockSize, gy = my / BlockSize;
        // Only allow on enemy path (mapDistance >= 0)
        if (gy >= 0 && gy < MapHeight && gx >= 0 && gx < MapWidth
         && mapDistance[gy][gx] >= 0)
        {
            // spawn Hero
            float px = gx * BlockSize + BlockSize/2;
            float py = gy * BlockSize + BlockSize/2;
            Hero *h = new Hero(px, py);
            HeroGroup->AddNewObject(h);

            // clean up preview
            UIGroup->RemoveObject(heroPreview->GetObjectIterator());
            heroPreview = nullptr;
            heroMode = false;
        } else {
            // invalid spot: show a quick red X or dirty effect
            auto fx = new DirtyEffect(
              "play/target-invalid.png", 1,
              gx*BlockSize + BlockSize/2,
              gy*BlockSize + BlockSize/2
            );
            GroundEffectGroup->AddNewObject(fx);
        }
        return;  // consume the click—don’t fall into turret code
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
                             float vol = std::clamp(AudioHelper::SFXVolume * 20.0f, 0.0f, 20.0f);
                             AudioHelper::PlaySample("levelup.mp3", false, vol);

                            // Optionally: show a visual upgrade effect or change sprite, etc.
                        } else {
                            float vol = std::clamp(AudioHelper::SFXVolume * 20.0f, 0.0f, 20.0f);
                            AudioHelper::PlaySample("cancel.mp3", false, vol);
                        }
                    } else {
                        float vol = std::clamp(AudioHelper::SFXVolume * 15.0f, 0.0f, 15.0f);
                        AudioHelper::PlaySample("cancel.mp3", false, vol);
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
                TileType orig = mapState[y][x];
                bool isBomb = dynamic_cast<BombTurret *>(preview) != nullptr;
                if (!isBomb && orig != TILE_TETRIS) {
                    auto sprite = new DirtyEffect(
                        "play/target-invalid.png", 1,
                        x * BlockSize + BlockSize/2,
                        y * BlockSize + BlockSize/2
                    );
                    GroundEffectGroup->AddNewObject(sprite);
                    // 3) then set its rotation
                    sprite->Rotation = 0;
                    return;
                }
            
            if (isBomb) {
                if (mapState[y][x] != TILE_DIRT && mapState[y][x] != TILE_WHITE_FLOOR) {
                    Engine::Sprite *sprite;
                    GroundEffectGroup->AddNewObject(sprite = new DirtyEffect(
                        "play/target-invalid.png", 1,
                        x * BlockSize + BlockSize / 2,
                        y * BlockSize + BlockSize / 2));
                    sprite->Rotation = 0;
                    return;
                }
            } else {
                if (!CheckSpaceValid(x, y)) {
                    Engine::Sprite *sprite;
                    GroundEffectGroup->AddNewObject(sprite = new DirtyEffect(
                        "play/target-invalid.png", 1,
                        x * BlockSize + BlockSize / 2,
                        y * BlockSize + BlockSize / 2));
                    sprite->Rotation = 0;
                    return;
                }
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

bool PlayScene::CanPlaceTetrisAt(int gx, int gy, const TetrisBlock::Shape &cells) {
    // 1) Backup and block all of the target tiles
    std::vector<TileType> old;
    old.reserve(cells.size());
    for (auto [dx,dy] : cells) {
        old.push_back(mapState[gy+dy][gx+dx]);
        mapState[gy+dy][gx+dx] = TILE_TETRIS;
    }

    // 2) Recompute the distance‐map
    auto newDist = CalculateBFSDistance();

    // 3) Check that **every** spawn point can still reach an exit
    bool ok = true;
    for (auto &sp : SpawnGridPoints) {
        if (newDist[sp.y][sp.x] == -1) {
            ok = false;
            break;
        }
    }

    // 4) Restore the old mapState
    for (size_t i = 0; i < cells.size(); ++i) {
        auto [dx,dy] = cells[i];
        mapState[gy+dy][gx+dx] = old[i];
    }

    return ok;
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

    SpawnGridPoints.clear();
    EndGridPoints.clear();
    // 1) Open map file
    std::string filename = "Resource/map" + std::to_string(MapId) + ".txt";
    std::ifstream fin(filename);
    if (!fin) {
        throw std::ios_base::failure("Cannot open map file: " + filename);
    }

    // 2) Read all lines (preserves spaces)
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(fin, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();  // strip Windows CR
        }
        lines.push_back(line);
    }
    fin.close();

    // 3) Validate row count
    if ((int)lines.size() != MapHeight) {
        throw std::ios_base::failure(
            "Map data is corrupted: expected " + 
            std::to_string(MapHeight) + " rows but got " + 
            std::to_string(lines.size()));
    }

    // 4) Flatten into mapData with padding/truncation
    std::vector<TileType> mapData;
    mapData.reserve(MapWidth * MapHeight);
    for (int i = 0; i < MapHeight; i++) {
        // pad short lines or truncate long ones
        if ((int)lines[i].size() < MapWidth) {
            lines[i] += std::string(MapWidth - lines[i].size(), ' ');
        } else if ((int)lines[i].size() > MapWidth) {
            lines[i].resize(MapWidth);
        }

        for (int j = 0; j < MapWidth; j++) {
            char ch = lines[i][j];
            switch (ch) {
                // 1) Blue floor (D/d)
                case 'D': case 'd':
                    mapData.push_back(TILE_DIRT);
                    break;
            
                // 2) Big corner pieces
                case 'C':  // corner-bot-left.png
                    mapData.push_back(TILE_CORNER_BOT_LEFT);
                    break;
                case '3':  // corner-top-left.png
                    mapData.push_back(TILE_CORNER_TOP_LEFT);
                    break;
                case '4':  // corner-top-right.png
                    mapData.push_back(TILE_CORNER_TOP_RIGHT);
                    break;
                case '5':  // corner1.png
                    mapData.push_back(TILE_CORNER1);
                    break;
                case '6':  // corner2.png
                    mapData.push_back(TILE_CORNER2);
                    break;
                case '9':  // wall1 uses '9' as well as '#'
                case '#':
                    mapData.push_back(TILE_WALL1);
                    break;
            
                case 'S':  // spawn
                    SpawnGridPoints.push_back( Engine::Point(j, i) );
                    mapData.push_back(TILE_S); 
                    break;
                case 'E':  // exit
                    EndGridPoints.push_back( Engine::Point(j, i) );
                    mapData.push_back(TILE_S);
                    break;
                // 3) “Small” corner pieces (you’ll need to add these enums & assets)
                case 's':  // corner-small-1.png
                    mapData.push_back(TILE_CORNER_SMALL_1);
                    break;
                case 't':  // corner-small-2.png
                    mapData.push_back(TILE_CORNER_SMALL_2);
                    break;
                case 'u':  // corner-small-3.png
                    mapData.push_back(TILE_CORNER_SMALL_3);
                    break;
                case 'v':  // corner-small-4.png
                    mapData.push_back(TILE_CORNER_SMALL_4);
                    break;
                case 'w':  // corner-4.png
                    mapData.push_back(TILE_CORNER_4);
                    break;
                case 'x':  // corner-3.png
                    mapData.push_back(TILE_CORNER_3);
                    break;
            
                // 4) Platform & tile011
                case '7':  // platform.png
                    mapData.push_back(TILE_PLATFORM);
                    break;
                case '8':  // tile011.png
                    mapData.push_back(TILE_TILE011);
                    break;
            
                // 5) The other walls
                case 'A': case 'a': 
                    mapData.push_back(TILE_WALL2);  // wall2.png
                    break;
                case 'B': case 'b': 
                    mapData.push_back(TILE_WALL3);  // wall3.png
                    break;
            
                // 6) Standard floor & dirt
                case '1':  // white-floor.png
                    mapData.push_back(TILE_WHITE_FLOOR);
                    break;
                case '0':  // black/dirt background
                case ' ':  
                    mapData.push_back(TILE_BLUE_FLOOR);
                    break;
            
                default:
                    throw std::ios_base::failure(
                      std::string("Map data corrupted: invalid char '")
                      + ch + "' at row " + std::to_string(i)
                      + ", col " + std::to_string(j));
            }
            
        }
    }

    // 5) Validate total cell count
    if ((int)mapData.size() != MapWidth * MapHeight) {
        throw std::ios_base::failure(
            "Map data is corrupted: expected " +
            std::to_string(MapWidth * MapHeight) + " cells but got " +
            std::to_string(mapData.size()));
    }

    // 6) Store in mapState and create tile images
    mapState.assign(MapHeight, std::vector<TileType>(MapWidth));
    TileMapGroup->Clear();
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++) {
            TileType t = mapData[i * MapWidth + j];
            mapState[i][j] = t;

            float x = j * BlockSize;
            float y = i * BlockSize;
            switch (t) {
                // 1) Dirt (background)
                case TILE_DIRT:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("play/dirt.png", x, y, BlockSize, BlockSize));
                    break;
            
                // 2) White floor (‘1’)
                case TILE_WHITE_FLOOR:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/white-floor.png", x, y, BlockSize, BlockSize));
                    break;
            
                // 3) Blue floor (‘D’/‘d’)
                case TILE_BLUE_FLOOR:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/blue-floor.png", x, y, BlockSize, BlockSize));
                    break;
            
                // 4) Big corners
                case TILE_CORNER_BOT_LEFT:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner-bot-left.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_CORNER_TOP_LEFT:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner-top-left.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_CORNER_TOP_RIGHT:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner-top-right.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_CORNER1:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner1.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_CORNER2:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner2.png", x, y, BlockSize, BlockSize));
                    break;
            
                // 5) Small corners
                case TILE_CORNER_SMALL_1:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner-small-1.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_CORNER_SMALL_2:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner-small-2.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_CORNER_SMALL_3:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner-small-3.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_CORNER_SMALL_4:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner-small-4.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_CORNER_4:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner4.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_CORNER_3:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/corner3.png", x, y, BlockSize, BlockSize));
                    break;
            
                // 6) Platform & special tile
                case TILE_PLATFORM:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/platform.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_TILE011:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/tile011.png", x, y, BlockSize, BlockSize));
                    break;
            
                // 7) Walls
                case TILE_WALL1:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/wall1.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_WALL2:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/wall2.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_WALL3:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/wall3.png", x, y, BlockSize, BlockSize));
                    break;
                case TILE_S:
                    TileMapGroup->AddNewObject(
                        new Engine::Image("tile/S-top.png", x, y, BlockSize, BlockSize));
                    break;
            
                // 8) (Optional) Occupied marker
                case TILE_OCCUPIED:
                    // you can draw your “occupied” overlay here, if you use one
                    break;
            
                default:
                    // nothing to draw
                    break;
            }
            
        }
    }
}

void PlayScene::ReadEnemyWave() {
    std::string filename = std::string("./Resource/enemy") + std::to_string(MapId) + ".txt";
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
    // DASHBOARD
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
        1520, 136, BallistaTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 5));
    UIGroup->AddNewControlObject(btn);

     btn = new TurretButton(
        "play/floor.png", "play/dirt.png",
        Engine::Sprite("play/tower-base.png", 1446, 240, 0, 0, 0, 0),
        Engine::Sprite("play/Ballista2.png", 1446, 240, 0, 0, 0, 0),
        1446, 240, BowTurret::Price);
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

    //hero
    auto heroBtn = new Engine::ImageButton(
    "play/yellowninja.png",   // up & down images
    "play/yellowninja.png",
    1294, 300,       // pick a spot in your side panel
    64, 64
);
    heroBtn->SetOnClickCallback([this](){
        if (isPaused) return;
        heroMode = true;
        heroPreview = new Engine::Sprite("play/yellowninja.png", 0, 0);
        heroPreview->Tint = al_map_rgba(255,255,255,150);
        heroPreview->Anchor = Engine::Point(0.5, 0.5);
        UIGroup->AddNewObject(heroPreview);
    });
    UIGroup->AddNewControlObject(heroBtn);

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
    
    //tetris button
    // right of your map, under the turret/shovel buttons.
// pick a small icon size (e.g. 32px) and a y‐offset in the panel
        // in PlayScene.cpp, inside PlayScene::ConstructUI():

// … your existing turret/shovel button setup here …

// 1) STATIC 32×32 ICONS  
const int iconSize = 32;
const std::vector<TetrominoType> tetTypes = {
  TetrominoType::T,
  TetrominoType::L,
  TetrominoType::I
};

int panelX = MapWidth * BlockSize + 20;  // just right of the map
int panelY = 600;                        // under your turret/shovel row

_tetrisIcons.clear();

for (int i = 0; i < 3; ++i) {
  TetrominoType type = tetTypes[i];
  auto *icon = new TetrisBlock(type, iconSize);

  // position the little icon
  int ix = panelX + i * (iconSize*3 + 10);
  int iy = panelY;
  icon->SetPosition(ix, iy);
  icon->CommitToScene(UIGroup);

  // remember its clickable area (max tetro width = 3 cells, height ~2)
  _tetrisIcons.emplace_back(type, ix, iy, iconSize*3, iconSize*2);
}


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
    static auto isWalkable = [](TileType t){
        return t == TILE_S || t == TILE_WHITE_FLOOR;   // add any other “walkable” types here
    };
    // Prepare distance grid, init to -1
    std::vector<std::vector<int>> dist(MapHeight, std::vector<int>(MapWidth, -1));
    std::queue<Engine::Point> que;

    // 1) Seed the queue with *all* exits
    for (auto &e : EndGridPoints) {
        if (e.x >= 0 && e.x < MapWidth && e.y >= 0 && e.y < MapHeight
            && isWalkable(mapState[e.y][e.x])) {
            dist[e.y][e.x] = 0;
            que.push(e);
        }
    }

    // 2) Standard BFS
    while (!que.empty()) {
        Engine::Point p = que.front(); que.pop();
        for (auto &dir : directions) {
            int nx = p.x + dir.x;
            int ny = p.y + dir.y;
            if (nx < 0 || nx >= MapWidth || ny < 0 || ny >= MapHeight) continue;
            if (!isWalkable(mapState[ny][nx]) || dist[ny][nx] != -1) continue;

            dist[ny][nx] = dist[p.y][p.x] + 1;
            que.push( Engine::Point(nx, ny) );

        }
    }

    return dist;
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
    static size_t nextSpawnIdx = 0;
    if (SpawnGridPoints.empty()) return;             // ← use the static you filled
    Engine::Point g = SpawnGridPoints[nextSpawnIdx++];
    nextSpawnIdx %= SpawnGridPoints.size();          // wrap around
  
    float cx = g.x * BlockSize + BlockSize/2;
    float cy = g.y * BlockSize + BlockSize/2;
    Enemy* enemy = nullptr;
    switch (type) {
      case 1: EnemyGroup->AddNewObject(enemy = new SlimeEnemy(cx, cy)); break;
      case 2: EnemyGroup->AddNewObject(enemy = new WolfEnemy  (cx, cy)); break;
      case 3: EnemyGroup->AddNewObject(enemy = new GolemEnemy(cx, cy)); break;
      case 4: EnemyGroup->AddNewObject(enemy = new SorcererEnemy(cx, cy)); break;
      case 5: EnemyGroup->AddNewObject(enemy = new GolemEnemy(cx, cy)); break;
      case 6: EnemyGroup->AddNewObject(enemy = new CaninaEnemy(cx, cy)); break;
      case 7: EnemyGroup->AddNewObject(enemy = new SlimeEnemy(cx, cy)); break;
      case 8: EnemyGroup->AddNewObject(enemy = new WolfEnemy(cx, cy)); break;
      case 9: EnemyGroup->AddNewObject(enemy = new NecromancerEnemy(cx, cy)); break;
      default: return;
    }

    // only once, here:
    if (enemy) {
        g_enemies.push_back(enemy);
        enemy->UpdatePath(mapDistance);
        enemy->Update(extraTicks);
      }
}


  