#include "Account/RegisterScene.hpp"
#include "Account/AccountManager.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <cctype>
#include <algorithm>

using namespace Engine;

RegisterScene::RegisterScene()
    : typedUsername()
    , typedPassword()
    , typedConfirmPassword()
    , activeField(UsernameField)
    , usernamePromptLabel(nullptr)
    , usernameInputLabel(nullptr)
    , passwordPromptLabel(nullptr)
    , passwordInputLabel(nullptr)
    , confirmPromptLabel(nullptr)
    , confirmInputLabel(nullptr)
    , validationLabel(nullptr)
    , confirmButton(nullptr)
    , backButton(nullptr)
    , validationMessage()
    , usernameBoxX(0)
    , usernameBoxY(0)
    , usernameBoxW(0)
    , usernameBoxH(0)
    , passwordBoxX(0)
    , passwordBoxY(0)
    , passwordBoxW(0)
    , passwordBoxH(0)
    , confirmBoxX(0)
    , confirmBoxY(0)
    , confirmBoxW(0)
    , confirmBoxH(0)
{ }

RegisterScene::~RegisterScene() { }

void RegisterScene::Initialize() {
    typedUsername.clear();
    typedPassword.clear();
    typedConfirmPassword.clear();
    activeField = UsernameField;
    validationMessage.clear();

    int w = GameEngine::GetInstance().GetScreenSize().x;  // 1600
    int h = GameEngine::GetInstance().GetScreenSize().y;  // 832
    int halfW = w / 2;  // 800
    int halfH = h / 2;  // 416

    //
    // 1) Username prompt + input
    //

    // 1a) Prompt label at (halfW - 300, halfH - 150):
    usernamePromptLabel = new Label(
        "Username:",
        "balatro.ttf",
        60,
        halfW - 300,
        halfH - 150,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(usernamePromptLabel);

    // 1b) Clickable rectangle for Username:
    //     400x60 centered at (halfW + 100, halfH - 150):
    usernameBoxW = 500;
    usernameBoxH = 80;
    usernameBoxX = (halfW + 100) - (usernameBoxW / 2);
    usernameBoxY = (halfH - 150) - (usernameBoxH / 2);

    // 1c) Place the input‐Label 10px inside that box:
    usernameInputLabel = new Label(
        "",
        "balatro.ttf",
        60,
        usernameBoxX + 10,
        usernameBoxY + (usernameBoxH / 2),
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(usernameInputLabel);

    //
    // 2) Password prompt + input
    //

    // 2a) Prompt label at (halfW - 300, halfH - 50):
    passwordPromptLabel = new Label(
        "Password:",
        "balatro.ttf",
        60,
        halfW - 300,
        halfH - 50,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(passwordPromptLabel);

    // 2b) Rectangle for Password at center (halfW + 100, halfH - 50):
    passwordBoxW = 500;
    passwordBoxH = 80;
    passwordBoxX = (halfW + 100) - (passwordBoxW / 2);
    passwordBoxY = (halfH - 50) - (passwordBoxH / 2);

    // 2c) Input label inside that box, 10px in from left:
    passwordInputLabel = new Label(
        "",
        "balatro.ttf",
        60,
        passwordBoxX + 10,
        passwordBoxY + (passwordBoxH / 2),
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(passwordInputLabel);

    //
    // 3) Confirm Password prompt + input
    //

    // 3a) Prompt label at (halfW - 300, halfH + 50):
    confirmPromptLabel = new Label(
        "Confirm Password:",
        "balatro.ttf",
        60,
        halfW - 385,
        halfH + 50,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(confirmPromptLabel);

    // 3b) Rectangle for Confirm at center (halfW + 100, halfH + 50):
    confirmBoxW = 500;
    confirmBoxH = 80;
    confirmBoxX = (halfW + 100) - (confirmBoxW / 2);
    confirmBoxY = (halfH + 50) - (confirmBoxH / 2);

    // 3c) Input label 10px inside that box:
    confirmInputLabel = new Label(
        "",
        "balatro.ttf",
        60,
        confirmBoxX + 10,
        confirmBoxY + (confirmBoxH / 2),
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(confirmInputLabel);

    // Bounding box for Confirm
    confirmBoxW = 500;
    confirmBoxH = 80;
    confirmBoxX = (halfW + 100) - (confirmBoxW / 2);
    confirmBoxY = (halfH + 50) - (confirmBoxH / 2);

    //
    // 4) Validation label (red, 40px) under the confirm field
    //

    validationLabel = new Label(
        "",                     // initially empty
        "balatro.ttf",
        40,
        halfW,                  // center
        halfH + 130,            // 80 px below "Confirm Password" row
        255, 0, 0, 255,         // red
        0.5f, 0.5f
    );
    AddNewObject(validationLabel);

    //
    // 5) Confirm button (300×80), centered under validation label
    //

    confirmButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW - 150,            // 300px wide → left at halfW - 150
        halfH + 200,            // 70px below validation label
        300,                    // width
        80                      // height
    );
    confirmButton->SetOnClickCallback([this]() { OnConfirmClicked(); });
    AddNewControlObject(confirmButton);
    AddNewObject(new Label(
        "Confirm",
        "balatro.ttf",
        60,
        halfW,
        halfH + 200 + 40,       // center of the button's height
        0, 0, 0, 255,           // black
        0.5f, 0.5f
    ));

    //
    // 6) Back to Login button (to the right of Confirm)
    //

    backButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW + 200,            // 200px gap between the two buttons
        halfH + 200,
        300,
        80
    );
    backButton->SetOnClickCallback([this]() { OnBackClicked(); });
    AddNewControlObject(backButton);
    AddNewObject(new Label(
        "Back to Login",
        "balatro.ttf",
        60,
        halfW + 200 + 150,      // center of this button
        halfH + 200 + 40,
        0, 0, 0, 255,
        0.5f, 0.5f
    ));
}

void RegisterScene::Terminate() {
    IScene::Terminate();
}

void RegisterScene::Update(float dt) {
    IScene::Update(dt);
    validationLabel->Text = validationMessage;
}

void RegisterScene::Draw() const {
    IScene::Draw();

    // Highlight the active field with a 4px green border
    ALLEGRO_COLOR focusColor = al_map_rgb(0, 255, 0);
    if (activeField == UsernameField) {
        al_draw_rectangle(
            usernameBoxX, usernameBoxY,
            usernameBoxX + usernameBoxW, usernameBoxY + usernameBoxH,
            focusColor, 4.0f
        );
    }
    else if (activeField == PasswordField) {
        al_draw_rectangle(
            passwordBoxX, passwordBoxY,
            passwordBoxX + passwordBoxW, passwordBoxY + passwordBoxH,
            focusColor, 4.0f
        );
    }
    else {
        al_draw_rectangle(
            confirmBoxX, confirmBoxY,
            confirmBoxX + confirmBoxW, confirmBoxY + confirmBoxH,
            focusColor, 4.0f
        );
    }
}

void RegisterScene::OnKeyChar(int unicode) {
    // Enter or Tab cycles or “Confirm”
    if (unicode == '\r') {
        if (activeField == UsernameField)       ToggleInputFocus();
        else if (activeField == PasswordField)  ToggleInputFocus();
        else                                    OnConfirmClicked();
        return;
    }
    if (unicode == '\t') {
        ToggleInputFocus();
        return;
    }

    // Backspace
    if (unicode == '\b') {
        if (activeField == UsernameField && !typedUsername.empty())
            typedUsername.pop_back();
        else if (activeField == PasswordField && !typedPassword.empty())
            typedPassword.pop_back();
        else if (activeField == ConfirmField && !typedConfirmPassword.empty())
            typedConfirmPassword.pop_back();
    }
    // Printable ASCII
    else if (unicode >= 32 && unicode < 127) {
        char c = static_cast<char>(unicode);
        if (activeField == UsernameField) {
            if (typedUsername.size() < 12)
                typedUsername.push_back(c);
        }
        else if (activeField == PasswordField) {
            if (typedPassword.size() < 12)
                typedPassword.push_back(c);
        }
        else {
            if (typedConfirmPassword.size() < 12)
                typedConfirmPassword.push_back(c);
        }
    }

    usernameInputLabel->Text = typedUsername;
    passwordInputLabel->Text = std::string(typedPassword.size(), '*');
    confirmInputLabel->Text  = std::string(typedConfirmPassword.size(), '*');

    // Revalidate if typing in password or confirm
    if (activeField == PasswordField || activeField == ConfirmField) {
        validationMessage = CheckPasswordStrength(typedPassword);
        if (validationMessage.empty()) {
            if (!typedConfirmPassword.empty() &&
                typedPassword != typedConfirmPassword)
            {
                validationMessage = "Passwords do not match.";
            }
        }
    }
}

void RegisterScene::ToggleInputFocus() {
    if (activeField == UsernameField)       activeField = PasswordField;
    else if (activeField == PasswordField)  activeField = ConfirmField;
    else                                    activeField = UsernameField;
}

std::string RegisterScene::CheckPasswordStrength(const std::string& pwd) const {
    if (pwd.length() < 12)   return "Password must be at least 8 characters.";
    bool hasLower = std::any_of(pwd.begin(), pwd.end(), [](char c){ return std::islower(c); });
    bool hasUpper = std::any_of(pwd.begin(), pwd.end(), [](char c){ return std::isupper(c); });
    bool hasDigit = std::any_of(pwd.begin(), pwd.end(), [](char c){ return std::isdigit(c); });
    bool hasSpecial = std::any_of(pwd.begin(), pwd.end(), [](char c){
        return !(std::isalnum(static_cast<unsigned char>(c)));
    });
    if (!hasLower)   return "Password needs a lowercase letter.";
    if (!hasUpper)   return "Password needs an uppercase letter.";
    if (!hasDigit)   return "Password needs a digit.";
    if (!hasSpecial) return "Password needs a special character.";
    return "";
}

void RegisterScene::OnConfirmClicked() {
    if (typedUsername.empty() || typedPassword.empty() || typedConfirmPassword.empty()) {
        validationMessage = "All fields are required.";
        return;
    }
    if (typedPassword != typedConfirmPassword) {
        validationMessage = "Passwords do not match.";
        return;
    }
    validationMessage = CheckPasswordStrength(typedPassword);
    if (!validationMessage.empty()) return;
    if (AccountManager::UsernameExists(typedUsername)) {
        validationMessage = "Username already exists.";
        return;
    }
    AccountManager::RegisterNewAccount(typedUsername, typedPassword);
    GameEngine::GetInstance().ChangeScene("login");
}

void RegisterScene::OnBackClicked() {
    GameEngine::GetInstance().ChangeScene("login");
}

void RegisterScene::OnMouseDown(int button, int x, int y) {
    if (button == 1) {
        // Username field?
        if (x >= usernameBoxX && x <= usernameBoxX + usernameBoxW &&
            y >= usernameBoxY && y <= usernameBoxY + usernameBoxH)
        {
            activeField = UsernameField;
            return;
        }
        // Password field?
        if (x >= passwordBoxX && x <= passwordBoxX + passwordBoxW &&
            y >= passwordBoxY && y <= passwordBoxY + passwordBoxH)
        {
            activeField = PasswordField;
            return;
        }
        // Confirm field?
        if (x >= confirmBoxX && x <= confirmBoxX + confirmBoxW &&
            y >= confirmBoxY && y <= confirmBoxY + confirmBoxH)
        {
            activeField = ConfirmField;
            return;
        }
    }
    // Forward other clicks (e.g. on buttons)
    IScene::OnMouseDown(button, x, y);
}
