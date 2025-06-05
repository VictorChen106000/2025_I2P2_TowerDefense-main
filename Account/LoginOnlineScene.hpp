#ifndef LOGIN_ONLINE_SCENE_HPP
#define LOGIN_ONLINE_SCENE_HPP

#include "Engine/IScene.hpp"

namespace Engine {
    // forward‐declare anything you might need later (e.g. Label, ImageButton, etc.)
    // but for now, we leave it empty since you'll fill in logic yourself.
}

class LoginOnlineScene : public Engine::IScene {
public:
    LoginOnlineScene();
    ~LoginOnlineScene();

    void Initialize() override;
    void Terminate() override;

    // (you can add OnKeyChar, OnMouseDown, callbacks, etc. later)
};

#endif // LOGIN_ONLINE_SCENE_HPP
