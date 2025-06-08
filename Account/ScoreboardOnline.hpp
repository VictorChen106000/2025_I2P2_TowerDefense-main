// ScoreboardOnline.hpp
#ifndef SCOREBOARDONLINE_HPP
#define SCOREBOARDONLINE_HPP

#include <string>
#include <vector>
#include <utility>

/// Provides REST-based Firebase Auth and Realtime Database calls for the online scoreboard
class ScoreboardOnline {
public:
    /// Initialize with your Firebase Realtime Database host (e.g., "your-project-id.firebaseio.com") and Web API key
    static void Initialize(const std::string& firebaseHost,
                           const std::string& apiKey);

    /// Sign in existing user via email/password
    /// @returns true on success, populates idToken & localId
    static bool SignIn(const std::string& email,
                       const std::string& password);

    /// Upload or update the player's best score under /leaderboard/{uid}
    static bool UploadScore(const std::string& playerName,
                            int score,
                            int limit = 10);

    /// Fetch top `limit` scores (playerName, score)
    static std::vector<std::pair<std::string,int>> FetchTopScores(int limit = 10);

    /// Holds Firebase credentials after SignIn
    static std::string idToken;
    static std::string localId;

private:
    /// Internal Realtime Database host and Web API key
    static std::string host;
    static std::string key;
};

#endif // SCOREBOARDONLINE_HPP