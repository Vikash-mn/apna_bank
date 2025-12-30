#include "utils.h"
#include "banking.h"
#include "file_operations.h"
#include "security.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <regex>
#include <functional>
#include <algorithm>
#include <limits>

using namespace std;

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

string get_current_date() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    
    ostringstream oss;
    oss << 1900 + ltm->tm_year << "-" 
        << setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
        << setw(2) << setfill('0') << ltm->tm_mday;
    
    return oss.str();
}

string get_current_datetime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    
    ostringstream oss;
    oss << 1900 + ltm->tm_year << "-" 
        << setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
        << setw(2) << setfill('0') << ltm->tm_mday << " "
        << setw(2) << setfill('0') << ltm->tm_hour << ":"
        << setw(2) << setfill('0') << ltm->tm_min << ":"
        << setw(2) << setfill('0') << ltm->tm_sec;
    
    return oss.str();
}

string generate_random_string(int length, const string& charset) {
    string result;
    for (int i = 0; i < length; ++i) {
        result += charset[rand() % charset.length()];
    }
    return result;
}

string generate_account_number() {
    string acc_no;
    do {
        acc_no = "APNA" + generate_random_string(12, "0123456789");
    } while (accounts.find(acc_no) != accounts.end());
    return acc_no;
}

string generate_cif_number() {
    return generate_random_string(10, "0123456789");
}

string generate_ifsc_code() {
    return generate_random_string(11, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
}

string generate_micr_code() {
    return generate_random_string(9, "0123456789");
}

bool validate_phone_number(const string& phone) {
    return regex_match(phone, regex("^[0-9]{10}$"));
}

bool validate_email(const string& email) {
    const regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return regex_match(email, pattern);
}

bool validate_amount(const string& amount_str) {
    if (amount_str.empty()) return false;

    // Check for valid number format with at most 2 decimal places
    const regex pattern(R"(^\d+(\.\d{1,2})?$)");
    if (!regex_match(amount_str, pattern)) return false;

    try {
        double amount = stod(amount_str);
        return amount > 0;
    } catch (...) {
        return false;
    }
}

void check_inactivity(Account& acc) {
    if (acc.last_transaction_date.empty()) return;

    string last_date_str = acc.last_transaction_date.substr(0, 10); // Get YYYY-MM-DD part
    
    // Parse the date
    tm last_active = {};
    istringstream iss(last_date_str);
    char dash1, dash2;
    int year, month, day;
    
    iss >> year >> dash1 >> month >> dash2 >> day;
    
    if (iss.fail() || dash1 != '-' || dash2 != '-') return;

    last_active.tm_year = year - 1900;
    last_active.tm_mon = month - 1;
    last_active.tm_mday = day;
    last_active.tm_hour = 0;
    last_active.tm_min = 0;
    last_active.tm_sec = 0;

    time_t last_time = mktime(&last_active);
    time_t now = time(0);
    
    double diff = difftime(now, last_time);
    if (diff > INACTIVITY_LOCK_DAYS * 24 * 60 * 60) {
        acc.status = "LOCKED";
        acc.failed_attempts = MAX_FAILED_ATTEMPTS;
        log_audit_event("Account " + acc.account_number + " locked due to inactivity");
    }
}

void reset_daily_withdrawal_if_new_day(Account& acc) {
    string today = get_current_date();
    if (!acc.last_transaction_date.empty() && acc.last_transaction_date.substr(0, 10) != today) {
        acc.daily_withdrawal_total = 0.0;
        acc.last_transaction_date = today + " 00:00:00";
    }
}

Account get_user_input() {
    Account new_acc;
    
    cout << "\nCREATE NEW ACCOUNT\n";
    cout << "---------------------\n";
    
    cout << "Enter your name: ";
    getline(cin, new_acc.name);
    while (new_acc.name.empty()) {
        cout << "Name cannot be empty. Please enter your name: ";
        getline(cin, new_acc.name);
    }
    
    while (true) {
        cout << "Enter your gender (M/F/O): ";
        getline(cin, new_acc.gender);
        transform(new_acc.gender.begin(), new_acc.gender.end(), new_acc.gender.begin(), ::toupper);
        if (new_acc.gender == "M" || new_acc.gender == "F" || new_acc.gender == "O") break;
        cout << "Invalid input. Please enter M, F, or O.\n";
    }
    
    while (true) {
        cout << "Enter your 10-digit phone number: ";
        getline(cin, new_acc.phone_number);
        if (validate_phone_number(new_acc.phone_number)) {
            break;
        }
        cout << "Invalid phone number. Please enter a 10-digit number.\n";
    }
    
    while (true) {
        cout << "Enter your age: ";
        string age_input;
        getline(cin, age_input);
        try {
            new_acc.age = stoi(age_input);
            if (new_acc.age < 18) {
                cout << "You must be at least 18 years old to open an account.\n";
            } else {
                break;
            }
        } catch (...) {
            cout << "Invalid age. Please enter a valid number.\n";
        }
    }
    
    while (true) {
        cout << "Enter account type (Savings/Current): ";
        getline(cin, new_acc.account_type);
        transform(new_acc.account_type.begin(), new_acc.account_type.end(), new_acc.account_type.begin(), ::toupper);
        if (new_acc.account_type == "SAVINGS" || new_acc.account_type == "CURRENT") break;
        cout << "Invalid account type. Please enter Savings or Current.\n";
    }
    
    while (true) {
        cout << "Enter your email address: ";
        getline(cin, new_acc.email);
        if (validate_email(new_acc.email)) break;
        cout << "Invalid email address. Please enter a valid email.\n";
    }
    
    cout << "Enter your residential address: ";
    getline(cin, new_acc.address);
    while (new_acc.address.empty()) {
        cout << "Address cannot be empty. Please enter your address: ";
        getline(cin, new_acc.address);
    }
    
    // Generate account details
    new_acc.account_number = generate_account_number();
    
    // Generate and confirm PIN
    string pin, pin_confirm;
    while (true) {
        cout << "Create a 4-digit PIN: ";
        getline(cin, pin);
        
        if (!is_strong_pin(pin)) {
            cout << "PIN must be 4 digits and not all the same number.\n";
            continue;
        }
        
        cout << "Confirm your 4-digit PIN: ";
        getline(cin, pin_confirm);
        
        if (pin != pin_confirm) {
            cout << "PINs do not match. Please try again.\n";
        } else {
            new_acc.pin_hash = hash_pin(pin);
            break;
        }
    }
    
    new_acc.cif_number = generate_cif_number();
    new_acc.ifsc_code = generate_ifsc_code();
    new_acc.micr_code = generate_micr_code();
    new_acc.branch_name = "Main Branch";
    new_acc.branch_address = "123 Bank Street, Financial District";
    new_acc.balance = 0.0;
    new_acc.status = "ACTIVE";
    new_acc.opening_date = get_current_datetime();
    new_acc.failed_attempts = 0;
    new_acc.last_transaction_date = "";
    new_acc.daily_withdrawal_total = 0.0;
    
    return new_acc;
}

void record_transaction(const string& acc_number, const string& type, 
                       double amount, const string& description,
                       const string& from, const string& to) {
    Transaction tx;
    tx.account_number = acc_number;
    tx.type = type;
    tx.from_account = from;
    tx.to_account = to;
    tx.amount = amount;
    tx.timestamp = get_current_datetime();
    tx.description = description;
    
    transactions.push_back(tx);
    save_transaction_to_file(tx);
    
    // Update last transaction date
    if (accounts.find(acc_number) != accounts.end()) {
        accounts[acc_number].last_transaction_date = tx.timestamp;
    }
}

double get_balance_after_transaction(const string& acc_number, const string& timestamp) {
    double balance = 0.0;
    for (const auto& tx : transactions) {
        if (tx.account_number == acc_number) {
            if (tx.type == "DEPOSIT" || tx.type == "TRANSFER_IN") {
                balance += tx.amount;
            } else if (tx.type == "WITHDRAWAL" || tx.type == "TRANSFER_OUT") {
                balance -= tx.amount;
            }
            if (tx.timestamp == timestamp) {
                break;
            }
        }
    }
    return balance;
}

// TransactionGuard implementation
void TransactionGuard::add_rollback_action(function<void()> action) {
    rollback_actions.push_back(action);
}

void TransactionGuard::commit() { 
    committed = true; 
}

TransactionGuard::~TransactionGuard() {
    if (!committed) {
        for (auto it = rollback_actions.rbegin(); it != rollback_actions.rend(); ++it) {
            try {
                (*it)();
            } catch (...) {
                // Ignore rollback errors to prevent throwing during stack unwinding
            }
        }
    }
}