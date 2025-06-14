#ifndef ACCOUNT_MANAGER_HPP
#define ACCOUNT_MANAGER_HPP

#include <string>
#include <vector>

/**
 * A simple struct representing one local account entry.
 * - username:   the unique user ID
 * - passwordHash:  a (non-cryptographic) hash of the password (for demo)
 * - createdAt:  timestamp string, e.g. "2025-06-04 16:00:00"
 */
struct AccountInfo {
    std::string username;
    std::string passwordHash;
    std::string createdAt;
};

class AccountManager {
public:
    /** 
     * Read all accounts from Resource/accounts.txt into a vector.
     * Each line in accounts.txt should be: 
     *     username <space> passwordHash <space> createdAt
     */
    static std::vector<AccountInfo> LoadAllAccounts();

    /** Return true if username already exists in accounts.txt */
    static bool UsernameExists(const std::string& username);

    /** 
     * Verify that given username/password matches an entry (by re-hashing password).
     * Returns true if (username, password) is correct.
     */
    static bool VerifyPassword(const std::string& username, const std::string& password);

    /** 
     * Register a brand-new account. 
     * - If username exists, return false.
     * - Otherwise, hash password, append to accounts.txt, return true.
     */
    static bool RegisterNewAccount(const std::string& username, const std::string& password);

private:
    /** 
     * A very simple “hash” for demo purposes. 
     * Note: std::hash is not cryptographically secure! 
     * For a prototype, this is okay, but do NOT use in production.
     */
    static std::string HashPassword(const std::string& plain);

    /** 
     * Append a new AccountInfo to Resource/accounts.txt. 
     * Returns false if the file cannot be opened.
     */
    static bool AppendAccountToFile(const AccountInfo& info);
};

#endif // ACCOUNT_MANAGER_HPP
