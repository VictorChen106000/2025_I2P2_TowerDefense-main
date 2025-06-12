cmake_minimum_required(VERSION 3.27)
project(2025_I2P2_TowerDefense)

set(CMAKE_CXX_STANDARD 17)

# include_directories(.)
# include_directories(Resource)
# -----------------------------------------------------------------------------
# Include paths
# -----------------------------------------------------------------------------
# Your own headers and third-party single-header libs
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# file(COPY Resource/audios DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY Resource/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY Resource/images DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY Resource DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# file(COPY allegro_monolith-5.2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# -----------------------------------------------------------------------------
# Resources & DLLs
# -----------------------------------------------------------------------------
# Copy the Allegro DLL next to the exe
file(COPY allegro_monolith-5.2.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(2025_I2P2_TowerDefense

    Account/AccountManager.cpp
    Account/AccountManager.hpp
    Account/LoginScene.cpp
    Account/LoginScene.hpp
    Account/RegisterScene.cpp
    Account/RegisterScene.hpp
    Account/LocalAndOnlineScene.cpp
    Account/LocalAndOnlineScene.hpp
    Account/LoginOnlineScene.cpp
    Account/LoginOnlineScene.hpp
    Account/RegisterOnlineScene.cpp
    Account/RegisterOnlineScene.hpp
    Account/ScoreboardOnline.cpp
    Account/ScoreboardOnline.hpp
    
    Bullet/Bullet.cpp
    Bullet/Bullet.hpp
    Bullet/FireBullet.cpp
    Bullet/FireBullet.hpp
    Bullet/LaserBullet.cpp
    Bullet/LaserBullet.hpp
    Bullet/RocketBullet.hpp
    Bullet/RocketBullet.cpp

    # TODO HACKATHON-3 (3/3): Add the new enemy source files here
    Enemy/Enemy.cpp
    Enemy/Enemy.hpp
    Enemy/SoldierEnemy.cpp
    Enemy/SoldierEnemy.hpp
    Enemy/TankEnemy.cpp
    Enemy/TankEnemy.hpp
    Enemy/PlaneEnemy.cpp
    Enemy/PlaneEnemy.hpp
    Enemy/BigTankEnemy.hpp
    Enemy/BigTankEnemy.cpp
    Enemy/WolfEnemy.cpp
    Enemy/WolfEnemy.hpp
    Enemy/SlimeEnemy.cpp
    Enemy/SlimeEnemy.hpp
    Enemy/GolemEnemy.cpp
    Enemy/GolemEnemy.hpp
    Enemy/FlyEnemy.cpp
    Enemy/FlyEnemy.hpp
    Enemy/BatEnemy.cpp
    Enemy/BatEnemy.hpp
    Enemy/DemonEnemy.cpp
    Enemy/DemonEnemy.hpp


    Engine/Allegro5Exception.cpp
    Engine/Allegro5Exception.hpp
    Engine/AudioHelper.cpp
    Engine/AudioHelper.hpp
    Engine/Collider.cpp
    Engine/Collider.hpp
    Engine/GameEngine.cpp
    Engine/GameEngine.hpp
    Engine/Group.cpp
    Engine/Group.hpp
    Engine/IControl.cpp
    Engine/IControl.hpp
    Engine/IObject.cpp
    Engine/IObject.hpp
    Engine/IScene.cpp
    Engine/IScene.hpp
    Engine/LOG.cpp
    Engine/LOG.hpp
    Engine/Point.cpp
    Engine/Point.hpp
    Engine/Resources.cpp
    Engine/Resources.hpp
    Engine/Sprite.cpp
    Engine/Sprite.hpp
    
    # TODO HACKATHON-2 (3/3): Add the new scene source files here
    Scene/LoseScene.cpp
    Scene/LoseScene.hpp
    Scene/PlayScene.cpp
    Scene/PlayScene.hpp
    Scene/StageSelectScene.cpp
    Scene/StageSelectScene.hpp
    Scene/StartScene.cpp
    Scene/StartScene.h
    Scene/WinScene.cpp
    Scene/WinScene.hpp
    Scene/SettingsScene.cpp
    Scene/SettingsScene.hpp
    Scene/Scoreboard.hpp
    Scene/Scoreboard.cpp
    Scene/ShopScene.cpp
    Scene/ShopScene.hpp
    Scene/ModeSelectionScene.hpp
    Scene/ModeSelectionScene.cpp

    Turret/LaserTurret.cpp
    Turret/LaserTurret.hpp
    Turret/MachineGunTurret.cpp
    Turret/MachineGunTurret.hpp
    Turret/RocketTurret.hpp
    Turret/RocketTurret.cpp
    Turret/Turret.cpp
    Turret/Turret.hpp
    Turret/TurretButton.cpp
    Turret/TurretButton.hpp
    Turret/BombTurret.cpp
    Turret/BombTurret.hpp
    Turret/BallistaTurret.cpp
    Turret/BallistaTurret.hpp
    Turret/BowTurret.cpp
    Turret/BowTurret.hpp

    Shovel/ShovelButton.hpp

    UI/Animation/DirtyEffect.cpp
    UI/Animation/DirtyEffect.hpp
    UI/Animation/ExplosionEffect.cpp
    UI/Animation/ExplosionEffect.hpp
    UI/Animation/Plane.cpp
    UI/Animation/Plane.hpp

    UI/Component/Image.cpp
    UI/Component/Image.hpp
    UI/Component/ImageButton.cpp
    UI/Component/ImageButton.hpp
    UI/Component/Label.cpp
    UI/Component/Label.hpp
    UI/Component/Slider.cpp
    UI/Component/Slider.hpp
    
    main.cpp
)

# -----------------------------------------------------------------------------
# Platform-specific configuration
# -----------------------------------------------------------------------------
if (WIN32)
  # Allegro include + lib paths
  target_include_directories(2025_I2P2_TowerDefense PUBLIC
    C:/allegro/include
  )
  target_link_directories(2025_I2P2_TowerDefense PUBLIC
    C:/allegro/lib
  )

  # Find & link OpenSSL for HTTPS support
  find_package(OpenSSL REQUIRED)

  # Enable cpp-httplib SSLClient
  target_compile_definitions(2025_I2P2_TowerDefense PUBLIC
    CPPHTTPLIB_OPENSSL_SUPPORT
  )

  target_link_libraries(2025_I2P2_TowerDefense PUBLIC
    liballegro_monolith
    OpenSSL::SSL
    OpenSSL::Crypto

    ws2_32
    crypt32
  )
else()
  # On Unix/macOS, find Allegro via pkg-config
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(ALLEGRO5 REQUIRED
    allegro-5
    allegro_main-5
    allegro_font-5
    allegro_dialog-5
    allegro_primitives-5
    allegro_image-5
    allegro_ttf-5
    allegro_audio-5
    allegro_acodec-5
  )

    target_compile_options(2025_I2P2_TowerDefense
    PUBLIC
      ${ALLEGRO5_CFLAGS_OTHER}
  )

  target_include_directories(2025_I2P2_TowerDefense PUBLIC
    ${ALLEGRO5_INCLUDE_DIRS}
  )
  target_link_directories(2025_I2P2_TowerDefense PUBLIC
    ${ALLEGRO5_LIBRARY_DIRS}
  )

  # Find & link OpenSSL as well
  find_package(OpenSSL REQUIRED)

  target_compile_definitions(2025_I2P2_TowerDefense PUBLIC
    CPPHTTPLIB_OPENSSL_SUPPORT
  )

  target_link_libraries(2025_I2P2_TowerDefense PUBLIC
    ${ALLEGRO5_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
  )
endif()

# -----------------------------------------------------------------------------
# Post‐build: copy your Resource folder next to the exe (lower-case “resource”)
# -----------------------------------------------------------------------------
add_custom_command(TARGET 2025_I2P2_TowerDefense POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/Resource"
    "$<TARGET_FILE_DIR:2025_I2P2_TowerDefense>/resource"
)