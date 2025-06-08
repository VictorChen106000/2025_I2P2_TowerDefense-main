// RegisterScene.cpp

#include "Account/RegisterScene.hpp"
#include "Account/AccountManager.hpp"
#include "Engine/GameEngine.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/ImageButton.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>   // ensure image addon is initialized
#include <allegro5/allegro_ttf.h>
#include <cctype>
#include <algorithm>
#include <stdexcept>

using namespace Engine;

// ────────────────────────────────────────────────────────────────────────
// FILE‐SCOPE STATICS FOR EYE ICONS & FONT
// ────────────────────────────────────────────────────────────────────────

// Font for measuring/drawing at 60px
static ALLEGRO_FONT* registerFont = nullptr;

// Reveal‐password toggles
static bool revealPassword = false;
static bool revealConfirm  = false;

// Eye icon bitmaps (white “eye open” and “eye closed”)
static ALLEGRO_BITMAP* openEyeBmp     = nullptr;
static ALLEGRO_BITMAP* closeEyeBmp    = nullptr;

// The two eye ImageButtons
static ImageButton* passwordEyeButton = nullptr;
static ImageButton* confirmEyeButton  = nullptr;
// ────────────────────────────────────────────────────────────────────────

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

RegisterScene::~RegisterScene() {
    // Destroy eye bitmaps and font to free GPU memory
    if (openEyeBmp)     { al_destroy_bitmap(openEyeBmp);     openEyeBmp = nullptr; }
    if (closeEyeBmp)    { al_destroy_bitmap(closeEyeBmp);    closeEyeBmp = nullptr; }
    if (registerFont)   { al_destroy_font(registerFont);     registerFont = nullptr; }
}

void RegisterScene::Initialize() {
    typedUsername.clear();
    typedPassword.clear();
    typedConfirmPassword.clear();
    activeField = UsernameField;
    validationMessage.clear();
    revealPassword = false;
    revealConfirm  = false;

    int w = GameEngine::GetInstance().GetScreenSize().x;  // e.g. 1600
    int h = GameEngine::GetInstance().GetScreenSize().y;  // e.g. 832
    int halfW = w / 2;  // 800
    int halfH = h / 2;  // 416

    // ─── 1) Username prompt + input ─────────────────────────────────

    // 1a) “Username:” prompt label at (halfW - 300, halfH - 150)
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

    // 1b) Rectangle for Username: 500×80 centered at (halfW + 100, halfH - 150)
    usernameBoxW = 500;
    usernameBoxH = 80;
    usernameBoxX = (halfW + 100) - (usernameBoxW / 2);
    usernameBoxY = (halfH - 150) - (usernameBoxH / 2);

    // 1c) Input label inside Username box, 10px inset
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

    // ─── 2) Password prompt + input ─────────────────────────────────

    // 2a) “Password:” prompt label at (halfW - 300, halfH - 50)
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

    // 2b) Rectangle for Password: 500×80 centered at (halfW + 100, halfH - 50)
    passwordBoxW = 500;
    passwordBoxH = 80;
    passwordBoxX = (halfW + 100) - (passwordBoxW / 2);
    passwordBoxY = (halfH - 50) - (passwordBoxH / 2);

    // 2c) Input label inside Password box, 10px inset
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

    // ─── 3) Confirm Password prompt + input ───────────────────────────

    // 3a) “Confirm Password:” prompt label at (halfW - 385, halfH + 50)
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

    // 3b) Rectangle for Confirm Password: 500×80 centered at (halfW + 100, halfH + 50)
    confirmBoxW = 500;
    confirmBoxH = 80;
    confirmBoxX = (halfW + 100) - (confirmBoxW / 2);
    confirmBoxY = (halfH + 50) - (confirmBoxH / 2);

    // 3c) Input label inside Confirm box, 10px inset
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

    // ─── 4) Validation label under Confirm ─────────────────────────────

    validationLabel = new Label(
        "",
        "balatro.ttf",
        40,
        halfW,
        halfH + 130,
        255, 0, 0, 255,
        0.5f, 0.5f
    );
    AddNewObject(validationLabel);

    // ─── 5) Confirm + Back buttons ────────────────────────────────────

    confirmButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW - 150,
        halfH + 200,
        300,
        80
    );
    confirmButton->SetOnClickCallback([this]() { OnConfirmClicked(); });
    AddNewControlObject(confirmButton);
    AddNewObject(new Label(
        "Confirm",
        "balatro.ttf",
        60,
        halfW,
        halfH + 200 + 40,
        0, 0, 0, 255,
        0.5f, 0.5f
    ));

    backButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW + 200,
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
        halfW + 200 + 150,
        halfH + 200 + 40,
        0, 0, 0, 255,
        0.5f, 0.5f
    ));

    // ─── 6) Load 60px font once (for measuring) ───────────────────────
    if (!registerFont) {
        registerFont = al_load_ttf_font("Resource/fonts/balatro.ttf", 60, 0);
        if (!registerFont) {
            throw std::runtime_error("Failed to load Resource/fonts/balatro.ttf");
        }
    }

    // ─── 7) Load eye icons once ────────────────────────────────────────
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

    // ─── 8) Create password‐eye button ─────────────────────────────────
    {
        int eyeW = 60, eyeH = 130; // adjust size as desired
        float eyeX = passwordBoxX + passwordBoxW + 20;
        float eyeY = passwordBoxY + (passwordBoxH / 2.0f) - (eyeH / 2.0f);

        passwordEyeButton = new ImageButton(
            "closeeyewhite.png",  // out: closed‐eye
            "openeyewhite.png",   // in:  open‐eye on hover
            eyeX,
            eyeY,
            eyeW, eyeH
        );
        passwordEyeButton->Visible = false;
        passwordEyeButton->SetOnClickCallback([&]() {
            revealPassword = !revealPassword;
            if (revealPassword) {
                // Reveal password, swap out/in to open‐eye
                passwordEyeButton->SetImage(
                    "openeyewhite.png",
                    "closeeyewhite.png"
                );
                passwordInputLabel->Text = typedPassword;
            } else {
                // Mask password, swap back to closed‐eye
                passwordEyeButton->SetImage(
                    "closeeyewhite.png",
                    "openeyewhite.png"
                );
                passwordInputLabel->Text = std::string(typedPassword.size(), '*');
            }
        });
        AddNewControlObject(passwordEyeButton);
    }

    // ─── 9) Create confirm‐eye button ──────────────────────────────────
    {
        int eyeW = 60, eyeH = 130; // same size for confirm eye
        float eyeX = confirmBoxX + confirmBoxW + 20;
        float eyeY = confirmBoxY + (confirmBoxH / 2.0f) - (eyeH / 2.0f);

        confirmEyeButton = new ImageButton(
            "closeeyewhite.png",  // out: closed‐eye
            "openeyewhite.png",   // in:  open‐eye on hover
            eyeX,
            eyeY,
            eyeW, eyeH
        );
        confirmEyeButton->Visible = false;
        confirmEyeButton->SetOnClickCallback([&]() {
            revealConfirm = !revealConfirm;
            if (revealConfirm) {
                // Reveal confirm‐password, swap to open‐eye
                confirmEyeButton->SetImage(
                    "openeyewhite.png",
                    "closeeyewhite.png"
                );
                confirmInputLabel->Text = typedConfirmPassword;
            } else {
                // Mask confirm‐password, swap to closed‐eye
                confirmEyeButton->SetImage(
                    "closeeyewhite.png",
                    "openeyewhite.png"
                );
                confirmInputLabel->Text = std::string(typedConfirmPassword.size(), '*');
            }
        });
        AddNewControlObject(confirmEyeButton);
    }
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

    if (unicode == '\b') {
        if (activeField == UsernameField && !typedUsername.empty())
            typedUsername.pop_back();
        else if (activeField == PasswordField && !typedPassword.empty())
            typedPassword.pop_back();
        else if (activeField == ConfirmField && !typedConfirmPassword.empty())
            typedConfirmPassword.pop_back();
    }
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

    // 1) Update Username label
    usernameInputLabel->Text = typedUsername;

    // 2) Update Password label & eye visibility
    // ── new show/hide code for password eye (purely based on typedPassword) ──

if (!typedPassword.empty()) {
    // If there is at least one character, ensure the eye is visible
    if (!passwordEyeButton->Visible) {
        passwordEyeButton->Visible = true;
        // Default to “masked” state (closed-eye) whenever it re-appears
        revealPassword = false;
        passwordEyeButton->SetImage(
            "closeeyewhite.png",  // out = closed-eye
            "openeyewhite.png"    // in  = open-eye on hover
        );
    }
} else {
    // If password is completely empty, hide the eye
    if (passwordEyeButton->Visible) {
        passwordEyeButton->Visible = false;
        revealPassword = false;
    }
}

if (revealPassword) {
    // Show actual characters
    passwordInputLabel->Text = typedPassword;
    // And swap the hover so that in=close-eye, out=open-eye
    passwordEyeButton->SetImage(
        "openeyewhite.png",   // out = open-eye
        "closeeyewhite.png"   // in  = closed-eye on hover
    );
} else {
    // Show asterisks
    passwordInputLabel->Text = std::string(typedPassword.size(), '*');
    // Make sure the eyeclick appearance is “masked”
    if (passwordEyeButton->Visible) {
        passwordEyeButton->SetImage(
            "closeeyewhite.png",  // out = closed-eye
            "openeyewhite.png"    // in  = open-eye on hover
        );
    }
}

// ── new show/hide code for confirm eye (purely based on typedConfirmPassword) ──

if (!typedConfirmPassword.empty()) {
    if (!confirmEyeButton->Visible) {
        confirmEyeButton->Visible = true;
        revealConfirm = false;
        confirmEyeButton->SetImage(
            "closeeyewhite.png",  // out = closed-eye
            "openeyewhite.png"    // in  = open-eye on hover
        );
    }
} else {
    if (confirmEyeButton->Visible) {
        confirmEyeButton->Visible = false;
        revealConfirm = false;
    }
}

if (revealConfirm) {
    confirmInputLabel->Text = typedConfirmPassword;
    confirmEyeButton->SetImage(
        "openeyewhite.png",   // out = open-eye
        "closeeyewhite.png"   // in  = closed-eye on hover
    );
} else {
    confirmInputLabel->Text = std::string(typedConfirmPassword.size(), '*');
    if (confirmEyeButton->Visible) {
        confirmEyeButton->SetImage(
            "closeeyewhite.png",  // out = closed-eye
            "openeyewhite.png"    // in  = open-eye on hover
        );
    }
}


    // 4) Revalidate password strength
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
    IScene::OnMouseDown(button, x, y);
}


