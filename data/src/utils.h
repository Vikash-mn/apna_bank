#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <functional>
#include <vector>
#include "banking.h"

// Utility functions
void clear_screen();
std::string get_current_date();
std::string get_current_datetime();
std::string generate_random_string(int length, const std::string& charset);
std::string generate_account_number();
std::string generate_cif_number();
std::string generate_ifsc_code();
std::string generate_micr_code();
bool validate_phone_number(const std::string& phone);
bool validate_email(const std::string& email);
bool validate_amount(const std::string& amount_str);
void check_inactivity(Account& acc);
void reset_daily_withdrawal_if_new_day(Account& acc);
Account get_user_input();
void record_transaction(const std::string& acc_number, const std::string& type, 
                       double amount, const std::string& description,
                       const std::string& from = "", const std::string& to = "");
double get_balance_after_transaction(const std::string& acc_number, const std::string& timestamp);

// Transaction safety
class TransactionGuard {
private:
    bool committed = false;
    std::vector<std::function<void()>> rollback_actions;
public:
    void add_rollback_action(std::function<void()> action);
    void commit();
    ~TransactionGuard();
};

#endif