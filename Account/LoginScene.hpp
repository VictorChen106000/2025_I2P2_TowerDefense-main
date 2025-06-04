#ifndef LOGIN_SCENE_HPP
#define LOGIN_SCENE_HPP

#include "Engine/IScene.hpp"
#include <string>

namespace Engine {
    class Label;
    class ImageButton;
}

class LoginScene : public Engine::IScene {
public:
    LoginScene();
    ~LoginScene();
    

    void Initialize() override;
    void Terminate()  override;
    void Update(float deltaTime) override;
    void Draw()   const override;

    void OnKeyChar(int unicode) override;

    // <-- Override the real mouse‐down signature, not OnMouseButtonDown:
    void OnMouseDown(int button, int x, int y) override;

private:
    std::string typedUsername;
    std::string typedPassword;
    bool typingUsername;   // true = Username field has focus; false = Password field

    Engine::Label* usernamePromptLabel;
    Engine::Label* usernameInputLabel;
    Engine::Label* passwordPromptLabel;
    Engine::Label* passwordInputLabel;
    Engine::Label* infoLabel;        // for errors

    Engine::ImageButton* loginButton;
    Engine::ImageButton* registerButton;
    Engine::ImageButton* guestButton;

    std::string errorMessage;

    void OnLoginClicked();
    void OnRegisterClicked();
    void ToggleInputFocus();

    // Bounding-box for each text field (to detect clicks)
    int usernameBoxX, usernameBoxY, usernameBoxW, usernameBoxH;
    int passwordBoxX, passwordBoxY, passwordBoxW, passwordBoxH;
};

#endif // LOGIN_SCENE_HPP
