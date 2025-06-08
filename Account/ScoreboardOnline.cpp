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
    httplib::SSLClient cli("identitytoolkit.googleapis.com", 443);
    cli.set_default_headers({{"Content-Type", "application/json"}});

    json payload = {
        {"email", email},
        {"password", password},
        {"returnSecureToken", true}
    };

    auto res = cli.Post(
                (std::string("/v1/accounts:signInWithPassword?key=") + key).c_str(),
                        payload.dump(),
                        "application/json");
    if (!res || res->status != 200) {
        return false;
    }
    auto j = json::parse(res->body);
    idToken = j.value("idToken", "");
    localId = j.value("localId", "");
    return !idToken.empty() && !localId.empty();
}

bool ScoreboardOnline::UploadScore(const std::string& playerName,
                                  int score,
                                  int limit) {
    if (idToken.empty() || localId.empty()) return false;
    httplib::SSLClient cli(host.c_str());
    cli.set_default_headers({{"Content-Type", "application/json"}});

    json payload = {
        {"name", playerName},
        {"score", score}
    };
    std::string path = "/leaderboard/" + localId + ".json?auth=" + idToken;
    auto res = cli.Put(path.c_str(), payload.dump(), "application/json");
    return res && res->status == 200;
}

std::vector<std::pair<std::string,int>>
ScoreboardOnline::FetchTopScores(int limit) {
    std::vector<std::pair<std::string,int>> result;
    if (idToken.empty()) return result;

    httplib::SSLClient cli(host.c_str());
    cli.set_default_headers({{"Content-Type", "application/json"}});

    std::string path = "/leaderboard.json?auth=" + idToken
                     + "&orderBy=\"score\"&limitToLast=" + std::to_string(limit);
    auto res = cli.Get(path.c_str());
    if (!res || res->status != 200) {
        return result;
    }
    auto j = json::parse(res->body);
    for (auto& item : j.items()) {
        const auto& entry = item.value();
        std::string name = entry.value("name", "");
        int scoreVal = entry.value("score", 0);
        result.emplace_back(name, scoreVal);
    }
    std::sort(result.begin(), result.end(), [](auto &a, auto &b) {
        return a.second > b.second;
    });
    return result;
}
