#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "Account/LoginOnlineScene.hpp"
#include "Engine/GameEngine.hpp"
#include "allegro5/allegro_primitives.h"
#include "Account/ScoreboardOnline.hpp"
#include "Scene/StartScene.h"

// Include the single‐header HTTP+HTTPS client and JSON library:
#include "httplib.h"    // https://github.com/yhirose/cpp-httplib (place httplib.h in your project root)
#include "json.hpp"     // https://github.com/nlohmann/json (place json.hpp in your project root)

using namespace Engine;
using json = nlohmann::json;

// ──────────────────────────────────────────────────────────────────────────
// File‐scope statics for the scene:
ALLEGRO_FONT*   LoginOnlineScene::onlineFont            = nullptr;
bool            LoginOnlineScene::loginRevealPasswordOnline = false;
ALLEGRO_BITMAP* LoginOnlineScene::openEyeBmpOnline      = nullptr;
ALLEGRO_BITMAP* LoginOnlineScene::closeEyeBmpOnline     = nullptr;
ImageButton*    LoginOnlineScene::eyeButtonOnline       = nullptr;

// ── Replace this with your actual Firebase Web API Key ───────────────────
static constexpr const char* FIREBASE_WEB_API_KEY = "AIzaSyAarObWywZJ_rQ2AlXDd6czNdnRqqSTRbo";
// ──────────────────────────────────────────────────────────────────────────


//───────────────────────────────────────────────────────────────────────────
// Helper: PerformFirebaseLogin
//
// Calls Firebase’s Identity Toolkit REST endpoint “signInWithPassword”.
// Returns true on success (idToken present), or false on failure (error in outError).
// You must link OpenSSL so that httplib::SSLClient (HTTPS) works.
//
// Firebase endpoint:
//   POST https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=[API_KEY]
//   Body (JSON):
//     {
//       "email": "<USER_EMAIL>",
//       "password": "<USER_PASSWORD>",
//       "returnSecureToken": true
//     }
//
// On success, the response JSON contains "idToken".  On failure, it contains “error” with “message”.
//───────────────────────────────────────────────────────────────────────────
static bool PerformFirebaseLogin(const std::string& email,
                                 const std::string& password,
                                 std::string& outError)
{
    std::cout << "[DBG] PerformFirebaseLogin(email=\"" << email << "\")\n";

    // 1) Create HTTPS client
    httplib::SSLClient client("identitytoolkit.googleapis.com", 443);
    client.enable_server_certificate_verification(false);
    client.set_connection_timeout(5, 0);
    client.set_read_timeout(5, 0);
    std::cout << "[DBG] Timeouts: connect=5s, read=5s\n";

    // 2) Log every request/response
    client.set_logger([](const httplib::Request &req, const httplib::Response &res) {
        std::cout << "[HTTP] " << req.method << " " << req.path
                  << " --> " << res.status << "\n"
                  << "[HTTP] Response body:\n" << res.body << "\n";
    });

    // 3) Build path + JSON body
    std::string path = "/v1/accounts:signInWithPassword?key=" + std::string(FIREBASE_WEB_API_KEY);
    json body = {
        {"email", email},
        {"password", password},
        {"returnSecureToken", true}
    };
    std::cout << "[DBG] POST " << path << "\n"
              << "[DBG] Payload: " << body.dump() << "\n";

    // 4) Send
    auto res = client.Post(path.c_str(), body.dump(), "application/json");
    std::cout << "[DBG] client.Post() returned " << (res ? "response" : "nullptr") << "\n";
    if (!res) {
        std::cerr << "[ERR] Network failure (httplib error=" 
                  << static_cast<int>(res.error()) << ")\n";
        outError = "Network error: could not connect.";
        return false;
    }

    // 5) Check HTTP status
    std::cout << "[DBG] HTTP status = " << res->status << "\n";
    if (res->status != 200) {
        try {
            auto errJ = json::parse(res->body);
            outError = errJ["error"]["message"].get<std::string>();
        } catch(...) {
            outError = "Unknown server error";
        }
        std::cerr << "[ERR] Server returned HTTP " << res->status
                  << ": " << outError << "\n";
        return false;
    }

    // 6) Success: parse idToken
    auto successJ = json::parse(res->body);
    if (auto it = successJ.find("idToken"); it != successJ.end()) {
        std::cout << "[DBG] idToken length = " << it->get<std::string>().size() << "\n";
        return true;
    }

    outError = "No idToken in response";
    std::cerr << "[ERR] Login succeeded but no idToken\n";
    return false;
}


//───────────────────────────────────────────────────────────────────────────
// LoginOnlineScene Implementation
//───────────────────────────────────────────────────────────────────────────

LoginOnlineScene::LoginOnlineScene()
    : typedEmail()
    , typedPassword()
    , typingEmail(true)
    , emailPromptLabel(nullptr)
    , emailInputLabel(nullptr)
    , passwordPromptLabel(nullptr)
    , passwordInputLabel(nullptr)
    , infoLabel(nullptr)
    , loginButton(nullptr)
    , registerButton(nullptr)
    , backButton(nullptr)
    , backLabel(nullptr)
    , emailBoxX(0), emailBoxY(0), emailBoxW(0), emailBoxH(0)
    , passwordBoxX(0), passwordBoxY(0), passwordBoxW(0), passwordBoxH(0)
    , errorMessage()
{
}

LoginOnlineScene::~LoginOnlineScene()
{
    // Destroy static bitmaps and font once when this class is torn down
    if (openEyeBmpOnline)   { al_destroy_bitmap(openEyeBmpOnline);   openEyeBmpOnline   = nullptr; }
    if (closeEyeBmpOnline)  { al_destroy_bitmap(closeEyeBmpOnline);  closeEyeBmpOnline  = nullptr; }
    if (onlineFont)         { al_destroy_font(onlineFont);            onlineFont         = nullptr; }
}

void LoginOnlineScene::Initialize()
{
    // Reset state
    typedEmail.clear();
    typedPassword.clear();
    typingEmail = true;
    loginRevealPasswordOnline = false;
    errorMessage.clear();

    // Screen dimensions
    int w     = GameEngine::GetInstance().GetScreenSize().x;
    int h     = GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // ── 1) Load TTF font once ─────────────────────────────────────────────
    if (!onlineFont) {
        onlineFont = al_load_font("Resource/fonts/balatro.ttf", 60, 0);
        if (!onlineFont) {
            throw std::runtime_error("Failed to load Resource/fonts/balatro.ttf");
        }
    }

    // ── 2) “Back” Button ───────────────────────────────────────────────────
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
            "balatro.ttf", 
            60,
            btnX + (btnW / 2),
            btnY + (btnH / 2) + 5,
            0, 0, 0, 255,
            0.5f, 0.5f
        );
        AddNewObject(backLabel);
    }

    // ── 3) “Email:” Prompt & Input Box ─────────────────────────────────────
    emailPromptLabel = new Label(
        "Email:",
        "balatro.ttf", 60,
        halfW - 200,
        halfH - 140,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(emailPromptLabel);

    emailBoxW = 600; emailBoxH = 80;
    emailBoxX = (halfW + 200) - (emailBoxW / 2);
    emailBoxY = (halfH - 140) - (emailBoxH / 2);

    emailInputLabel = new Label(
        "",
        "balatro.ttf", 60,
        emailBoxX + 15,
        emailBoxY + (emailBoxH / 2),
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(emailInputLabel);

    // ── 4) “Password:” Prompt & Input Box ─────────────────────────────────
    passwordPromptLabel = new Label(
        "Password:",
        "balatro.ttf", 60,
        halfW - 200,
        halfH - 40,
        255, 255, 255, 255,
        0.5f, 0.5f
    );
    AddNewObject(passwordPromptLabel);

    passwordBoxW = 600; passwordBoxH = 80;
    passwordBoxX = (halfW + 200) - (passwordBoxW / 2);
    passwordBoxY = (halfH - 40) - (passwordBoxH / 2);

    passwordInputLabel = new Label(
        "",
        "balatro.ttf", 60,
        passwordBoxX + 15,
        passwordBoxY + (passwordBoxH / 2),
        255, 255, 255, 255,
        0.0f, 0.5f
    );
    AddNewObject(passwordInputLabel);

    // ── 5) Load eye icons (show/hide password) ─────────────────────────────
    if (!openEyeBmpOnline) {
        openEyeBmpOnline = al_load_bitmap("Resource/images/openeyewhite.png");
        if (!openEyeBmpOnline) {
            throw std::runtime_error("Failed to load openeyewhite.png");
        }
    }
    if (!closeEyeBmpOnline) {
        closeEyeBmpOnline = al_load_bitmap("Resource/images/closeeyewhite.png");
        if (!closeEyeBmpOnline) {
            throw std::runtime_error("Failed to load closeeyewhite.png");
        }
    }

    {
        int eyeX = passwordBoxX + passwordBoxW + 20;
        int eyeY = passwordBoxY + (passwordBoxH / 2) - 60;
        eyeButtonOnline = new ImageButton(
            "closeeyewhite.png",
            "openeyewhite.png",
            eyeX, eyeY,
            80, 130
        );
        eyeButtonOnline->Visible = false;
        eyeButtonOnline->SetOnClickCallback([&]() {
            loginRevealPasswordOnline = !loginRevealPasswordOnline;
            if (loginRevealPasswordOnline) {
                eyeButtonOnline->SetImage("openeyewhite.png", "closeeyewhite.png");
                passwordInputLabel->Text = typedPassword;
            } else {
                eyeButtonOnline->SetImage("closeeyewhite.png", "openeyewhite.png");
                passwordInputLabel->Text = std::string(typedPassword.size(), '*');
            }
        });
        AddNewControlObject(eyeButtonOnline);
    }

    // ── 6) Info label (for errors) ─────────────────────────────────────────
    infoLabel = new Label(
        "",
        "balatro.ttf", 40,
        halfW,
        halfH + 60,
        255, 0, 0, 255,
        0.5f, 0.5f
    );
    AddNewObject(infoLabel);

    // ── 7) “Login” Button ───────────────────────────────────────────────────
    loginButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW - 150,
        halfH + 140,
        300, 80
    );
    loginButton->SetOnClickCallback([&]() { OnLoginClicked(); });
    AddNewControlObject(loginButton);

    AddNewObject(new Label(
        "Login",
        "balatro.ttf", 60,
        halfW,
        halfH + 140 + 40,
        0, 0, 0, 255,
        0.5f, 0.5f
    ));

    // ── 8) “Register” Button (optional) ────────────────────────────────────
    registerButton = new ImageButton(
        "stage-select/dirt.png",
        "stage-select/floor.png",
        halfW + 200,
        halfH + 140,
        300, 80
    );
    registerButton->SetOnClickCallback([&]() { OnRegisterClicked(); });
    AddNewControlObject(registerButton);

    AddNewObject(new Label(
        "Register",
        "balatro.ttf", 60,
        halfW + 200 + 150,
        halfH + 140 + 40,
        0, 0, 0, 255,
        0.5f, 0.5f
    ));
}

void LoginOnlineScene::Terminate()
{
    IScene::Terminate();
}

void LoginOnlineScene::Update(float dt)
{
    IScene::Update(dt);
    // Display any error message in the info label
    infoLabel->Text = errorMessage;
}

void LoginOnlineScene::Draw() const
{
    IScene::Draw();

    // Draw a colored rectangle around the currently focused input box
    ALLEGRO_COLOR focusColor = typingEmail ? al_map_rgb(0, 255, 0)
                                           : al_map_rgb(255, 0, 0);

    int w     = GameEngine::GetInstance().GetScreenSize().x;
    int h     = GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // Recompute box positions (same as Initialize)
    int eW = 600, eH = 80;
    int eX = (halfW + 200) - (eW / 2);
    int eY = (halfH - 140) - (eH / 2);

    int pW = 600, pH = 80;
    int pX = (halfW + 200) - (pW / 2);
    int pY = (halfH - 40)  - (pH / 2);

    if (typingEmail) {
        al_draw_rectangle(
            eX, eY,
            eX + eW, eY + eH,
            focusColor, 4.0f
        );
    } else {
        al_draw_rectangle(
            pX, pY,
            pX + pW, pY + pH,
            focusColor, 4.0f
        );
    }
}

void LoginOnlineScene::OnKeyChar(int unicode)
{
    // Handle Enter and Tab
    if (unicode == '\r') {
        if (typingEmail) ToggleInputFocus();
        else             OnLoginClicked();
        return;
    }
    if (unicode == '\t') {
        ToggleInputFocus();
        return;
    }

    // Handle Backspace
    if (unicode == '\b') {
        if (typingEmail && !typedEmail.empty()) {
            typedEmail.pop_back();
        } else if (!typingEmail && !typedPassword.empty()) {
            typedPassword.pop_back();
        }
    }
    // Handle printable ASCII characters
    else if (unicode >= 32 && unicode < 127) {
        char c = static_cast<char>(unicode);
        if (typingEmail) {
            if (typedEmail.size() < 64) {
                typedEmail.push_back(c);
            }
        } else {
            if (typedPassword.size() < 64) {
                typedPassword.push_back(c);
            }
        }
    }

    // 1) Update the email label with typedEmail
    emailInputLabel->Text = typedEmail;

    // 2) Show/hide eye icon if password is nonempty
    if (!typedPassword.empty()) {
        if (!eyeButtonOnline->Visible) {
            eyeButtonOnline->Visible = true;
            loginRevealPasswordOnline = false;
            eyeButtonOnline->SetImage("closeeyewhite.png",
                                      "openeyewhite.png");
        }
    } else {
        if (eyeButtonOnline->Visible) {
            eyeButtonOnline->Visible = false;
            loginRevealPasswordOnline = false;
        }
    }

    // 3) Update password label text (masked or revealed)
    if (loginRevealPasswordOnline) {
        passwordInputLabel->Text = typedPassword;
    } else {
        passwordInputLabel->Text = std::string(typedPassword.size(), '*');
        if (eyeButtonOnline->Visible) {
            eyeButtonOnline->SetImage("closeeyewhite.png",
                                      "openeyewhite.png");
        }
    }
}

void LoginOnlineScene::ToggleInputFocus()
{
    typingEmail = !typingEmail;
}

void LoginOnlineScene::OnLoginClicked()
{

    if (typedEmail.empty() || typedPassword.empty()) {
              infoLabel->Text = "Email and password required";
               return;
            }
        
            // Sign in via our online service (this sets idToken & localId)
    if (ScoreboardOnline::SignIn(typedEmail, typedPassword)) {
        CurrentUser = typedEmail;
        if (auto* s = dynamic_cast<StartScene*>(
        Engine::GameEngine::GetInstance().GetScene("start")))
            {
                s->SetPreviousScene("login-online");
            }
            GameEngine::GetInstance().ChangeScene("start");
            return;
    }
        
           // otherwise show the error
   infoLabel->Text = "Login failed";
        
    // Basic client‐side validation
    if (typedEmail.empty() || typedPassword.empty()) {
        errorMessage = "Email and password required.";
        return;
    }

    // Call helper to perform HTTPS POST to Firebase
    std::string firebaseError;
    bool success = PerformFirebaseLogin(typedEmail, typedPassword, firebaseError);
    if (success) {
        // Login succeeded: store current user (for example) and change scene
        std::cout << "Login success!\n";
        extern std::string CurrentUser;
        CurrentUser = typedEmail;
        GameEngine::GetInstance().ChangeScene("start");
    }
    else {
        // Display Firebase’s error (e.g. “INVALID_PASSWORD”)
        errorMessage = firebaseError;
    }
}

void LoginOnlineScene::OnRegisterClicked()
{
    // If you have a separate registration scene, you could do:
    GameEngine::GetInstance().ChangeScene("register-online");
    // For now, just show a placeholder:
    
}

void LoginOnlineScene::OnBackClicked()
{
    GameEngine::GetInstance().ChangeScene("local-online");
}

void LoginOnlineScene::OnMouseDown(int button, int x, int y)
{
    if (button == 1) { // left‐mouse
        int w     = GameEngine::GetInstance().GetScreenSize().x;
        int h     = GameEngine::GetInstance().GetScreenSize().y;
        int halfW = w / 2;
        int halfH = h / 2;

        // Recompute box rectangles (same as Initialize & Draw)
        int eW = 600, eH = 80;
        int eX = (halfW + 200) - (eW / 2);
        int eY = (halfH - 140) - (eH / 2);

        int pW = 600, pH = 80;
        int pX = (halfW + 200) - (pW / 2);
        int pY = (halfH - 40)  - (pH / 2);

        // If user clicked inside the “email” box
        if (x >= eX && x <= eX + eW &&
            y >= eY && y <= eY + eH)
        {
            typingEmail = true;
            return;
        }
        // If user clicked inside the “password” box
        if (x >= pX && x <= pX + pW &&
            y >= pY && y <= pY + pH)
        {
            typingEmail = false;
            return;
        }
    }
    IScene::OnMouseDown(button, x, y);
}
