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

    int w = GameEngine::GetInstance().GetScreenSize().x;   // e.g. 1600
    int h = GameEngine::GetInstance().GetScreenSize().y;   // e.g. 832
    int halfW = w / 2;  // 800
    int halfH = h / 2;  // 416

    //
    // 1) Username prompt and input field
    //

    // 1a) Prompt label (still at halfW - 200, halfH - 100):
    usernamePromptLabel = new Label(
        "Username:",
        "balatro.ttf",  // or whatever TTF you’re using
        60,
        halfW - 200,
        halfH - 100,
        255, 255, 255, 255,
        0.5f, 0.5f    // centered on that point
    );
    AddNewObject(usernamePromptLabel);

    // 1b) Define the clickable rectangle for “Username”:
    //     400×60 centered at (halfW+200, halfH−100):
    usernameBoxW = 500;
    usernameBoxH = 80;
    usernameBoxX = (halfW + 200) - (usernameBoxW / 2);
    usernameBoxY = (halfH - 100) - (usernameBoxH / 2);

    // 1c) Place the input‐Label **inside** that rectangle, with a little left padding:
    //     X = usernameBoxX + 10  (10px inset)
    //     Y = usernameBoxY + (usernameBoxH/2)  (vertical center)
    usernameInputLabel = new Label(
        "",                     // initially empty
        "balatro.ttf",
        60,
        usernameBoxX + 15,      // start 10px into the box
        usernameBoxY + (usernameBoxH / 2),
        255, 255, 255, 255,
        0.0f, 0.5f              // left‐aligned, vertically centered
    );
    AddNewObject(usernameInputLabel);

    //
    // 2) Password prompt and input field
    //

    // 2a) Prompt label at (halfW - 200, halfH + 0):
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

    // 2b) Define rectangle for “Password” (also 400×60 at center (halfW+200, halfH)):
    passwordBoxW = 500;
    passwordBoxH = 80;
    passwordBoxX = (halfW + 200) - (passwordBoxW / 2);
    passwordBoxY = (halfH + 0) - (passwordBoxH / 2);

    // 2c) Place input‐Label inside that rectangle (with 10px left padding):
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

    // Bounding box for Password:
    passwordBoxW = 500;
    passwordBoxH = 80;
    passwordBoxX = (halfW + 200) - (passwordBoxW / 2);
    passwordBoxY = (halfH + 0) - (passwordBoxH / 2);

    //
    // 3) Info label (red) under fields for error messages (40px)
    //
    infoLabel = new Label(
        "",                     // no text initially
        "balatro.ttf",
        40,                     // slightly smaller, 40px for error
        halfW,                  // centered horizontally
        halfH + 100,            // 100px below vertical center
        255, 0, 0, 255,         // red color
        0.5f, 0.5f
    );
    AddNewObject(infoLabel);

    //
    // 4) Login button (centered under the error label)
    //
    loginButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW - 150,            // button width 300, so left = halfW - 150
        halfH + 180,            // 180px below center (i.e. 80px below error label)
        300,                    // width
        80                      // height
    );
    loginButton->SetOnClickCallback([this]() { OnLoginClicked(); });
    AddNewControlObject(loginButton);
    AddNewObject(new Label(
        "Login",
        "balatro.ttf",
        60,                     // match 60px so the button label is consistent
        halfW,                  // center of the button
        halfH + 180 + 40,       // halfway down the 80px height  (halfH+180 plus 40)
        0, 0, 0, 255,           // black text
        0.5f, 0.5f
    ));

    //
    // 5) Register button (to the right of the Login button)
    //
    registerButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW + 200,            // leaves a 50px gap between the two buttons
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
        halfW + 200 + 150,      // center of Register button (halfW+200 + half width 150)
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

    // Draw a border around whichever field is focused
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
    // Enter or Tab behavior remains the same
    if (unicode == '\r') {
        if (typingUsername) ToggleInputFocus();
        else                OnLoginClicked();
        return;
    }
    if (unicode == '\t') {
        ToggleInputFocus();
        return;
    }
    if (unicode == '\b') {
        if (typingUsername && !typedUsername.empty())
            typedUsername.pop_back();
        else if (!typingUsername && !typedPassword.empty())
            typedPassword.pop_back();
    }
    else if (unicode >= 32 && unicode < 127) {
        char c = static_cast<char>(unicode);
        if (typingUsername) {
            if (typedUsername.size() < 12)
                typedUsername.push_back(c);
        } else {
            if (typedPassword.size() < 12)
                typedPassword.push_back(c);
        }
    }

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

void LoginScene::OnMouseDown(int button, int x, int y) {
    if (button == 1) {  // only left click
        // If click inside Username box, focus Username
        if (x >= usernameBoxX && x <= usernameBoxX + usernameBoxW &&
            y >= usernameBoxY && y <= usernameBoxY + usernameBoxH)
        {
            typingUsername = true;
            return;  // consumed: do not pass on to buttons
        }
        // If click inside Password box, focus Password
        if (x >= passwordBoxX && x <= passwordBoxX + passwordBoxW &&
            y >= passwordBoxY && y <= passwordBoxY + passwordBoxH)
        {
            typingUsername = false;
            return;
        }
    }
    // Otherwise forward to base so buttons still get the event
    IScene::OnMouseDown(button, x, y);
}
