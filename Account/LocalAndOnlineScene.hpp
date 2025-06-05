#ifndef LOCAL_AND_ONLINE_SCENE_HPP
#define LOCAL_AND_ONLINE_SCENE_HPP

#include "Engine/IScene.hpp"

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

private:
    // callback handlers for each button
    void LocalOnClick();
    void OnlineOnClick();

    // two button objects
    Engine::ImageButton* localButton;
    Engine::ImageButton* onlineButton;
};

#endif // LOCAL_AND_ONLINE_SCENE_HPP
