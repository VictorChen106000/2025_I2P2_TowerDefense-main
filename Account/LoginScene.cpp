// LoginScene.cpp

#include "Account/LoginScene.hpp"
#include "Account/AccountManager.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include "Scene/StartScene.h"
#include "UI/Animation/ParallaxBackground.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>
#include <stdexcept>
#include <cmath>

using namespace Engine;
extern std::string CurrentUser;

// ─── static helpers for text‐fields ─────────────────────────────────

static void handleTextInput(LoginScene::TextField& f,
                            int keycode, int unicode,
                            ALLEGRO_FONT* font, float padding)
{
    // 1) insert or delete
    if (unicode >= 32 && unicode < 127) {
        f.text.insert(f.caretIndex, 1, static_cast<char>(unicode));
        f.caretIndex++;
    }
    else if (keycode == ALLEGRO_KEY_BACKSPACE && f.caretIndex > 0) {
        f.text.erase(f.caretIndex - 1, 1);
        f.caretIndex--;
    }

    // 2) measure
    float visibleW = f.w - 2*padding;
    // caret position in pixels
    float caretPos = al_get_text_width(font,
        f.text.substr(0, f.caretIndex).c_str());
    // full text width
    float fullW = al_get_text_width(font, f.text.c_str());

    // 3) first, keep caret in view (same as before)
    if (caretPos < f.scrollX) {
        f.scrollX = caretPos;
    } else if (caretPos > f.scrollX + visibleW) {
        f.scrollX = caretPos - visibleW;
    }

    // 4) **new**: if you're typing at the very end, always scroll the tail fully into view
    if (f.caretIndex == f.text.size() && fullW > visibleW) {
        f.scrollX = fullW - visibleW;
    }

    // 5) clamp
    f.scrollX = std::max(0.0f, std::min(f.scrollX, fullW - visibleW));
}

static void drawTextField(const LoginScene::TextField& f,
                          ALLEGRO_FONT* font, float padding,
                          bool isPassword = false)
{
    // 1) clip
    al_set_clipping_rectangle(f.x, f.y, f.w, f.h);

    // 2) frame
    al_draw_rectangle(
      f.x, f.y, f.x + f.w, f.y + f.h,
      al_map_rgb(200,200,200), 2.0f
    );

    // 3) mask if needed
    std::string disp = isPassword
      ? std::string(f.text.size(), '*')
      : f.text;

    // 4) draw text shifted by scrollX
    al_draw_text(
      font,
      al_map_rgb(255,255,255),
      f.x + padding - f.scrollX,
      f.y + (f.h - al_get_font_line_height(font)) * 0.5f,
      0,
      disp.c_str()
    );

    // 5) blinking caret
    if (fmod(al_get_time(), 1.0) < 0.5) {
        float cp = al_get_text_width(font, disp.substr(0, f.caretIndex).c_str());
        float cx = f.x + padding - f.scrollX + cp;
        al_draw_line(cx, f.y + 2, cx, f.y + f.h - 2,
                     al_map_rgb(255,255,255), 1.0f);
    }

    // 6) reset clip
    al_reset_clipping_rectangle();
}

// ─── static members ────────────────────────────────────────────────
ALLEGRO_FONT*       LoginScene::loginFont            = nullptr;
bool                LoginScene::loginRevealPassword = false;
ALLEGRO_BITMAP*     LoginScene::openEyeBmp          = nullptr;
ALLEGRO_BITMAP*     LoginScene::closeEyeBmp         = nullptr;
Engine::ImageButton* LoginScene::eyeButton          = nullptr;

// ─── ctor / dtor ─────────────────────────────────────────────────

LoginScene::LoginScene()
  : typedUsername()
  , typedPassword()
  , typingUsername(true)
  , usernamePromptLabel(nullptr)
  , passwordPromptLabel(nullptr)
  , infoLabel(nullptr)
  , loginButton(nullptr)
  , registerButton(nullptr)
  , guestButton(nullptr)
  , backButton(nullptr)
  , backLabel(nullptr)
  , errorMessage()
{}

LoginScene::~LoginScene() {
    if (openEyeBmp)   { al_destroy_bitmap(openEyeBmp);   openEyeBmp   = nullptr; }
    if (closeEyeBmp)  { al_destroy_bitmap(closeEyeBmp);  closeEyeBmp  = nullptr; }
    if (loginFont)    { al_destroy_font(loginFont);      loginFont    = nullptr; }
}

// ─── Initialize ──────────────────────────────────────────────────

void LoginScene::Initialize() {
    parallax.Load({
      "Resource/images/background/wl5.png",
      "Resource/images/background/wl4.png",
      "Resource/images/background/wl3.png",
      "Resource/images/background/wl2.png",
      "Resource/images/background/wl1.png"
    });
    typedUsername.clear();
    typedPassword.clear();
    typingUsername = true;
    errorMessage.clear();
    loginRevealPassword = false;

    auto& eng = GameEngine::GetInstance();
    int w = eng.GetScreenSize().x;
    int h = eng.GetScreenSize().y;
    int halfW = w/2, halfH = h/2;

    // load font
    if (!loginFont) {
        loginFont = al_load_ttf_font("Resource/fonts/balatro.ttf", 60, 0);
        if (!loginFont)
            throw std::runtime_error("Failed to load balatro.ttf");
    }

    // Back button
    {
        int btnW=200, btnH=60, btnX=50, btnY=50;
        backButton = new ImageButton("stage-select/dirt.png","stage-select/floor.png",
                                     btnX,btnY,btnW,btnH);
        backButton->SetOnClickCallback([&](){ OnBackClicked(); });
        AddNewControlObject(backButton);

        backLabel = new Label("Back","balatro.ttf",60,
                              btnX+btnW/2,btnY+btnH/2,
                              255,255,255,255,0.5f,0.5f);
        AddNewObject(backLabel);
    }

    // Username prompt + field
    usernamePromptLabel = new Label("Username:","balatro.ttf",60,
                                    halfW-200,halfH-100,
                                    255,255,255,255,0.5f,0.5f);
    AddNewObject(usernamePromptLabel);

    usernameBoxW = 500;  usernameBoxH = 80;
    usernameBoxX = (halfW+200) - usernameBoxW/2;
    usernameBoxY = (halfH-100) - usernameBoxH/2;
    usernameField = { usernameBoxX, usernameBoxY,
                      usernameBoxW, usernameBoxH,
                      "", 0, 0 };

    // Password prompt + field
    passwordPromptLabel = new Label("Password:","balatro.ttf",60,
                                    halfW-200,halfH+0,
                                    255,255,255,255,0.5f,0.5f);
    AddNewObject(passwordPromptLabel);

    passwordBoxW = 500;  passwordBoxH = 80;
    passwordBoxX = (halfW+200) - passwordBoxW/2;
    passwordBoxY = (halfH+0 ) - passwordBoxH/2;
    passwordField = { passwordBoxX, passwordBoxY,
                      passwordBoxW, passwordBoxH,
                      "", 0, 0 };

    // eye icons
    if (!openEyeBmp) {
        openEyeBmp  = al_load_bitmap("Resource/images/openeyewhite.png");
        closeEyeBmp = al_load_bitmap("Resource/images/closeeyewhite.png");
    }
    {
        float eyeX = passwordBoxX + passwordBoxW + 20;
        float eyeY = passwordBoxY + passwordBoxH/2 - 60;
        eyeButton = new ImageButton("closeeyewhite.png","openeyewhite.png",
                                    eyeX, eyeY, 80, 130);
        eyeButton->Visible = false;
        eyeButton->SetOnClickCallback([&](){
            loginRevealPassword = !loginRevealPassword;
            if (loginRevealPassword)
                eyeButton->SetImage("openeyewhite.png","closeeyewhite.png");
            else
                eyeButton->SetImage("closeeyewhite.png","openeyewhite.png");
        });
        AddNewControlObject(eyeButton);
    }

    // Info label
    infoLabel = new Label("","balatro.ttf",40,
                          halfW,halfH+100,
                          255,0,0,255,0.5f,0.5f);
    AddNewObject(infoLabel);

    // Login / Register / Guest
    loginButton = new ImageButton("stage-select/dirt.png","stage-select/floor.png",
                                  halfW-150,halfH+180,300,80);
    loginButton->SetOnClickCallback([&](){ OnLoginClicked(); });
    AddNewControlObject(loginButton);
    AddNewObject(new Label("Login","balatro.ttf",60,
                           halfW,halfH+180+40,0,0,0,255,0.5f,0.5f));

    registerButton = new ImageButton("stage-select/dirt.png","stage-select/floor.png",
                                     halfW+200,halfH+180,300,80);
    registerButton->SetOnClickCallback([&](){ OnRegisterClicked(); });
    AddNewControlObject(registerButton);
    AddNewObject(new Label("Register","balatro.ttf",60,
                           halfW+200+150,halfH+180+40,0,0,0,255,0.5f,0.5f));

    guestButton = new ImageButton("stage-select/dirt.png","stage-select/floor.png",
                                  halfW-500,halfH+180,300,80);
    guestButton->SetOnClickCallback([&](){
        CurrentUser = "Guest";
        GameEngine::GetInstance().ChangeScene("start");
    });
    AddNewControlObject(guestButton);
    AddNewObject(new Label("Play as Guest","balatro.ttf",60,
                           halfW-500+150,halfH+180+40,0,0,0,255,0.5f,0.5f));
}

void LoginScene::Terminate() {
    parallax.Unload();
    IScene::Terminate();
}

void LoginScene::Update(float dt) {
    IScene::Update(dt);
    infoLabel->Text = errorMessage;
}

void LoginScene::Draw() const {
    auto& eng = GameEngine::GetInstance();
    int  w   = eng.GetScreenSize().x,
         h   = eng.GetScreenSize().y;
    double t = al_get_time();

    // 1) Draw parallax background
    parallax.Draw(w, h, t);

    // 2) Draw this scene’s buttons / sprites / UI
    Group::Draw();

    ALLEGRO_COLOR focusColor = typingUsername
      ? al_map_rgb(0,255,0)
      : al_map_rgb(255,0,0);

    if (typingUsername) {
        al_draw_rectangle(
            usernameBoxX, usernameBoxY,
            usernameBoxX + usernameBoxW, usernameBoxY + usernameBoxH,
            focusColor, 4.0f
        );
    } else {
        al_draw_rectangle(
            passwordBoxX, passwordBoxY,
            passwordBoxX + passwordBoxW, passwordBoxY + passwordBoxH,
            focusColor, 4.0f
        );
    }

    // draw both fields via our helper
    drawTextField(usernameField, loginFont, 5.0f, /*password=*/false);
    drawTextField(passwordField, loginFont, 5.0f,
                  /*password=*/!loginRevealPassword);
}

void LoginScene::OnKeyChar(int unicode) {
    // Enter/Tab logic
    if (unicode == '\r') {
        if (typingUsername) ToggleInputFocus();
        else OnLoginClicked();
        return;
    }
    if (unicode == '\t') {
        ToggleInputFocus();
        return;
    }

    // handle editing
    int kc = (unicode == '\b') ? ALLEGRO_KEY_BACKSPACE : 0;
    if (typingUsername)
        handleTextInput(usernameField, kc, unicode, loginFont, 5.0f);
    else
        handleTextInput(passwordField, kc, unicode, loginFont, 5.0f);

    // sync for login logic
    typedUsername = usernameField.text;
    typedPassword = passwordField.text;

    // eye‐icon visibility
    eyeButton->Visible = !passwordField.text.empty();
    if (!eyeButton->Visible) loginRevealPassword = false;

    if (eyeButton->Visible) {
        if (loginRevealPassword)
            eyeButton->SetImage("openeyewhite.png","closeeyewhite.png");
        else
            eyeButton->SetImage("closeeyewhite.png","openeyewhite.png");
    }
}

void LoginScene::ToggleInputFocus() {
    typingUsername = !typingUsername;
}

void LoginScene::OnLoginClicked() {
    if (typedUsername.empty() || typedPassword.empty()) {
        errorMessage = "Username and password required.";
        return;
    }
    if (AccountManager::VerifyPassword(typedUsername, typedPassword)) {
        CurrentUser = typedUsername;
        if (auto* s = dynamic_cast<StartScene*>(
              GameEngine::GetInstance().GetScene("start")))
        {
            s->SetPreviousScene("login");
        }
        GameEngine::GetInstance().ChangeScene("start");
    }
    else {
        errorMessage = "Invalid username or password.";
    }
}

void LoginScene::OnRegisterClicked() {
    GameEngine::GetInstance().ChangeScene("register");
}

void LoginScene::OnBackClicked() {
    GameEngine::GetInstance().ChangeScene("local-online");
}

void LoginScene::OnMouseDown(int button, int x, int y) {
    if (button == 1) {
        if (x >= usernameBoxX && x <= usernameBoxX + usernameBoxW &&
            y >= usernameBoxY && y <= usernameBoxY + usernameBoxH)
        {
            typingUsername = true; return;
        }
        if (x >= passwordBoxX && x <= passwordBoxX + passwordBoxW &&
            y >= passwordBoxY && y <= passwordBoxY + passwordBoxH)
        {
            typingUsername = false; return;
        }
    }
    IScene::OnMouseDown(button, x, y);
}
