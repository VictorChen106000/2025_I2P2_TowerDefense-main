#ifndef REGISTER_ONLINE_SCENE_HPP
#define REGISTER_ONLINE_SCENE_HPP

#include "Engine/IScene.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include <string>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

namespace Engine {
    class Label;
    class ImageButton;
}

///
/// RegisterOnlineScene:
///
///   Similar to LoginOnlineScene, but with three input boxes:
///     1. Email
///     2. Password
///     3. Confirm Password
///
///   The “Confirm Password” field is only used client‐side to ensure that
///   the user typed the same password twice.  We do NOT send “confirm” to Firebase.
///   Instead, in OnRegisterClicked() we check typedPassword == typedConfirmPassword,
///   and only if they match do we POST to Firebase’s signUp endpoint. Otherwise,
///   we display an error message (“Passwords do not match”) in the infoLabel.
///
class RegisterOnlineScene : public Engine::IScene {
public:
    RegisterOnlineScene();
    virtual ~RegisterOnlineScene();

    // IScene interface
    virtual void Initialize() override;
    virtual void Terminate() override;
    virtual void Update(float dt) override;
    virtual void Draw() const override;

    // Input handlers
    virtual void OnKeyChar(int unicode) override;
    virtual void OnMouseDown(int button, int x, int y) override;

private:
    void ToggleInputFocus();
    void OnRegisterClicked();
    void OnBackClicked();

    // ── State ─────────────────────────────────────────────────────────────────
    // typingField: 0 = email, 1 = password, 2 = confirmPassword
    int         typingField;
    std::string typedEmail;
    std::string typedPassword;
    std::string typedConfirmPassword;
    std::string errorMessage;

    // ── UI Elements ───────────────────────────────────────────────────────────
    Engine::Label*       emailPromptLabel;          // “Email:”
    Engine::Label*       emailInputLabel;           // typedEmail

    Engine::Label*       passwordPromptLabel;       // “Password:”
    Engine::Label*       passwordInputLabel;        // typedPassword (masked/revealed)

    Engine::Label*       confirmPromptLabel;        // “Confirm Password:”
    Engine::Label*       confirmInputLabel;         // typedConfirmPassword (masked/revealed)

    Engine::Label*       infoLabel;                 // shows errors (“EMAIL_EXISTS” or “Passwords do not match”)

    Engine::ImageButton* registerButton;            // “Register” button
    Engine::ImageButton* backButton;                // “Back” to Local/Online menu
    Engine::Label*       backLabel;                 // label over backButton

    // “Show/Hide password” and “Show/Hide confirm” eye icons
    // We will show/hide the same eye icon for whichever of the two password fields is focused.
    static bool            revealPassword;
    static ALLEGRO_BITMAP* openEyeBmp;
    static ALLEGRO_BITMAP* closeEyeBmp;
    static Engine::ImageButton* eyeButton;

    // Shared TTF font (loaded once)
    static ALLEGRO_FONT*   onlineFont;

    // Rectangle for email, password, and confirm boxes (for drawing focus outlines)
    int emailBoxX,    emailBoxY,    emailBoxW,    emailBoxH;
    int passwordBoxX, passwordBoxY, passwordBoxW, passwordBoxH;
    int confirmBoxX,  confirmBoxY,  confirmBoxW,  confirmBoxH;
};

#endif // REGISTER_ONLINE_SCENE_HPP
