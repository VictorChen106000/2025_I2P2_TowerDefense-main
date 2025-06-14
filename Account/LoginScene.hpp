// LoginScene.hpp
#ifndef LOGIN_SCENE_HPP
#define LOGIN_SCENE_HPP

#include "Engine/IScene.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Animation/ParallaxBackground.hpp"
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

    struct TextField {
        int          x, y, w, h;      // box geometry
        std::string  text;            // stored chars
        size_t       caretIndex = 0;  // insertion point
        float        scrollX   = 0;   // horizontal offset
        size_t       selStart  = 0;   // selection anchor
        size_t       selEnd    = 0;   // selection caret
        std::string  placeholder;     // shown when empty
    };

private:
    void ToggleInputFocus();
    void OnLoginClicked();
    void OnRegisterClicked();
    void OnBackClicked();

    // — Text fields for username & password —
    TextField usernameField;
    TextField passwordField;

    // mirror for login logic
    std::string typedUsername;
    std::string typedPassword;
    bool        typingUsername;

    // prompts, buttons, info label
    Engine::Label*       usernamePromptLabel;
    Engine::Label*       passwordPromptLabel;
    Engine::Label*       infoLabel;
    Engine::ImageButton* loginButton;
    Engine::ImageButton* registerButton;
    Engine::ImageButton* guestButton;
    Engine::ImageButton* backButton;
    Engine::Label*       backLabel;

    // geometry for drawing highlight around fields
    int usernameBoxX, usernameBoxY, usernameBoxW, usernameBoxH;
    int passwordBoxX, passwordBoxY, passwordBoxW, passwordBoxH;

    std::string errorMessage;

    // — static/shared resources —
    static ALLEGRO_FONT*       loginFont;
    static bool                loginRevealPassword;
    static ALLEGRO_BITMAP*     openEyeBmp;
    static ALLEGRO_BITMAP*     closeEyeBmp;
    static Engine::ImageButton* eyeButton;
    ParallaxBackground parallax;
};

#endif // LOGIN_SCENE_HPP
