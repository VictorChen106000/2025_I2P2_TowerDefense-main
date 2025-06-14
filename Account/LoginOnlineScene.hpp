// LoginOnlineScene.hpp
#ifndef LOGIN_ONLINE_SCENE_HPP
#define LOGIN_ONLINE_SCENE_HPP

#include "Engine/IScene.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Animation/ParallaxBackground.hpp"
#include <string>
#include <allegro5/allegro.h>

struct ALLEGRO_FONT;
struct ALLEGRO_BITMAP;

namespace Engine {
    class ImageButton;
    class Label;
}

class LoginOnlineScene : public Engine::IScene {
public:
    LoginOnlineScene();
    ~LoginOnlineScene();

    void Initialize() override;
    void Terminate() override;
    void Update(float dt) override;
    void Draw() const override;

    void OnKeyChar(int unicode) override;
    void OnMouseDown(int button, int x, int y) override;
     struct TextField {
        int          x, y, w, h;
        std::string  text;
        size_t       caretIndex = 0;
        float        scrollX   = 0;
    };

private:
    void ToggleInputFocus();
    void OnLoginClicked();
    void OnRegisterClicked();
    void OnBackClicked();

    // ── UI State ─────────────────────────────────────────────────────────
    std::string typedEmail;
    std::string typedPassword;
    bool        typingEmail;

    Engine::Label*        emailPromptLabel;
    // Engine::Label*      emailInputLabel;       // no longer needed
    Engine::Label*        passwordPromptLabel;
    // Engine::Label*      passwordInputLabel;    // no longer needed
    Engine::Label*        infoLabel;

    Engine::ImageButton*  loginButton;
    Engine::ImageButton*  registerButton;
    Engine::ImageButton*  backButton;
    Engine::Label*        backLabel;

    static bool            loginRevealPasswordOnline;
    static ALLEGRO_BITMAP* openEyeBmpOnline;
    static ALLEGRO_BITMAP* closeEyeBmpOnline;
    static Engine::ImageButton* eyeButtonOnline;

    static ALLEGRO_FONT*   onlineFont;

    TextField emailField;
    TextField passwordField;

    // geometry for drawing focus rectangle
    int emailBoxX, emailBoxY, emailBoxW, emailBoxH;
    int passwordBoxX, passwordBoxY, passwordBoxW, passwordBoxH;

    std::string errorMessage;
    ParallaxBackground parallax;
};

#endif // LOGIN_ONLINE_SCENE_HPP
