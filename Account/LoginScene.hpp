// LoginScene.hpp

#ifndef LOGIN_SCENE_HPP
#define LOGIN_SCENE_HPP

#include "Engine/IScene.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include <string>

namespace Engine {
    class ImageButton;
    class Label;
}

class LoginScene : public Engine::IScene {
public:
    LoginScene();
    ~LoginScene();

    void Initialize() override;
    void Terminate() override;
    void Update(float dt) override;
    void Draw() const override;
    void OnKeyChar(int unicode) override;
    void OnMouseDown(int button, int x, int y) override;

private:
    // Toggle between username/password input focus
    void ToggleInputFocus();

    // Called when “Login” is clicked
    void OnLoginClicked();

    // Called when “Register” is clicked
    void OnRegisterClicked();

    // Called when “Back” is clicked
    void OnBackClicked();

    // ── Member variables (existing) ────────────────────────────────────
    std::string typedUsername;
    std::string typedPassword;
    bool        typingUsername;

    Engine::Label*       usernamePromptLabel;
    Engine::Label*       usernameInputLabel;
    Engine::Label*       passwordPromptLabel;
    Engine::Label*       passwordInputLabel;
    Engine::Label*       infoLabel;

    Engine::ImageButton* loginButton;
    Engine::ImageButton* registerButton;
    Engine::ImageButton* guestButton;

    // Coordinates and dimensions for drawing input boxes
    int usernameBoxX, usernameBoxY, usernameBoxW, usernameBoxH;
    int passwordBoxX, passwordBoxY, passwordBoxW, passwordBoxH;

    std::string errorMessage;

    // ── Static/FILE‐SCOPE resources (existing) ─────────────────────────
    static ALLEGRO_FONT*    loginFont;
    static bool             loginRevealPassword;
    static ALLEGRO_BITMAP*  openEyeBmp;
    static ALLEGRO_BITMAP*  closeEyeBmp;
    static Engine::ImageButton* eyeButton;

    // ── NEW “Back” button ──────────────────────────────────────────────
    Engine::ImageButton* backButton;
    Engine::Label*       backLabel;
};

#endif // LOGIN_SCENE_HPP
