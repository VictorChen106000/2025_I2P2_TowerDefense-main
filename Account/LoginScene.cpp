// LoginScene.cpp

#include "Account/LoginScene.hpp"
#include "Account/AccountManager.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>   // ensure the image addon is initialized
#include <allegro5/allegro_ttf.h>
#include <stdexcept>
#include <string>

using namespace Engine;
extern std::string CurrentUser;

// ─────────────────────────────────────────────────────────────
// FILE‐SCOPE STATICS FOR EYE ICONS & FONT
// ─────────────────────────────────────────────────────────────

// 1) 60px font (loaded once for Label/measurement)
static ALLEGRO_FONT* loginFont = nullptr;

// 2) Reveal‐password toggle
static bool loginRevealPassword = false;

// 3) Eye icon bitmaps (white “open”/“closed”)
static ALLEGRO_BITMAP* openEyeBmp  = nullptr;
static ALLEGRO_BITMAP* closeEyeBmp = nullptr;

// 4) The eye ImageButton itself
static ImageButton* eyeButton = nullptr;
// ─────────────────────────────────────────────────────────────

LoginScene::LoginScene()
    : typedUsername()
    , typedPassword()
    , typingUsername(true)
    , usernamePromptLabel(nullptr)
    , usernameInputLabel(nullptr)
    , passwordPromptLabel(nullptr)
    , passwordInputLabel(nullptr)
    , infoLabel(nullptr)
    , loginButton(nullptr)
    , registerButton(nullptr)
    , errorMessage()
    , usernameBoxX(0)
    , usernameBoxY(0)
    , usernameBoxW(0)
    , usernameBoxH(0)
    , passwordBoxX(0)
    , passwordBoxY(0)
    , passwordBoxW(0)
    , passwordBoxH(0)
{ }

LoginScene::~LoginScene() {
    // Destroy the eye bitmaps and TTF font to avoid leaks:
    if (openEyeBmp)   { al_destroy_bitmap(openEyeBmp);   openEyeBmp   = nullptr; }
    if (closeEyeBmp)  { al_destroy_bitmap(closeEyeBmp);  closeEyeBmp  = nullptr; }
    if (loginFont)    { al_destroy_font(loginFont);      loginFont    = nullptr; }
}

void LoginScene::Initialize() {
    typedUsername.clear();
    typedPassword.clear();
    typingUsername = true;
    errorMessage.clear();
    loginRevealPassword = false;

    int w = GameEngine::GetInstance().GetScreenSize().x;   // e.g. 1600
    int h = GameEngine::GetInstance().GetScreenSize().y;   // e.g. 832
    int halfW = w / 2;  // 800
    int halfH = h / 2;  // 416

    // ─── 1) Load 60px font once (for labels & measuring) ───
    if (!loginFont) {
        loginFont = al_load_ttf_font("Resource/fonts/balatro.ttf", 60, 0);
        if (!loginFont) {
            throw std::runtime_error("Failed to load Resource/fonts/balatro.ttf");
        }
    }

    // ─── 2) “Username:” prompt + input box ───────────────────────────────
    usernamePromptLabel = new Label(
        "Username:",
        "balatro.ttf",    // uses Resources::GetInstance().GetFont internally
        60,
        halfW - 200,
        halfH - 100,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(usernamePromptLabel);

    usernameBoxW = 500;   usernameBoxH = 80;
    usernameBoxX = (halfW + 200) - (usernameBoxW / 2);
    usernameBoxY = (halfH - 100) - (usernameBoxH / 2);

    usernameInputLabel = new Label(
        "",
        "balatro.ttf",
        60,
        usernameBoxX + 15,                    // 15px inset from left edge
        usernameBoxY + (usernameBoxH / 2),    // vertical center
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(usernameInputLabel);

    // ─── 3) “Password:” prompt + input box ───────────────────────────────
    passwordPromptLabel = new Label(
        "Password:",
        "balatro.ttf",
        60,
        halfW - 200,
        halfH + 0,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(passwordPromptLabel);

    passwordBoxW = 500;   passwordBoxH = 80;
    passwordBoxX = (halfW + 200) - (passwordBoxW / 2);
    passwordBoxY = (halfH + 0) - (passwordBoxH / 2);

    passwordInputLabel = new Label(
        "",
        "balatro.ttf",
        60,
        passwordBoxX + 15,
        passwordBoxY + (passwordBoxH / 2),
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(passwordInputLabel);

    // ─── 4) Load eye icons (white PNGs) once ─────────────────────────────
    if (!openEyeBmp) {
        openEyeBmp = al_load_bitmap("images/openeyewhite.png");
        if (!openEyeBmp) {
            throw std::runtime_error("Failed to load openeyewhite.png");
        }
    }
    if (!closeEyeBmp) {
        closeEyeBmp = al_load_bitmap("images/closeeyewhite.png");
        if (!closeEyeBmp) {
            throw std::runtime_error("Failed to load closeeyewhite.png");
        }
    }

    // ─── 5) Create the eye ImageButton to the right of the password box ──
    {
        float eyeX = passwordBoxX + passwordBoxW + 20;  
        float eyeY = passwordBoxY + (passwordBoxH / 2)-60; 

        eyeButton = new ImageButton(
            "closeeyewhite.png",   // “closed eye” as default
            "openeyewhite.png",    // “open eye” when pressed
            eyeX,
            eyeY,
            80, 130      // size of the eye icon (40×40)
        );
        // Hide the eye until the user types something:
        eyeButton->Visible = false;

        eyeButton->SetOnClickCallback([&]() {
            loginRevealPassword = !loginRevealPassword;
        
            if (loginRevealPassword) {
                // ─── “Revealed” state: out=open, in=close ───
                eyeButton->SetImage(
                    "openeyewhite.png",   // out image
                    "closeeyewhite.png"   // in (hover) image
                );
                passwordInputLabel->Text = typedPassword;
            } else {
                // ─── “Masked” state: out=close, in=open ───
                eyeButton->SetImage(
                    "closeeyewhite.png",  // out image
                    "openeyewhite.png"    // in (hover) image
                );
                passwordInputLabel->Text = std::string(typedPassword.size(), '*');
            }
        });

        AddNewControlObject(eyeButton);
    }

    // ─── 6) Info label (red) under the fields ───────────────────────────
    infoLabel = new Label(
        "",
        "balatro.ttf",
        40,
        halfW,
        halfH + 100,
        255, 0, 0, 255,
        0.5f, 0.5f
    );
    AddNewObject(infoLabel);

    // ─── 7) Login + Register buttons (unchanged) ───────────────────────
    loginButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW - 150,
        halfH + 180,
        300,
        80
    );
    loginButton->SetOnClickCallback([this]() { OnLoginClicked(); });
    AddNewControlObject(loginButton);
    AddNewObject(new Label(
        "Login",
        "balatro.ttf",
        60,
        halfW,
        halfH + 180 + 40,
        0, 0, 0, 255,
        0.5f, 0.5f
    ));

    registerButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW + 200,
        halfH + 180,
        300,
        80
    );
    registerButton->SetOnClickCallback([this]() { OnRegisterClicked(); });
    AddNewControlObject(registerButton);
    AddNewObject(new Label(
        "Register",
        "balatro.ttf",
        60,
        halfW + 200 + 150,
        halfH + 180 + 40,
        0, 0, 0, 255,
        0.5f, 0.5f
    ));
}

void LoginScene::Terminate() {
    IScene::Terminate();
}

void LoginScene::Update(float dt) {
    IScene::Update(dt);
    infoLabel->Text = errorMessage;
}

void LoginScene::Draw() const {
    IScene::Draw();
    // Draw a highlight rectangle around focused field
    ALLEGRO_COLOR focusColor = typingUsername ? al_map_rgb(0,255,0)
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
}

void LoginScene::OnKeyChar(int unicode) {
    // Handle Enter/Tab same as before:
    if (unicode == '\r') {
        if (typingUsername) ToggleInputFocus();
        else                OnLoginClicked();
        return;
    }
    if (unicode == '\t') {
        ToggleInputFocus();
        return;
    }

    // Backspace:
    if (unicode == '\b') {
        if (typingUsername && !typedUsername.empty()) {
            typedUsername.pop_back();
        } else if (!typingUsername && !typedPassword.empty()) {
            typedPassword.pop_back();
        }
    }
    // Printable ASCII:
    else if (unicode >= 32 && unicode < 127) {
        char c = static_cast<char>(unicode);
        if (typingUsername) {
            if (typedUsername.size() < 12) {
                typedUsername.push_back(c);
            }
        } else {
            if (typedPassword.size() < 12) {
                typedPassword.push_back(c);
            }
        }
    }

    // 1) Update username label text:
    usernameInputLabel->Text = typedUsername;

    // 2) Show or hide the eye based on typedPassword:
    if (!typedPassword.empty()) {
        // If password has at least one char, show the eye:
        if (!eyeButton->Visible) {
            eyeButton->Visible = true;
            // Ensure it starts in “closed” state:
            loginRevealPassword = false;
            eyeButton->SetImage("closeeyewhite.png",
                                "openeyewhite.png");
        }
    } else {
        // If password is now empty, hide the eye and reset reveal:
        if (eyeButton->Visible) {
            eyeButton->Visible = false;
            loginRevealPassword = false;
        }
    }

    // 3) Update password label based on reveal flag:
    if (loginRevealPassword) {
        passwordInputLabel->Text = typedPassword;
    } else {
        passwordInputLabel->Text = std::string(typedPassword.size(), '*');
        // Keep the eye icon “closed” if masking is on:
        if (eyeButton->Visible) {
            eyeButton->SetImage("closeeyewhite.png",
                                "openeyewhite.png");
        }
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
        GameEngine::GetInstance().ChangeScene("start");
    } else {
        errorMessage = "Invalid username or password.";
    }
}

void LoginScene::OnRegisterClicked() {
    GameEngine::GetInstance().ChangeScene("register");
}

void LoginScene::OnMouseDown(int button, int x, int y) {
    if (button == 1) { // left click
        if (x >= usernameBoxX && x <= usernameBoxX + usernameBoxW &&
            y >= usernameBoxY && y <= usernameBoxY + usernameBoxH)
        {
            typingUsername = true;
            return;
        }
        if (x >= passwordBoxX && x <= passwordBoxX + passwordBoxW &&
            y >= passwordBoxY && y <= passwordBoxY + passwordBoxH)
        {
            typingUsername = false;
            return;
        }
    }
    IScene::OnMouseDown(button, x, y);
}
