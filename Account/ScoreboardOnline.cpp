#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "ScoreboardOnline.hpp"
#include <httplib.h>      // cpp-httplib library
#include "json.hpp"     // nlohmann::json
#include <algorithm>      // std::sort

using json = nlohmann::json;

// Static member definitions
std::string ScoreboardOnline::host;
std::string ScoreboardOnline::key;
std::string ScoreboardOnline::idToken;
std::string ScoreboardOnline::localId;

void ScoreboardOnline::Initialize(const std::string& firebaseHost,
                                 const std::string& apiKey) {
    host = firebaseHost;
    key  = apiKey;
}

bool ScoreboardOnline::SignIn(const std::string& email,
                             const std::string& password) {
    try {
        // ⬇ Talk to the Auth API host, not your RTDB host
        httplib::SSLClient cli("identitytoolkit.googleapis.com", 443);
        // ⬇ For development on Windows—skip cert verification
        cli.enable_server_certificate_verification(false);
        cli.set_default_headers({{"Content-Type", "application/json"}});

        // build payload
        nlohmann::json payload = {
            {"email", email},
            {"password", password},
            {"returnSecureToken", true}
        };

        // fire the request
        auto res = cli.Post(
            (std::string("/v1/accounts:signInWithPassword?key=") + key).c_str(),
            payload.dump(),
            "application/json"
        );

        if (!res || res->status != 200) {
            std::cerr << "[ScoreboardOnline::SignIn] HTTP error: "
                      << (res ? res->status : -1) << "\n";
            return false;
        }

        // parse tokens
        auto j = nlohmann::json::parse(res->body);
        idToken = j.value("idToken", "");
        localId = j.value("localId", "");
        return !idToken.empty() && !localId.empty();
    }
    catch (const std::exception &ex) {
        std::cerr << "[ScoreboardOnline::SignIn] exception: "
                  << ex.what() << "\n";
        return false;
    }
}

bool ScoreboardOnline::UploadScore(const std::string& playerName,
                                  int score,
                                  int limit) {
    // if (idToken.empty() || localId.empty()) return false;
    // httplib::SSLClient cli(host.c_str());
    // cli.set_default_headers({{"Content-Type", "application/json"}});

    // ── DEBUG: check that we have credentials & host
    std::cout << "[DBG] UploadScore(): host=\"" << host 
     << "\" localId=\"" << localId    
     << "\" idToken.len=" << idToken.size() << "\n";  
     
    if (host.empty() || localId.empty() || idToken.empty()) {
        std::cerr << "[ERR] Missing host/localId/idToken - cannot upload\n";
        return false;
    }
    try {
        httplib::SSLClient cli(host.c_str(), 443);
        // for debug you can disable cert check, or later load a cacert.pem
        cli.enable_server_certificate_verification(false);
         cli.set_default_headers({{"Content-Type", "application/json"}});
        json payload = {
            {"name", playerName},
            {"score", score}
        };
    // std::string path = "/leaderboard/" + localId + ".json?auth=" + idToken;
    // auto res = cli.Put(path.c_str(), payload.dump(), "application/json");
    // return res && res->status == 200;
        std::string path = "/leaderboard/" + localId + ".json?auth=" + idToken;
        std::cout << "[DBG] PUT https://" << host << path << "\n"
        << "[DBG] Payload: " << payload.dump() << "\n";
        auto res = cli.Put(path.c_str(), payload.dump(), "application/json");
        if (!res) {
            std::cerr << "[ERR] UploadScore PUT failed, httplib error="
            << static_cast<int>(res.error()) << "\n";
            return false;
        }
        std::cout << "[DBG] UploadScore HTTP status=" << res->status << "\n";
        return res->status == 200;
    } catch (const std::exception &ex) {
        std::cerr << "[EXC] UploadScore exception: " << ex.what() << "\n";
         return false;
    }
}


std::vector<std::pair<std::string,int>>
ScoreboardOnline::FetchTopScores(int limit) {
    std::vector<std::pair<std::string,int>> result;
    try {
        httplib::SSLClient cli(host.c_str(), 443);
        cli.enable_server_certificate_verification(false);
        cli.set_default_headers({{"Content-Type", "application/json"}});

        std::string path =
            "/leaderboard.json?auth=" + idToken +
            "&orderBy=\"score\"&limitToLast=" + std::to_string(limit);
        auto res = cli.Get(path.c_str());
        if (!res || res->status != 200) {
            std::cerr << "[FetchTopScores] HTTP error: "
                      << (res ? res->status : -1) << "\n";
            return result;
        }

        auto j = nlohmann::json::parse(res->body);
        for (auto& item : j.items()) {
            const auto& entry = item.value();
            std::string name    = entry.value("name", "");
            int         scoreVal = entry.value("score", 0);
            result.emplace_back(name, scoreVal);
        }
        std::sort(result.begin(), result.end(),
                  [](auto &a, auto &b){ return a.second > b.second; });
        return result;
    }
    catch (const std::exception &ex) {
        std::cerr << "[FetchTopScores] exception: " << ex.what() << "\n";
        return result;
    }
}
