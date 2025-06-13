#ifndef REGISTER_SCENE_HPP
#define REGISTER_SCENE_HPP

#include "Engine/IScene.hpp"
#include "UI/Animation/ParallaxBackground.hpp"
#include <string>

namespace Engine {
    class Label;
    class ImageButton;
}

class RegisterScene : public Engine::IScene {
public:
    RegisterScene();
    ~RegisterScene();

    void Initialize() override;
    void Terminate()  override;
    void Update(float deltaTime) override;
    void Draw() const override;

    void OnKeyChar(int unicode) override;

    // <-- Again, override the engine’s OnMouseDown:
    void OnMouseDown(int button, int x, int y) override;

private:
    std::string typedUsername;
    std::string typedPassword;
    std::string typedConfirmPassword;

    enum InputField { UsernameField, PasswordField, ConfirmField };
    InputField activeField;

    Engine::Label* usernamePromptLabel;
    Engine::Label* usernameInputLabel;
    Engine::Label* passwordPromptLabel;
    Engine::Label* passwordInputLabel;
    Engine::Label* confirmPromptLabel;
    Engine::Label* confirmInputLabel;

    Engine::Label* validationLabel;   // red text under confirm

    Engine::ImageButton* confirmButton;
    Engine::ImageButton* backButton;

    std::string validationMessage;

    void ToggleInputFocus();
    std::string CheckPasswordStrength(const std::string& pwd) const;
    void OnConfirmClicked();
    void OnBackClicked();

    // Now define three bounding boxes:
    int usernameBoxX, usernameBoxY, usernameBoxW, usernameBoxH;
    int passwordBoxX, passwordBoxY, passwordBoxW, passwordBoxH;
    int confirmBoxX,  confirmBoxY,  confirmBoxW,  confirmBoxH;
    ParallaxBackground parallax;
};

#endif // REGISTER_SCENE_HPP
