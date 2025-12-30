#include "security.h"
#include "banking.h"
#include "file_operations.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <functional>

using namespace std;

// Session management static members
map<string, time_t> SessionManager::active_sessions;
map<string, int> SessionManager::session_attempts;
map<string, string> SessionManager::session_tokens;

// Two-Factor Authentication static members
map<string, string> TwoFactorAuth::pending_verifications;
map<string, time_t> TwoFactorAuth::verification_expiry;

// Suspicious Activity Detection static members
map<string, vector<time_t>> SuspiciousActivityDetector::login_attempts;
map<string, vector<pair<double, time_t>>> SuspiciousActivityDetector::transaction_patterns;

// Enhanced PIN hashing with salt
string hash_pin(const string& pin) {
    string data = pin + "PIN_SALT_" + to_string(pin.length());
    size_t hash_val = hash<string>{}(data);
    stringstream ss;
    ss << hex << hash_val;
    return ss.str();
}

// Strong password hashing
string hash_password(const string& password) {
    string data = password + "PASSWORD_SALT_" + to_string(password.length());
    size_t hash_val = hash<string>{}(data);
    stringstream ss;
    ss << hex << hash_val;
    return ss.str();
}

// Generate secure random strings
string generate_secure_random(int length) {
    const string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    string result;
    
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<> distribution(0, charset.size() - 1);
    
    for (int i = 0; i < length; ++i) {
        result += charset[distribution(generator)];
    }
    
    return result;
}

bool is_strong_pin(const string& pin) {
    if (pin.length() != 4) return false;
    
    // Check if all characters are digits
    if (!all_of(pin.begin(), pin.end(), ::isdigit)) return false;
    
    // Check for all same digits
    if (all_of(pin.begin(), pin.end(), [&pin](char c) { return c == pin[0]; })) return false;
    
    // Check for sequential numbers (1234, 4321, etc.)
    bool sequential_asc = true;
    bool sequential_desc = true;
    for (size_t i = 1; i < pin.length(); i++) {
        if (pin[i] != pin[i-1] + 1) sequential_asc = false;
        if (pin[i] != pin[i-1] - 1) sequential_desc = false;
    }
    if (sequential_asc || sequential_desc) return false;
    
    // Check for common patterns (repeated, etc.)
    if (pin == "1212" || pin == "1234" || pin == "1111" || pin == "0000" || 
        pin == "2580" || pin == "1313" || pin == "1004" || pin == "2000") {
        return false;
    }
    
    return true;
}

bool is_strong_password(const string& password) {
    if (password.length() < MIN_PASSWORD_LENGTH) return false;
    
    bool has_upper = false, has_lower = false, has_digit = false, has_special = false;
    int unique_chars = 0;
    string unique_check;
    
    for (char c : password) {
        if (isupper(c)) has_upper = true;
        else if (islower(c)) has_lower = true;
        else if (isdigit(c)) has_digit = true;
        else if (ispunct(c)) has_special = true;
        
        // Check for unique characters
        if (unique_check.find(c) == string::npos) {
            unique_check += c;
            unique_chars++;
        }
    }
    
    // Require at least 60% unique characters
    bool sufficient_uniqueness = (unique_chars * 100 / password.length()) >= 60;
    
    return has_upper && has_lower && has_digit && has_special && sufficient_uniqueness;
}

bool validate_account(const string& acc_number, const string& pin) {
    auto it = accounts.find(acc_number);
    if (it == accounts.end()) {
        log_security_event("Failed login attempt - account not found: " + acc_number, MEDIUM);
        SuspiciousActivityDetector::record_login_attempt(acc_number, false);
        cout << "Account not found.\n";
        return false;
    }
    
    Account& acc = it->second;
    check_inactivity(acc);
    
    if (acc.status == "LOCKED") {
        log_security_event("Attempt to access locked account: " + acc_number, HIGH);
        cout << "Account locked due to too many failed attempts or inactivity. Please contact customer support.\n";
        return false;
    }
    
    // Check for suspicious login patterns
    if (SuspiciousActivityDetector::detect_suspicious_login(acc_number)) {
        log_security_event("Suspicious login pattern detected for account: " + acc_number, HIGH);
        SuspiciousActivityDetector::flag_suspicious_account(acc_number, "Suspicious login pattern");
        cout << "Suspicious activity detected. Account temporarily locked for security.\n";
        return false;
    }
    
    // Check session attempts
    if (!SessionManager::check_session_attempts(acc_number)) {
        log_security_event("Too many session attempts for account: " + acc_number, HIGH);
        cout << "Too many login attempts. Please try again later.\n";
        return false;
    }
    
    string hashed_pin = hash_pin(pin);
    if (acc.pin_hash == hashed_pin) {
        acc.failed_attempts = 0;
        acc.last_login_date = get_current_datetime();
        SessionManager::create_session(acc_number);
        SuspiciousActivityDetector::record_login_attempt(acc_number, true);
        log_security_event("Successful login: " + acc_number, LOW, acc_number);
        
        // Check if 2FA should be required
        if (acc.balance > SUSPICIOUS_AMOUNT_THRESHOLD) {
            cout << "High-value account detected. Two-factor authentication required.\n";
            TwoFactorAuth::send_verification_code(acc_number, acc.phone_number);
        }
        
        return true;
    } else {
        acc.failed_attempts++;
        SuspiciousActivityDetector::record_login_attempt(acc_number, false);
        log_security_event("Failed login attempt for account: " + acc_number + " (attempt " + to_string(acc.failed_attempts) + ")", MEDIUM, acc_number);
        
        if (acc.failed_attempts >= MAX_FAILED_ATTEMPTS) {
            acc.status = "LOCKED";
            save_all_accounts_to_file();
            log_security_event("Account locked: " + acc_number, HIGH, acc_number);
            cout << "Account locked due to too many failed attempts. Please contact customer support.\n";
        } else {
            cout << "Invalid PIN. " << (MAX_FAILED_ATTEMPTS - acc.failed_attempts) << " attempts remaining.\n";
            save_all_accounts_to_file();
        }
        return false;
    }
}

// Session Management Implementation
string SessionManager::create_session(const string& acc_number) {
    cleanup_expired_sessions();
    string token = generate_secure_random(32);
    active_sessions[acc_number] = time(0);
    session_tokens[acc_number] = token;
    session_attempts[acc_number] = 0;
    
    log_security_event("Session created: " + acc_number, LOW, acc_number);
    return token;
}

bool SessionManager::validate_session(const string& acc_number, const string& token) {
    auto session_it = active_sessions.find(acc_number);
    auto token_it = session_tokens.find(acc_number);
    
    if (session_it == active_sessions.end() || token_it == session_tokens.end()) {
        return false;
    }
    
    time_t current_time = time(0);
    if (difftime(current_time, session_it->second) > MAX_SESSION_TIME) {
        active_sessions.erase(session_it);
        session_tokens.erase(token_it);
        log_security_event("Session expired: " + acc_number, MEDIUM, acc_number);
        return false;
    }
    
    if (token_it->second != token) {
        log_security_event("Invalid session token for account: " + acc_number, HIGH, acc_number);
        return false;
    }
    
    // Update last activity
    session_it->second = current_time;
    return true;
}

void SessionManager::end_session(const string& acc_number) {
    active_sessions.erase(acc_number);
    session_tokens.erase(acc_number);
    session_attempts.erase(acc_number);
    log_security_event("Session ended: " + acc_number, LOW, acc_number);
}

void SessionManager::cleanup_expired_sessions() {
    time_t current_time = time(0);
    for (auto it = active_sessions.begin(); it != active_sessions.end();) {
        if (difftime(current_time, it->second) > MAX_SESSION_TIME) {
            string acc_number = it->first;
            session_tokens.erase(acc_number);
            session_attempts.erase(acc_number);
            it = active_sessions.erase(it);
            log_security_event("Expired session cleaned up: " + acc_number, LOW);
        } else {
            ++it;
        }
    }
}

bool SessionManager::check_session_attempts(const string& acc_number) {
    session_attempts[acc_number]++;
    bool within_limits = session_attempts[acc_number] <= MAX_SESSION_ATTEMPTS;
    
    if (!within_limits) {
        log_security_event("Session attempt limit exceeded: " + acc_number, HIGH, acc_number);
    }
    
    return within_limits;
}

string SessionManager::get_current_session(const string& acc_number) {
    auto it = session_tokens.find(acc_number);
    if (it != session_tokens.end()) {
        return it->second;
    }
    return "";
}

// Two-Factor Authentication Implementation
bool TwoFactorAuth::send_verification_code(const string& acc_number, const string& phone) {
    cleanup_expired_codes();
    
    string code = generate_secure_random(6);
    pending_verifications[acc_number] = code;
    verification_expiry[acc_number] = time(0) + 300; // 5 minutes expiry
    
    // In real implementation, integrate with SMS gateway
    cout << "\n=== TWO-FACTOR AUTHENTICATION ===\n";
    cout << "Verification code has been sent to your registered phone number.\n";
    cout << "Code: " << code << " (Expires in 5 minutes)\n";
    cout << "==============================\n";
    
    log_security_event("2FA code sent to " + acc_number, MEDIUM, acc_number);
    return true;
}

bool TwoFactorAuth::verify_code(const string& acc_number, const string& code) {
    cleanup_expired_codes();
    
    auto it = pending_verifications.find(acc_number);
    auto expiry_it = verification_expiry.find(acc_number);
    
    if (it == pending_verifications.end() || expiry_it == verification_expiry.end()) {
        log_security_event("2FA verification failed - no pending code: " + acc_number, MEDIUM, acc_number);
        return false;
    }
    
    if (time(0) > expiry_it->second) {
        pending_verifications.erase(it);
        verification_expiry.erase(expiry_it);
        log_security_event("2FA verification failed - code expired: " + acc_number, MEDIUM, acc_number);
        return false;
    }
    
    if (it->second == code) {
        pending_verifications.erase(it);
        verification_expiry.erase(expiry_it);
        log_security_event("2FA successful for " + acc_number, LOW, acc_number);
        return true;
    }
    
    log_security_event("2FA verification failed - invalid code: " + acc_number, MEDIUM, acc_number);
    return false;
}

void TwoFactorAuth::cleanup_expired_codes() {
    time_t current_time = time(0);
    for (auto it = verification_expiry.begin(); it != verification_expiry.end();) {
        if (current_time > it->second) {
            pending_verifications.erase(it->first);
            it = verification_expiry.erase(it);
        } else {
            ++it;
        }
    }
}

bool TwoFactorAuth::is_verification_pending(const string& acc_number) {
    cleanup_expired_codes();
    return pending_verifications.find(acc_number) != pending_verifications.end();
}

// Suspicious Activity Detection Implementation
bool SuspiciousActivityDetector::detect_suspicious_login(const string& acc_number, const string& ip_address) {
    // Clean old attempts (older than 1 hour)
    time_t current_time = time(0);
    time_t one_hour_ago = current_time - 3600;
    
    if (login_attempts.find(acc_number) != login_attempts.end()) {
        auto& attempts = login_attempts[acc_number];
        attempts.erase(
            remove_if(attempts.begin(), attempts.end(), 
                     [one_hour_ago](time_t t) { return t < one_hour_ago; }),
            attempts.end()
        );
        
        // If more than 10 attempts in last hour, suspicious
        if (attempts.size() > 10) {
            return true;
        }
    }
    
    return false;
}

bool SuspiciousActivityDetector::detect_suspicious_transaction(const string& acc_number, double amount, const string& transaction_type) {
    auto it = accounts.find(acc_number);
    if (it == accounts.end()) return false;
    
    Account& acc = it->second;
    
    // Check for unusually large transactions
    if (amount > SUSPICIOUS_AMOUNT_THRESHOLD) {
        log_security_event("Large transaction detected: " + to_string(amount) + " for " + acc_number, HIGH, acc_number);
        return true;
    }
    
    // Check for transactions that are much larger than account average
    if (amount > acc.balance * 0.8) { // More than 80% of balance
        log_security_event("High percentage transaction detected: " + to_string(amount) + " for " + acc_number, MEDIUM, acc_number);
        return true;
    }
    
    // Check for rapid successive transactions
    time_t current_time = time(0);
    time_t five_minutes_ago = current_time - 300;
    
    if (transaction_patterns.find(acc_number) != transaction_patterns.end()) {
        auto& transactions = transaction_patterns[acc_number];
        int recent_count = count_if(transactions.begin(), transactions.end(),
                                   [five_minutes_ago](const pair<double, time_t>& t) {
                                       return t.second > five_minutes_ago;
                                   });
        
        if (recent_count > 5) { // More than 5 transactions in 5 minutes
            log_security_event("Rapid transaction pattern detected for " + acc_number, HIGH, acc_number);
            return true;
        }
    }
    
    return false;
}

void SuspiciousActivityDetector::record_login_attempt(const string& acc_number, bool success) {
    login_attempts[acc_number].push_back(time(0));
}

void SuspiciousActivityDetector::record_transaction(const string& acc_number, double amount, const string& type) {
    transaction_patterns[acc_number].push_back(make_pair(amount, time(0)));
}

void SuspiciousActivityDetector::flag_suspicious_account(const string& acc_number, const string& reason) {
    auto it = accounts.find(acc_number);
    if (it != accounts.end()) {
        it->second.status = "UNDER_REVIEW";
        log_security_event("Account flagged: " + acc_number + " - " + reason, CRITICAL, acc_number);
        save_all_accounts_to_file();
    }
}

void SuspiciousActivityDetector::analyze_behavior_patterns(const string& acc_number) {
    // Implement behavioral analysis here
    // This could include machine learning in a real system
}

// Enhanced logging with security levels
void log_security_event(const string& event, SecurityLevel level, const string& account) {
    try {
        ofstream security_log("data/security.log", ios::app);
        if (security_log.is_open()) {
            string level_str;
            switch(level) {
                case LOW: level_str = "LOW"; break;
                case MEDIUM: level_str = "MEDIUM"; break;
                case HIGH: level_str = "HIGH"; break;
                case CRITICAL: level_str = "CRITICAL"; break;
                default: level_str = "UNKNOWN"; break;
            }
            
            security_log << get_current_datetime() << " [" << level_str << "] ";
            if (!account.empty()) {
                security_log << "Account:" << account << " ";
            }
            security_log << event << "\n";
            security_log.close();
        }
    } catch (const exception& e) {
        cerr << "Security log error: " << e.what() << endl;
    }
    
    // Also log critical events to audit log
    if (level >= HIGH) {
        log_audit_event("SECURITY[" + to_string(level) + "]: " + event);
    }
}

// CryptoUtils Implementation
string CryptoUtils::encrypt_sensitive_data(const string& data, const string& key) {
    // Simple XOR encryption for demonstration
    // In production, use AES or similar
    string result = data;
    char encryption_key = 0xAB;
    
    for (char& c : result) {
        c = c ^ encryption_key;
    }
    
    return result;
}

string CryptoUtils::decrypt_sensitive_data(const string& encrypted_data, const string& key) {
    // XOR decryption (same as encryption)
    return encrypt_sensitive_data(encrypted_data, key);
}

string CryptoUtils::generate_encryption_key() {
    return generate_secure_random(32);
}

string CryptoUtils::hash_data(const string& data) {
    return hash_password(data); // Reuse the secure hash function
}

// SecurityPolicy Implementation
bool SecurityPolicy::validate_transaction_amount(double amount, const string& account_type) {
    if (amount <= 0) return false;
    if (amount > MAX_TRANSACTION_AMOUNT) return false;
    
    // Additional policy checks based on account type
    if (account_type == "SAVINGS" && amount > 50000.0) {
        log_security_event("Large savings account transaction: " + to_string(amount), MEDIUM);
        return false;
    }
    
    return true;
}

bool SecurityPolicy::validate_operation_time() {
    // Check if operation is within banking hours (9 AM to 6 PM)
    // This is a simple implementation
    time_t now = time(0);
    tm* ltm = localtime(&now);
    int hour = ltm->tm_hour;
    
    if (hour < 9 || hour >= 18) {
        log_security_event("Operation outside banking hours", MEDIUM);
        return false;
    }
    
    return true;
}

bool SecurityPolicy::check_password_policy(const string& password) {
    return is_strong_password(password);
}

bool SecurityPolicy::check_account_lock_status(const string& acc_number) {
    auto it = accounts.find(acc_number);
    if (it != accounts.end()) {
        return it->second.status == "LOCKED" || it->second.status == "UNDER_REVIEW";
    }
    return true; // Treat non-existent accounts as locked
}

void SecurityPolicy::apply_security_policy(const string& acc_number) {
    auto it = accounts.find(acc_number);
    if (it != accounts.end()) {
        Account& acc = it->second;
        
        // Check for password age (force change every 90 days)
        // This would require storing password change date
        
        // Check for suspicious patterns
        SuspiciousActivityDetector::analyze_behavior_patterns(acc_number);
    }
}