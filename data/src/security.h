#ifndef SECURITY_H
#define SECURITY_H

#include <string>
#include <vector>
#include <map>
#include <ctime>

// Enhanced security constants
const int MAX_SESSION_TIME = 900; // 15 minutes in seconds
const int PASSWORD_HISTORY_COUNT = 5;
const int MIN_PASSWORD_LENGTH = 8;
const int MAX_SESSION_ATTEMPTS = 5;
const int MAX_TRANSACTION_AMOUNT = 100000.0;
const int SUSPICIOUS_AMOUNT_THRESHOLD = 50000.0;

// Security levels
enum SecurityLevel {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    CRITICAL = 3
};

// Security functions
std::string hash_pin(const std::string& pin);
std::string hash_password(const std::string& password);
std::string generate_secure_random(int length);
bool is_strong_pin(const std::string& pin);
bool is_strong_password(const std::string& password);
bool validate_account(const std::string& acc_number, const std::string& pin);
bool verify_password_history(const std::string& acc_number, const std::string& new_password);
void add_to_password_history(const std::string& acc_number, const std::string& password_hash);
void log_security_event(const std::string& event, SecurityLevel level = LOW, const std::string& account = "");

// Session management
class SessionManager {
private:
    static std::map<std::string, time_t> active_sessions;
    static std::map<std::string, int> session_attempts;
    static std::map<std::string, std::string> session_tokens;

public:
    static std::string create_session(const std::string& acc_number);
    static bool validate_session(const std::string& acc_number, const std::string& token);
    static void end_session(const std::string& acc_number);
    static void cleanup_expired_sessions();
    static bool check_session_attempts(const std::string& acc_number);
    static std::string get_current_session(const std::string& acc_number);
};

// Two-Factor Authentication
class TwoFactorAuth {
private:
    static std::map<std::string, std::string> pending_verifications;
    static std::map<std::string, time_t> verification_expiry;

public:
    static bool send_verification_code(const std::string& acc_number, const std::string& phone);
    static bool verify_code(const std::string& acc_number, const std::string& code);
    static void cleanup_expired_codes();
    static bool is_verification_pending(const std::string& acc_number);
};

// Suspicious Activity Detection
class SuspiciousActivityDetector {
private:
    static std::map<std::string, std::vector<time_t>> login_attempts;
    static std::map<std::string, std::vector<std::pair<double, time_t>>> transaction_patterns;

public:
    static bool detect_suspicious_login(const std::string& acc_number, const std::string& ip_address = "");
    static bool detect_suspicious_transaction(const std::string& acc_number, double amount, const std::string& transaction_type);
    static void record_login_attempt(const std::string& acc_number, bool success);
    static void record_transaction(const std::string& acc_number, double amount, const std::string& type);
    static void flag_suspicious_account(const std::string& acc_number, const std::string& reason);
    static void analyze_behavior_patterns(const std::string& acc_number);
};

// Encryption utilities
class CryptoUtils {
public:
    static std::string encrypt_sensitive_data(const std::string& data, const std::string& key = "");
    static std::string decrypt_sensitive_data(const std::string& encrypted_data, const std::string& key = "");
    static std::string generate_encryption_key();
    static std::string hash_data(const std::string& data);
};

// Security Policy Manager
class SecurityPolicy {
public:
    static bool validate_transaction_amount(double amount, const std::string& account_type);
    static bool validate_operation_time();
    static bool check_password_policy(const std::string& password);
    static bool check_account_lock_status(const std::string& acc_number);
    static void apply_security_policy(const std::string& acc_number);
};

#endif