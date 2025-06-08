#ifndef LOGIN_ONLINE_SCENE_HPP
#define LOGIN_ONLINE_SCENE_HPP

#include "Engine/IScene.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include <string>
#include <allegro5/allegro.h>

// Forward‐declare Allegro types
struct ALLEGRO_FONT;
struct ALLEGRO_BITMAP;

namespace Engine {
    class ImageButton;
    class Label;
}

///
/// LoginOnlineScene.hpp
///
/// Presents an email/password login form that calls Firebase’s REST “signInWithPassword”
/// endpoint over HTTPS.  On success, it transitions to the “start” scene.  On failure,
/// it shows the returned error message in a red info label.
///
/// You must have placed `httplib.h` and `json.hpp` in your project root, and
/// your CMakeLists.txt must `find_package(OpenSSL REQUIRED)` and link to OpenSSL::SSL
/// and OpenSSL::Crypto so that `httplib::SSLClient` can do HTTPS.
///
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
    Engine::Label*        emailInputLabel;
    Engine::Label*        passwordPromptLabel;
    Engine::Label*        passwordInputLabel;
    Engine::Label*        infoLabel;

    Engine::ImageButton*  loginButton;
    Engine::ImageButton*  registerButton;
    Engine::ImageButton*  backButton;
    Engine::Label*        backLabel;

    // Eye icon for show/hide password
    static bool            loginRevealPasswordOnline;
    static ALLEGRO_BITMAP* openEyeBmpOnline;
    static ALLEGRO_BITMAP* closeEyeBmpOnline;
    static Engine::ImageButton* eyeButtonOnline;

    // Shared font
    static ALLEGRO_FONT*   onlineFont;

    // Input‐box rectangle coordinates (for drawing & mouse clicks)
    int emailBoxX, emailBoxY, emailBoxW, emailBoxH;
    int passwordBoxX, passwordBoxY, passwordBoxW, passwordBoxH;

    // Holds any error message from Firebase
    std::string errorMessage;
};

#endif // LOGIN_ONLINE_SCENE_HPP
