#include "AccountManager.hpp"
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

//------------------------------------------------------------
// HashPassword: a quick & dirty “hash” via std::hash.
// WARNING: not cryptographically secure! For prototype only.
//------------------------------------------------------------
std::string AccountManager::HashPassword(const std::string& plain) {
    std::hash<std::string> hasher;
    size_t hashedValue = hasher(plain);
    // Convert size_t to hex string for readability
    std::ostringstream oss;
    oss << std::hex << hashedValue;
    return oss.str();
}

//------------------------------------------------------------
// LoadAllAccounts: read lines from Resource/accounts.txt.
// Expected format per line: username passwordHash createdAt
//------------------------------------------------------------
std::vector<AccountInfo> AccountManager::LoadAllAccounts() {
    std::vector<AccountInfo> out;
    std::ifstream ifs("../Resource/accounts.txt");
    if (!ifs.is_open()) {
        // If the file doesn’t exist yet, return empty vector.
        return out;
    }

    std::string line;
    while (std::getline(ifs, line)) {
        // Skip blank lines or comments
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        AccountInfo ai;
        iss >> ai.username >> ai.passwordHash;
        // The rest of the line (after these two tokens) is createdAt
        std::getline(iss, ai.createdAt);
        // Trim leading whitespace from createdAt
        if (!ai.createdAt.empty() && std::isspace(ai.createdAt.front())) {
            ai.createdAt.erase(0, ai.createdAt.find_first_not_of(" \t"));
        }
        out.push_back(ai);
    }
    return out;
}

//------------------------------------------------------------
// UsernameExists: checks if any AccountInfo.username == username.
//------------------------------------------------------------
bool AccountManager::UsernameExists(const std::string& username) {
    auto all = LoadAllAccounts();
    for (auto& ai : all) {
        if (ai.username == username) {
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------
// VerifyPassword: re-hash the given plain password, compare.
//------------------------------------------------------------
bool AccountManager::VerifyPassword(const std::string& username, const std::string& password) {
    auto all = LoadAllAccounts();
    std::string hash = HashPassword(password);
    for (auto& ai : all) {
        if (ai.username == username && ai.passwordHash == hash) {
            return true;
        }
    }
    return false;
}

//------------------------------------------------------------
// AppendAccountToFile: append one AccountInfo as a new line.
// Format: username passwordHash createdAt
//------------------------------------------------------------
bool AccountManager::AppendAccountToFile(const AccountInfo& info) {
    std::ofstream ofs("../Resource/accounts.txt", std::ios::app);
    if (!ofs.is_open()) {
        return false;
    }
    ofs << info.username << " "
        << info.passwordHash << " "
        << info.createdAt << "\n";
    return true;
}

//------------------------------------------------------------
// RegisterNewAccount:
//   1) Return false if username already exists.
//   2) Otherwise, build AccountInfo, get current timestamp, append.
//------------------------------------------------------------
bool AccountManager::RegisterNewAccount(const std::string& username, const std::string& password) {
    if (UsernameExists(username)) {
        return false;
    }

    AccountInfo ai;
    ai.username = username;
    ai.passwordHash = HashPassword(password);

    // Get current local time as "YYYY-MM-DD HH:MM:SS"
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
#if defined(_WIN32)
    std::tm localTm;
    localtime_s(&localTm, &now_c);
#else
    std::tm localTm;
    localtime_r(&now_c, &localTm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S");
    ai.createdAt = oss.str();

    return AppendAccountToFile(ai);
}
