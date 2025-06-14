# Tower Defense Game (I2P(II) Final Project)

A modern Tower Defense game built in C++ using Allegro 5, inspired by Emberward mechanics. Players place turrets on Tetromino blocks to dynamically shape enemy paths while defending the base. The game features adaptive difficulty, online and local accounts, scoreboards, and upgradeable towers.

---

## 🎮 Features

- 🧱 Tetromino Placement to shape the enemy path and can be place turret: ( right-click to rotate, left-click to place in the map) *T-L-I shape
- 🔫 Multiple Turret Types with upgrades up to 2 times ( right-click to upgrade ) * special bow-turret can be drag by user 
- 🧠 Adaptive Enemy Waves (AI director adjusts difficulty per waves) enemy 9 types with 3 enemy advanced skills
      Enemy 9 types :
      Slime ( Regen  )
      Skeleton ( Dash )
      Bat ( Speed )
      Muhsroom ( Teleport )
      Golem ( Low Hp->Increase Speed)
      Demon (Spawning Bat Every 3 Second) ( When died : Change to 5 Bat )
      Canina ( Green shield )-> Any Enemies inside that shield can't be damaged)
      Necromancer (Red Aura )-> Any Enemies inside that Aura got more Speed)
      Sorcerer (Purple Ocean Aura )-> Any Turret in that Radius will be turned off )
-😎 Hero can be place in map to attack enemy
- 🧍 Local ( Guest and username password)& Firebase-Based Online Accounts( email password)
- 🏆 Scoreboard System with time-based bonuses ( Local and Online)
- 🎵 Audio Settings (BGM/SFX sliders in in-game pause panel and start menu)
- 🚀 Cheat Code Support: LEFT → UP → RIGHT → DOWN
- 🛒 In-Game Shop with crystal-based rewards
- ⏳ Quest Mission with crystal rewards
- 📦 Organized scene transitions and modular structure for user flow

---

## 🔧 How to Build

### Requirements

- C++17 or later
- Allegro 5 libraries (`allegro`, `allegro_image`, `allegro_font`, `allegro_audio`, `allegro_primitives`, `allegro_ttf`)
- CMake
- OpenSSL (for Firebase login/register and Scoreboard Online)

### Build Instructions

```bash
git clone https://github.com/your-username/tower-defense-game.git
cd tower-defense-game
mkdir build
cd build
cmake ..
make
```

---

## 🔐 SSL Setup for Firebase

### 🪟 Windows Setup

1. Download OpenSSL  
   - https://slproweb.com/products/Win32OpenSSL.html  
   - Download: Win64 OpenSSL v1.1.1+

2. Install and Configure  
   - Install to:  
     ```
     C:\OpenSSL-Win64
     ```
   - Add to system PATH:  
     ```
     C:\OpenSSL-Win64\bin
     ```

3. Update Your CMakeLists.txt  
```cmake
find_package(OpenSSL REQUIRED)
target_link_libraries(YourGame PRIVATE OpenSSL::SSL OpenSSL::Crypto)
```

4. Copy DLLs to Your Build Folder  
```
libssl-1_1-x64.dll  
libcrypto-1_1-x64.dll
```

---

### 🍎 macOS Setup

1. Install OpenSSL with Homebrew  
```bash
brew install openssl
```

2. Update Your CMakeLists.txt  
```cmake
find_package(OpenSSL REQUIRED)
include_directories(/opt/homebrew/opt/openssl/include)
link_directories(/opt/homebrew/opt/openssl/lib)
target_link_libraries(YourGame PRIVATE OpenSSL::SSL OpenSSL::Crypto)
```

3. If Needed, Set Library Path When Running  
```bash
DYLD_LIBRARY_PATH=/opt/homebrew/opt/openssl/lib ./YourGame
```

## 👥 Contributors

- **Daryn James Welling (白鸿恩) — 113006271**  
  Local and online account for mac and windows, tetromino-based map editor, map UI, UI handling

- **Revallo (朱冠名) — 113006268**  
  Upgradable Turret and Skilled Enemies, Quest and Mission, Hero Character, Shop, UI handling

- **Thet Zaw Hein (陳家俊) — 113006270**  
  Online account debug for Windows, AI adaptive enemy waves, UI handling (Button UI, Background, and Scoreboard)

*Based on the I2P(II) Final Project at National Tsing Hua University (NTHU)*


