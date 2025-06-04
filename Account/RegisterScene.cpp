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

    int w = GameEngine::GetInstance().GetScreenSize().x;
    int h = GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // 1) Username prompt and input:
    usernamePromptLabel = new Label(
        "Username:", "pirulen.ttf", 24,
        halfW - 200, halfH - 100,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(usernamePromptLabel);

    usernameInputLabel = new Label(
        "", "pirulen.ttf", 24,
        halfW + 20, halfH - 100,
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(usernameInputLabel);

    // Bounding box for Username (200×30)
    usernameBoxW = 200;
    usernameBoxH = 30;
    usernameBoxX = (halfW + 20) - (usernameBoxW / 2);
    usernameBoxY = (halfH - 100) - (usernameBoxH / 2);

    // 2) Password prompt and input:
    passwordPromptLabel = new Label(
        "Password:", "pirulen.ttf", 24,
        halfW - 200, halfH - 50,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(passwordPromptLabel);

    passwordInputLabel = new Label(
        "", "pirulen.ttf", 24,
        halfW + 20, halfH - 50,
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(passwordInputLabel);

    // Bounding box for Password
    passwordBoxW = 200;
    passwordBoxH = 30;
    passwordBoxX = (halfW + 20) - (passwordBoxW / 2);
    passwordBoxY = (halfH - 50) - (passwordBoxH / 2);

    // 3) Confirm Password prompt and input:
    confirmPromptLabel = new Label(
        "Confirm Password:", "pirulen.ttf", 24,
        halfW - 200, halfH,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(confirmPromptLabel);

    confirmInputLabel = new Label(
        "", "pirulen.ttf", 24,
        halfW + 20, halfH,
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(confirmInputLabel);

    // Bounding box for Confirm
    confirmBoxW = 200;
    confirmBoxH = 30;
    confirmBoxX = (halfW + 20) - (confirmBoxW / 2);
    confirmBoxY = halfH - (confirmBoxH / 2);

    // 4) Validation label (red)
    validationLabel = new Label(
        "", "pirulen.ttf", 20,
        halfW, halfH + 50,
        255, 0, 0, 255,
        0.5f, 0.5f
    );
    AddNewObject(validationLabel);

    // 5) Confirm button
    confirmButton = new ImageButton(
        "stage-select/dirt.png", "stage-select/floor.png",
        halfW - 100, halfH + 100, 180, 50
    );
    confirmButton->SetOnClickCallback([this]() { OnConfirmClicked(); });
    AddNewControlObject(confirmButton);
    AddNewObject(new Label(
        "Confirm", "pirulen.ttf", 24,
        halfW - 100 + 90, halfH + 100 + 25,
        0, 0, 0, 255,
        0.5f, 0.5f
    ));

    // 6) Back to Login button
    backButton = new ImageButton(
        "stage-select/dirt.png", "stage-select/floor.png",
        halfW + 120, halfH + 100, 180, 50
    );
    backButton->SetOnClickCallback([this]() { OnBackClicked(); });
    AddNewControlObject(backButton);
    AddNewObject(new Label(
        "Back to Login", "pirulen.ttf", 24,
        halfW + 120 + 90, halfH + 100 + 25,
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

    // Optional: draw a green border around the active field
    ALLEGRO_COLOR focusColor = al_map_rgb(0,255,0);
    if (activeField == UsernameField) {
        al_draw_rectangle(
            usernameBoxX, usernameBoxY,
            usernameBoxX + usernameBoxW, usernameBoxY + usernameBoxH,
            focusColor, 2.0f
        );
    }
    else if (activeField == PasswordField) {
        al_draw_rectangle(
            passwordBoxX, passwordBoxY,
            passwordBoxX + passwordBoxW, passwordBoxY + passwordBoxH,
            focusColor, 2.0f
        );
    }
    else {
        al_draw_rectangle(
            confirmBoxX, confirmBoxY,
            confirmBoxX + confirmBoxW, confirmBoxY + confirmBoxH,
            focusColor, 2.0f
        );
    }
}

void RegisterScene::OnKeyChar(int unicode) {
    // If Enter, move focus or confirm:
    if (unicode == '\r') {
        if (activeField == UsernameField) {
            ToggleInputFocus();
        }
        else if (activeField == PasswordField) {
            ToggleInputFocus();
        }
        else {
            OnConfirmClicked();
        }
        return;
    }
    // Tab → next field
    if (unicode == '\t') {
        ToggleInputFocus();
        return;
    }
    // Backspace:
    if (unicode == '\b') {
        if (activeField == UsernameField && !typedUsername.empty()) {
            typedUsername.pop_back();
        }
        else if (activeField == PasswordField && !typedPassword.empty()) {
            typedPassword.pop_back();
        }
        else if (activeField == ConfirmField && !typedConfirmPassword.empty()) {
            typedConfirmPassword.pop_back();
        }
    }
    // Printable ASCII:
    else if (unicode >= 32 && unicode < 127) {
        char c = static_cast<char>(unicode);
        if (activeField == UsernameField) {
            if (typedUsername.size() < 12) {
                typedUsername.push_back(c);
            }
        }
        else if (activeField == PasswordField) {
            if (typedPassword.size() < 20) {
                typedPassword.push_back(c);
            }
        }
        else {
            if (typedConfirmPassword.size() < 20) {
                typedConfirmPassword.push_back(c);
            }
        }
    }
    usernameInputLabel->Text = typedUsername;
    passwordInputLabel->Text = std::string(typedPassword.size(), '*');
    confirmInputLabel ->Text = std::string(typedConfirmPassword.size(), '*');

    // Re‐validate as typing:
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
    if (pwd.length() < 8)   return "Password must be at least 8 characters.";
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

/**
 * This is the correct mouse‐down signature; GameEngine calls it when the user presses the mouse.
 * We simply check if the (x,y) click falls in one of our three 200×30 boxes and set focus accordingly.
 */
void RegisterScene::OnMouseDown(int button, int x, int y) {
    if (button == 1) {
        // Username field?
        if (x >= usernameBoxX && x <= usernameBoxX + usernameBoxW &&
            y >= usernameBoxY && y <= usernameBoxY + usernameBoxH)
        {
            activeField = UsernameField;
            return;  // consumed: do NOT call IScene::OnMouseDown
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

    // Otherwise, let any ImageButton receive the click:
    IScene::OnMouseDown(button, x, y);
}

