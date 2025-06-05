#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "Account/RegisterOnlineScene.hpp"
#include "Engine/GameEngine.hpp"
#include "httplib.h"    // https://github.com/yhirose/cpp-httplib
#include "json.hpp"     // https://github.com/nlohmann/json

using namespace Engine;
using json = nlohmann::json;

//─────────────────────────────────────────────────────────────────────────────
// Static data definitions (must match the header)
//─────────────────────────────────────────────────────────────────────────────
bool            RegisterOnlineScene::revealPassword        = false;
ALLEGRO_BITMAP* RegisterOnlineScene::openEyeBmp            = nullptr;
ALLEGRO_BITMAP* RegisterOnlineScene::closeEyeBmp           = nullptr;
ImageButton*    RegisterOnlineScene::eyeButton             = nullptr;
ALLEGRO_FONT*   RegisterOnlineScene::onlineFont            = nullptr;

// ── Replace this with your actual Firebase Web API Key ──────────────────────
static constexpr const char* FIREBASE_WEB_API_KEY = "AIzaSyAarObWywZJ_rQ2AlXDd6czNdnRqqSTRbo";
// ────────────────────────────────────────────────────────────────────────────


//─────────────────────────────────────────────────────────────────────────────
// Helper: PerformFirebaseRegister
//
// Calls Firebase’s signUp endpoint to create a new user. Returns true on success,
// or false plus an error‐message (in outError) on failure.
//
// POST https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=[API_KEY]
// Body (JSON):
// {
//   "email": "<USER_EMAIL>",
//   "password": "<USER_PASSWORD>",
//   "returnSecureToken": true
// }
//─────────────────────────────────────────────────────────────────────────────
static bool PerformFirebaseRegister(const std::string& email,
                                    const std::string& password,
                                    std::string& outError)
{
    // 1) Create an SSLClient to identitytoolkit.googleapis.com:443
    httplib::SSLClient client("identitytoolkit.googleapis.com", 443);
    client.set_connection_timeout(5, 0); // 5 seconds timeout

    // 2) Build the signUp path with your Firebase Web API key
    std::string path = "/v1/accounts:signUp?key=" + std::string(FIREBASE_WEB_API_KEY);

    // 3) Construct the JSON body
    json body = {
        {"email",             email},
        {"password",          password},
        {"returnSecureToken", true}
    };

    // 4) POST the JSON to Firebase
    auto res = client.Post(path.c_str(),
                           body.dump(),
                           "application/json");
    if (!res) {
        outError = "Network error: no response from Firebase.";
        return false;
    }

    // 5) If HTTP status != 200, parse out “error.message”
    if (res->status != 200) {
        try {
            auto errJson = json::parse(res->body);
            if (errJson.contains("error") && errJson["error"].contains("message")) {
                outError = errJson["error"]["message"].get<std::string>();
            } else {
                outError = "Unknown server error.";
            }
        }
        catch(...) {
            outError = "Malformed error response.";
        }
        return false;
    }

    // 6) On HTTP 200, parse for “idToken” (optional check)
    try {
        auto successJson = json::parse(res->body);
        if (successJson.contains("idToken")) {
            return true;
        } else {
            outError = "Registration succeeded but no idToken returned.";
            return false;
        }
    }
    catch(...) {
        outError = "Malformed success response.";
        return false;
    }
}


//─────────────────────────────────────────────────────────────────────────────
// RegisterOnlineScene Implementation
//─────────────────────────────────────────────────────────────────────────────

RegisterOnlineScene::RegisterOnlineScene()
    : typingField(0)
    , typedEmail()
    , typedPassword()
    , typedConfirmPassword()
    , errorMessage()
    , emailPromptLabel(nullptr)
    , emailInputLabel(nullptr)
    , passwordPromptLabel(nullptr)
    , passwordInputLabel(nullptr)
    , confirmPromptLabel(nullptr)
    , confirmInputLabel(nullptr)
    , infoLabel(nullptr)
    , registerButton(nullptr)
    , backButton(nullptr)
    , backLabel(nullptr)
    , emailBoxX(0),    emailBoxY(0),    emailBoxW(0),    emailBoxH(0)
    , passwordBoxX(0), passwordBoxY(0), passwordBoxW(0), passwordBoxH(0)
    , confirmBoxX(0),  confirmBoxY(0),  confirmBoxW(0),  confirmBoxH(0)
{
}

RegisterOnlineScene::~RegisterOnlineScene()
{
    // Destroy static bitmaps & font exactly once
    if (openEyeBmp)   { al_destroy_bitmap(openEyeBmp);     openEyeBmp   = nullptr; }
    if (closeEyeBmp)  { al_destroy_bitmap(closeEyeBmp);    closeEyeBmp  = nullptr; }
    if (onlineFont)   { al_destroy_font(onlineFont);       onlineFont   = nullptr; }
}

void RegisterOnlineScene::Initialize()
{
    // Reset all typed strings and state
    typedEmail.clear();
    typedPassword.clear();
    typedConfirmPassword.clear();
    typingField = 0;               // starting focus: 0 = email
    revealPassword = false;
    errorMessage.clear();

    // Load shared font if not already loaded
    if (!onlineFont) {
        onlineFont = al_load_font("Resource/fonts/balatro.ttf", 60, 0);
        if (!onlineFont) {
            throw std::runtime_error("Failed to load Resource/fonts/balatro.ttf");
        }
    }

    int w     = GameEngine::GetInstance().GetScreenSize().x;
    int h     = GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // ─── Back Button ───────────────────────────────────────────────────────
    {
        int btnW = 200, btnH = 60, btnX = 50, btnY = 50;
        backButton = new ImageButton(
            "stage-select/dirt.png",
            "stage-select/floor.png",
            btnX, btnY, btnW, btnH
        );
        backButton->SetOnClickCallback([&]() { OnBackClicked(); });
        AddNewControlObject(backButton);

        backLabel = new Label(
            "Back",
            "balatro.ttf", 36,
            btnX + (btnW / 2), btnY + (btnH / 2) + 5,
            255, 255, 255, 255,
            0.5f, 0.5f
        );
        AddNewObject(backLabel);
    }

    // ─── “Email:” Prompt & Input Box ─────────────────────────────────────────
    {
        emailPromptLabel = new Label(
            "Email:",
            "balatro.ttf", 60,
            halfW - 200, halfH - 180,   // positioned above others
            255, 255, 255, 255,
            0.5f, 0.5f
        );
        AddNewObject(emailPromptLabel);

        emailBoxW = 600; emailBoxH = 80;
        emailBoxX = (halfW + 200) - (emailBoxW / 2);
        emailBoxY = (halfH - 180) - (emailBoxH / 2);

        emailInputLabel = new Label(
            "",   // starts empty
            "balatro.ttf", 60,
            emailBoxX + 15, emailBoxY + (emailBoxH / 2),
            255, 255, 255, 255,
            0.0f, 0.5f   // left‐aligned
        );
        AddNewObject(emailInputLabel);
    }

    // ─── “Password:” Prompt & Input Box ───────────────────────────────────────
    {
        passwordPromptLabel = new Label(
            "Password:",
            "balatro.ttf", 60,
            halfW - 200, halfH - 60,
            255, 255, 255, 255,
            0.5f, 0.5f
        );
        AddNewObject(passwordPromptLabel);

        passwordBoxW = 600; passwordBoxH = 80;
        passwordBoxX = (halfW + 200) - (passwordBoxW / 2);
        passwordBoxY = (halfH - 60) - (passwordBoxH / 2);

        passwordInputLabel = new Label(
            "",   // starts empty
            "balatro.ttf", 60,
            passwordBoxX + 15, passwordBoxY + (passwordBoxH / 2),
            255, 255, 255, 255,
            0.0f, 0.5f
        );
        AddNewObject(passwordInputLabel);
    }

    // ─── “Confirm Password:” Prompt & Input Box ──────────────────────────────
    {
        confirmPromptLabel = new Label(
            "Confirm Password:",
            "balatro.ttf", 60,
            halfW - 300, halfH + 60,  // offset slightly left so text fits
            255, 255, 255, 255,
            0.5f, 0.5f
        );
        AddNewObject(confirmPromptLabel);

        confirmBoxW = 600; confirmBoxH = 80;
        confirmBoxX = (halfW + 200) - (confirmBoxW / 2);
        confirmBoxY = (halfH + 60) - (confirmBoxH / 2);

        confirmInputLabel = new Label(
            "",   // starts empty
            "balatro.ttf", 60,
            confirmBoxX + 15, confirmBoxY + (confirmBoxH / 2),
            255, 255, 255, 255,
            0.0f, 0.5f
        );
        AddNewObject(confirmInputLabel);
    }

    // ─── “Show/Hide Password” Eye Icon (shared for both password fields) ────
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
    {
        // Place the eye icon next to the “Confirm Password” box by default
        int eyeX = confirmBoxX + confirmBoxW + 20;
        int eyeY = confirmBoxY + (confirmBoxH / 2) - 60;
        eyeButton = new ImageButton(
            "closeeyewhite.png",
            "openeyewhite.png",
            eyeX, eyeY,
            80, 130
        );
        eyeButton->Visible = false;
        eyeButton->SetOnClickCallback([&]() {
            // Toggle revealPassword; whichever password field is focused gets toggled
            revealPassword = !revealPassword;

            if (revealPassword) {
                eyeButton->SetImage(
                    "openeyewhite.png",
                    "closeeyewhite.png"
                );
                // If focusing confirmField (2), reveal confirm; else reveal main password
                if (typingField == 2) {
                    confirmInputLabel->Text = typedConfirmPassword;
                } else {
                    passwordInputLabel->Text = typedPassword;
                }
            } else {
                eyeButton->SetImage(
                    "closeeyewhite.png",
                    "openeyewhite.png"
                );
                // Mask whichever field is active
                if (typingField == 2) {
                    confirmInputLabel->Text = std::string(typedConfirmPassword.size(), '*');
                } else {
                    passwordInputLabel->Text = std::string(typedPassword.size(), '*');
                }
            }
        });
        AddNewControlObject(eyeButton);
    }

    // ─── Info Label (for errors like “EMAIL_EXISTS” or “Passwords do not match”) ─
    {
        infoLabel = new Label(
            "",
            "balatro.ttf", 40,
            halfW,
            halfH + 160,
            255, 0, 0, 255,  // red text
            0.5f, 0.5f
        );
        AddNewObject(infoLabel);
    }

    // ─── “Register” Button ────────────────────────────────────────────────────
    {
        registerButton = new ImageButton(
            "stage-select/dirt.png",
            "stage-select/floor.png",
            halfW - 150,
            halfH + 240,
            300, 80
        );
        registerButton->SetOnClickCallback([&]() {
            OnRegisterClicked();
        });
        AddNewControlObject(registerButton);

        AddNewObject(new Label(
            "Register",
            "balatro.ttf", 60,
            halfW,
            halfH + 240 + 40,
            0, 0, 0, 255,  // black text
            0.5f, 0.5f
        ));
    }
}

void RegisterOnlineScene::Terminate()
{
    IScene::Terminate();
    // Static bitmaps/font already destroyed in the destructor
}

void RegisterOnlineScene::Update(float dt)
{
    IScene::Update(dt);
    // Display any error (Firebase error or “Passwords do not match”)
    infoLabel->Text = errorMessage;
}

void RegisterOnlineScene::Draw() const
{
    IScene::Draw();

    // Draw a colored rectangle around the active input field:
    //   typingField == 0 → email (green)
    //   typingField == 1 → password (red)
    //   typingField == 2 → confirm password (red)
    ALLEGRO_COLOR focusColor = (typingField == 0)
        ? al_map_rgb(0, 255, 0)
        : al_map_rgb(255, 0, 0);

    // Recompute positions (same as Initialize)
    int w     = GameEngine::GetInstance().GetScreenSize().x;
    int h     = GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // Email box rectangle:
    int eW = 600, eH = 80;
    int eX = (halfW + 200) - (eW / 2);
    int eY = (halfH - 180) - (eH / 2);

    // Password box rectangle:
    int pW = 600, pH = 80;
    int pX = (halfW + 200) - (pW / 2);
    int pY = (halfH - 60)  - (pH / 2);

    // Confirm password box rectangle:
    int cW = 600, cH = 80;
    int cX = (halfW + 200) - (cW / 2);
    int cY = (halfH + 60)  - (cH / 2);

    if (typingField == 0) {
        al_draw_rectangle(eX, eY, eX + eW, eY + eH, focusColor, 4.0f);
    }
    else if (typingField == 1) {
        al_draw_rectangle(pX, pY, pX + pW, pY + pH, focusColor, 4.0f);
    }
    else {
        al_draw_rectangle(cX, cY, cX + cW, cY + cH, focusColor, 4.0f);
    }
}

void RegisterOnlineScene::OnKeyChar(int unicode)
{
    // Enter (↵) and Tab (↹) to switch between fields or confirm:
    if (unicode == '\r') {
        if (typingField < 2) {
            ToggleInputFocus();
        } else {
            OnRegisterClicked();
        }
        return;
    }
    if (unicode == '\t') {
        ToggleInputFocus();
        return;
    }

    // Backspace
    if (unicode == '\b') {
        if (typingField == 0 && !typedEmail.empty()) {
            typedEmail.pop_back();
        }
        else if (typingField == 1 && !typedPassword.empty()) {
            typedPassword.pop_back();
        }
        else if (typingField == 2 && !typedConfirmPassword.empty()) {
            typedConfirmPassword.pop_back();
        }
    }
    // Printable ASCII
    else if (unicode >= 32 && unicode < 127) {
        char c = static_cast<char>(unicode);
        if (typingField == 0) {
            if (typedEmail.size() < 64) typedEmail.push_back(c);
        }
        else if (typingField == 1) {
            if (typedPassword.size() < 64) typedPassword.push_back(c);
        }
        else {
            if (typedConfirmPassword.size() < 64) typedConfirmPassword.push_back(c);
        }
    }

    // 1) Update email label
    emailInputLabel->Text = typedEmail;

    // 2) Show/hide eye icon if either password field is non‐empty and that field is focused
    if (typingField == 1) {
        if (!typedPassword.empty()) {
            if (!eyeButton->Visible) {
                eyeButton->Visible = true;
                revealPassword = false;
                eyeButton->SetImage(
                    "closeeyewhite.png",
                    "openeyewhite.png"
                );
            }
        } else {
            if (eyeButton->Visible) {
                eyeButton->Visible = false;
                revealPassword = false;
            }
        }
    }
    else if (typingField == 2) {
        if (!typedConfirmPassword.empty()) {
            if (!eyeButton->Visible) {
                eyeButton->Visible = true;
                revealPassword = false;
                eyeButton->SetImage(
                    "closeeyewhite.png",
                    "openeyewhite.png"
                );
            }
        } else {
            if (eyeButton->Visible) {
                eyeButton->Visible = false;
                revealPassword = false;
            }
        }
    } else {
        // If focus is on email (field 0), hide the eye icon
        if (eyeButton->Visible) {
            eyeButton->Visible = false;
            revealPassword = false;
        }
    }

    // 3) Update password label text (either masked or clear)
    if (typingField == 1) {
        if (revealPassword) {
            passwordInputLabel->Text = typedPassword;
        } else {
            passwordInputLabel->Text = std::string(typedPassword.size(), '*');
            if (eyeButton->Visible) {
                eyeButton->SetImage(
                    "closeeyewhite.png",
                    "openeyewhite.png"
                );
            }
        }
    }
    // 4) Update confirm password label (either masked or clear)
    if (typingField == 2) {
        if (revealPassword) {
            confirmInputLabel->Text = typedConfirmPassword;
        } else {
            confirmInputLabel->Text = std::string(typedConfirmPassword.size(), '*');
            if (eyeButton->Visible) {
                eyeButton->SetImage(
                    "closeeyewhite.png",
                    "openeyewhite.png"
                );
            }
        }
    }
}

void RegisterOnlineScene::ToggleInputFocus()
{
    // Cycle through 0 → 1 → 2 → 0 → …
    typingField = (typingField + 1) % 3;

    // Move the eye icon next to whichever password field is active (if fields 1 or 2)
    int w     = GameEngine::GetInstance().GetScreenSize().x;
    int h     = GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    if (typingField == 1) {
        // Password field
        float eyeX = passwordBoxX + passwordBoxW + 20;
        float eyeY = passwordBoxY + (passwordBoxH / 2) - 60;
        eyeButton->Position = Engine::Point(static_cast<float>(eyeX),
                                    static_cast<float>(eyeY));


    }
    else if (typingField == 2) {
        // Confirm password field
        float eyeX = confirmBoxX + confirmBoxW + 20;
        float eyeY = confirmBoxY + (confirmBoxH / 2) - 60;
        eyeButton->Position = Engine::Point(static_cast<float>(eyeX),
                                    static_cast<float>(eyeY));

    } else {
        // If focus moves to email (0), hide the eye
        eyeButton->Visible = false;
        revealPassword = false;
    }
}

void RegisterOnlineScene::OnRegisterClicked()
{
    // 1) Client‐side checks
    if (typedEmail.empty() || typedPassword.empty() || typedConfirmPassword.empty()) {
        errorMessage = "All fields are required.";
        return;
    }

    if (typedPassword != typedConfirmPassword) {
        errorMessage = "Passwords do not match.";
        return;
    }

    // 2) Call the Firebase register helper
    std::string firebaseError;
    bool success = PerformFirebaseRegister(typedEmail, typedPassword, firebaseError);

    if (success) {
        // On success, store current user and switch to “start”
        extern std::string CurrentUser;
        CurrentUser = typedEmail;
        GameEngine::GetInstance().ChangeScene("start");
    }
    else {
        // Display Firebase’s error (e.g. “EMAIL_EXISTS” or “WEAK_PASSWORD”)
        errorMessage = firebaseError;
    }
}

void RegisterOnlineScene::OnBackClicked()
{
    GameEngine::GetInstance().ChangeScene("login-online");
}

void RegisterOnlineScene::OnMouseDown(int button, int x, int y)
{
    if (button == 1) { // left click

        int w     = GameEngine::GetInstance().GetScreenSize().x;
        int h     = GameEngine::GetInstance().GetScreenSize().y;
        int halfW = w / 2;
        int halfH = h / 2;

        // Recompute the three box positions (must match Initialize()):
        // ────────────────────────────────────────────────────────────
        // Email box:
        int eW = 600, eH = 80;
        int eX = (halfW + 200) - (eW / 2);
        int eY = (halfH - 180) - (eH / 2);

        // Password box:
        int pW = 600, pH = 80;
        int pX = (halfW + 200) - (pW / 2);
        int pY = (halfH - 60) - (pH / 2);

        // Confirm password box:
        int cW = 600, cH = 80;
        int cX = (halfW + 200) - (cW / 2);
        int cY = (halfH + 60) - (cH / 2);
        // ────────────────────────────────────────────────────────────

        // 1) Click inside “Email” box?
        if (x >= eX && x <= eX + eW &&
            y >= eY && y <= eY + eH)
        {
            typingField = 0;
            // Hide the eye icon when on email:
            eyeButton->Visible = false;
            revealPassword = false;
            return;
        }

        // 2) Click inside “Password” box?
        if (x >= pX && x <= pX + pW &&
            y >= pY && y <= pY + pH)
        {
            typingField = 1;

            // Show/hide eye only when password is non‐empty:
            if (!typedPassword.empty()) {
                eyeButton->Visible = true;
                revealPassword = false;
                // Position the eye icon next to the password box:
                float eyeX = static_cast<float>(pX + pW + 20);
                float eyeY = static_cast<float>(pY + (pH / 2) - 60);
                eyeButton->Position = Engine::Point(eyeX, eyeY);
                // Make sure it shows the “closed” icon initially:
                eyeButton->SetImage(
                    "closeeyewhite.png",
                    "openeyewhite.png"
                );
            } else {
                eyeButton->Visible = false;
                revealPassword = false;
            }
            return;
        }

        // 3) Click inside “Confirm Password” box?
        if (x >= cX && x <= cX + cW &&
            y >= cY && y <= cY + cH)
        {
            typingField = 2;

            // Show/hide eye only when confirmPassword is non‐empty:
            if (!typedConfirmPassword.empty()) {
                eyeButton->Visible = true;
                revealPassword = false;
                // Position the eye icon next to the confirm box:
                float eyeX = static_cast<float>(cX + cW + 20);
                float eyeY = static_cast<float>(cY + (cH / 2) - 60);
                eyeButton->Position = Engine::Point(eyeX, eyeY);
                eyeButton->SetImage(
                    "closeeyewhite.png",
                    "openeyewhite.png"
                );
            } else {
                eyeButton->Visible = false;
                revealPassword = false;
            }
            return;
        }
    }

    // Otherwise, let the base IScene handle clicks on buttons (Register/Back/Eye):
    IScene::OnMouseDown(button, x, y);
}
