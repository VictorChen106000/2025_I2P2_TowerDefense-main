#include "Account/LoginScene.hpp"
#include "Account/AccountManager.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

using namespace Engine;
extern std::string CurrentUser;

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

LoginScene::~LoginScene() { }

void LoginScene::Initialize() {
    typedUsername.clear();
    typedPassword.clear();
    typingUsername = true;
    errorMessage.clear();

    int w = GameEngine::GetInstance().GetScreenSize().x;
    int h = GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // 1) Username prompt and input label:
    usernamePromptLabel = new Label(
        "Username:", "pirulen.ttf", 24,
        halfW - 150, halfH - 40,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(usernamePromptLabel);

    usernameInputLabel = new Label(
        "", "pirulen.ttf", 24,
        halfW + 20, halfH - 40,
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(usernameInputLabel);

    // Define the “clickable” rectangle around the Username field (200×30)
    usernameBoxW = 200;
    usernameBoxH = 30;
    // Center that box at (halfW + 20, halfH - 40):
    usernameBoxX = (halfW + 20) - (usernameBoxW / 2);
    usernameBoxY = (halfH - 40) - (usernameBoxH / 2);

    // 2) Password prompt and input label:
    passwordPromptLabel = new Label(
        "Password:", "pirulen.ttf", 24,
        halfW - 150, halfH,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(passwordPromptLabel);

    passwordInputLabel = new Label(
        "", "pirulen.ttf", 24,
        halfW + 20, halfH,
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(passwordInputLabel);

    // Bounding box for Password (same 200×30 size):
    passwordBoxW = 200;
    passwordBoxH = 30;
    passwordBoxX = (halfW + 20) - (passwordBoxW / 2);
    passwordBoxY = halfH - (passwordBoxH / 2);

    // 3) Info label (red) under fields for error messages:
    infoLabel = new Label(
        "", "pirulen.ttf", 20,
        halfW, halfH + 60,
        255, 0, 0, 255,
        0.5f, 0.5f
    );
    AddNewObject(infoLabel);

    // 4) Login button:
    loginButton = new ImageButton(
        "stage-select/dirt.png",  // up‐state texture
        "stage-select/floor.png", // down‐state texture
        halfW - 200,
        halfH + 100,
        180,
        50
    );
    loginButton->SetOnClickCallback([this]() { OnLoginClicked(); });
    AddNewControlObject(loginButton);
    AddNewObject(new Label(
        "Login", "pirulen.ttf", 24,
        halfW - 200 + 90, halfH + 100 + 25,
        0, 0, 0, 255,
        0.5f, 0.5f
    ));

    // 5) Register button:
    registerButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW + 20,
        halfH + 100,
        180,
        50
    );
    registerButton->SetOnClickCallback([this]() { OnRegisterClicked(); });
    AddNewControlObject(registerButton);
    AddNewObject(new Label(
        "Register", "pirulen.ttf", 24,
        halfW + 20 + 90, halfH + 100 + 25,
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

    // Optional: draw a colored rectangle around whichever field is focused
    ALLEGRO_COLOR focusColor = typingUsername ? al_map_rgb(0, 255, 0)
                                              : al_map_rgb(255, 0, 0);

    if (typingUsername) {
        al_draw_rectangle(
            usernameBoxX, usernameBoxY,
            usernameBoxX + usernameBoxW, usernameBoxY + usernameBoxH,
            focusColor, 2.0f
        );
    } else {
        al_draw_rectangle(
            passwordBoxX, passwordBoxY,
            passwordBoxX + passwordBoxW, passwordBoxY + passwordBoxH,
            focusColor, 2.0f
        );
    }
}

void LoginScene::OnKeyChar(int unicode) {
    // Enter → either move focus or attempt login
    if (unicode == '\r') {
        if (typingUsername) {
            ToggleInputFocus();
        } else {
            OnLoginClicked();
        }
        return;
    }
    // Tab → switch focus
    if (unicode == '\t') {
        ToggleInputFocus();
        return;
    }
    // Backspace
    if (unicode == '\b') {
        if (typingUsername && !typedUsername.empty()) {
            typedUsername.pop_back();
        } else if (!typingUsername && !typedPassword.empty()) {
            typedPassword.pop_back();
        }
    }
    // Printable ASCII
    else if (unicode >= 32 && unicode < 127) {
        char c = static_cast<char>(unicode);
        if (typingUsername) {
            if (typedUsername.size() < 12) {  // enforce max 12 letters
                typedUsername.push_back(c);
            }
        } else {
            if (typedPassword.size() < 20) {
                typedPassword.push_back(c);
            }
        }
    }
    // Update labels
    usernameInputLabel->Text = typedUsername;
    passwordInputLabel->Text = std::string(typedPassword.size(), '*');
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
        GameEngine::GetInstance().ChangeScene("stage-select");
    } else {
        errorMessage = "Invalid username or password.";
    }
}

void LoginScene::OnRegisterClicked() {
    GameEngine::GetInstance().ChangeScene("register");
}

/**
 * This is the correct method name to override (called by GameEngine when mouse is pressed).
 * We check whether the click (x,y) lies inside the username or password “box” and set focus accordingly.
 */
void LoginScene::OnMouseDown(int button, int x, int y) {
    // 1) If the click falls inside your Username or Password box, handle focus:
    if (button == 1) {
        if (x >= usernameBoxX && x <= usernameBoxX + usernameBoxW &&
            y >= usernameBoxY && y <= usernameBoxY + usernameBoxH)
        {
            typingUsername = true;
            return;   // we handled it—do NOT forward to buttons
        }
        if (x >= passwordBoxX && x <= passwordBoxX + passwordBoxW &&
            y >= passwordBoxY && y <= passwordBoxY + passwordBoxH)
        {
            typingUsername = false;
            return;   // we handled it—do NOT forward to buttons
        }
    }
    // 2) Otherwise (click is either a right‐click or falls outside both fields),
    //    call the base implementation so that any ImageButton will receive it:
    IScene::OnMouseDown(button, x, y);
}
