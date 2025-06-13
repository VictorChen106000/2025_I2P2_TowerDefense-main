#ifndef LOCAL_AND_ONLINE_SCENE_HPP
#define LOCAL_AND_ONLINE_SCENE_HPP

#include "Engine/IScene.hpp"
#include <allegro5/allegro.h>

namespace Engine {
    class ImageButton;
    class Label;
}

class LocalAndOnlineScene : public Engine::IScene {
public:
    LocalAndOnlineScene();
    ~LocalAndOnlineScene();

    void Initialize() override;
    void Terminate() override;
    void Draw() const override;

private:
     // 4-layer parallax: depth4 static, depths 3–1 scroll
    ALLEGRO_BITMAP* background;  // depth 4 (static)
    ALLEGRO_BITMAP* layer3;      // depth 3 (slowest scrolling)
    ALLEGRO_BITMAP* layer2;      // depth 2 (medium scrolling)
    ALLEGRO_BITMAP* layer1;      // depth 1 (fastest scrolling)
    // callback handlers for each button
    void LocalOnClick();
    void OnlineOnClick();

    // two button objects
    Engine::ImageButton* localButton;
    Engine::ImageButton* onlineButton;
};

#endif // LOCAL_AND_ONLINE_SCENE_HPP
