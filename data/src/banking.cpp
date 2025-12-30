#include "banking.h"
#include "utils.h"
#include "security.h"
#include "file_operations.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <iomanip>
#include <sstream>

using namespace std;

// Global data stores
map<string, Account> accounts;
vector<Transaction> transactions;
void initialize_system() {
    srand(time(0));
    create_data_directory();
    load_accounts_from_file();
    load_transactions_from_file();
    log_audit_event("System initialized");
}

void display_main_menu() {
    clear_screen();
    cout << "APNA BANK - MAIN MENU\n";
    cout << "----------------------\n";
    cout << "1. Create New Account\n";
    cout << "2. Deposit Money\n";
    cout << "3. Withdraw Money\n";
    cout << "4. Transfer Money\n";
    cout << "5. View Account Details\n";
    cout << "6. View Transaction History\n";
    cout << "7. Change PIN\n";
    cout << "8. Close Account\n";
    cout << "9. Generate Account Statement\n";
    cout << "10. Calculate Interest\n";
    cout << "11. Pay Bills\n";
    cout << "12. Exit\n";
    cout << "\nEnter your choice: ";
}

void create_account() {
    clear_screen();
    Account new_acc = get_user_input();
    
    accounts[new_acc.account_number] = new_acc;
    save_all_accounts_to_file();
    log_audit_event("Account created: " + new_acc.account_number + " for " + new_acc.name);
    
    cout << "\nACCOUNT CREATED SUCCESSFULLY!\n";
    cout << "--------------------------------\n";
    cout << "Account Number: " << new_acc.account_number << "\n";
    cout << "CIF Number: " << new_acc.cif_number << "\n";
    cout << "IFSC Code: " << new_acc.ifsc_code << "\n";
    cout << "Branch: " << new_acc.branch_name << "\n";
    cout << "Opening Date: " << new_acc.opening_date << "\n";
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void deposit() {
    clear_screen();
    cout << "DEPOSIT MONEY\n";
    cout << "----------------\n";
    
    string acc_number, pin;
    cout << "Enter your account number: ";
    getline(cin, acc_number);
    cout << "Enter your PIN: ";
    getline(cin, pin);
    
    if (!validate_account(acc_number, pin)) {
        cout << "Authentication failed. Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    Account& acc = accounts[acc_number];
    double amount;
    
    while (true) {
        cout << "Enter amount to deposit (min " << MIN_DEPOSIT << ", max " << MAX_DEPOSIT << "): ";
        string amount_input;
        getline(cin, amount_input);
        
        if (!validate_amount(amount_input)) {
            cout << "Invalid amount. Please enter a positive number.\n";
            continue;
        }
        
        try {
            amount = stod(amount_input);
            
            if (amount < MIN_DEPOSIT || amount > MAX_DEPOSIT) {
                cout << "Amount must be between ₹" << MIN_DEPOSIT << " and ₹" << MAX_DEPOSIT << ".\n";
                continue;
            }
            
            TransactionGuard guard;
            double original_balance = acc.balance;
            guard.add_rollback_action([&]() { acc.balance = original_balance; });
            
            acc.balance += amount;
            record_transaction(acc_number, "DEPOSIT", amount, "Cash deposit");
            save_all_accounts_to_file();
            log_audit_event("Deposit of " + to_string(amount) + " to account: " + acc_number);
            
            guard.commit();
            
            cout << "\nDeposit successful!\n";
            cout << "New balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";
            break;
        } catch (...) {
            cout << "Invalid amount. Please enter a valid number.\n";
        }
    }
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void withdraw() {
    clear_screen();
    cout << "WITHDRAW MONEY\n";
    cout << "----------------\n";

    string acc_number, pin;
    cout << "Enter your account number: ";
    getline(cin, acc_number);
    cout << "Enter your PIN: ";
    getline(cin, pin);

    if (!validate_account(acc_number, pin)) {
        cout << "Authentication failed. Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    Account& acc = accounts[acc_number];
    reset_daily_withdrawal_if_new_day(acc);

    cout << "Current balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";

    double amount;
    while (true) {
        cout << "Enter amount to withdraw (min " << MIN_WITHDRAWAL << "): ";
        string amount_input;
        getline(cin, amount_input);
        
        if (!validate_amount(amount_input)) {
            cout << "Invalid amount. Please enter a positive number.\n";
            continue;
        }
        
        try {
            amount = stod(amount_input);
            
            if (amount < MIN_WITHDRAWAL) {
                cout << "Minimum withdrawal amount is ₹" << MIN_WITHDRAWAL << ".\n";
                continue;
            }
            
            if (amount > acc.balance) {
                cout << "Insufficient balance. Current balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";
                continue;
            }
            
            if ((acc.daily_withdrawal_total + amount) > DAILY_WITHDRAWAL_LIMIT) {
                cout << "This withdrawal would exceed your daily limit of ₹" << DAILY_WITHDRAWAL_LIMIT << ".\n";
                cout << "Already withdrawn today: ₹" << fixed << setprecision(2) << acc.daily_withdrawal_total << "\n";
                continue;
            }
            
            TransactionGuard guard;
            double original_balance = acc.balance;
            double original_withdrawal_total = acc.daily_withdrawal_total;
            
            guard.add_rollback_action([&]() {
                acc.balance = original_balance;
                acc.daily_withdrawal_total = original_withdrawal_total;
            });
            
            acc.balance -= amount;
            acc.daily_withdrawal_total += amount;
            record_transaction(acc_number, "WITHDRAWAL", amount, "Cash withdrawal");
            save_all_accounts_to_file();
            log_audit_event("Withdrawal of " + to_string(amount) + " from account: " + acc_number);
            
            guard.commit();
            
            cout << "\nWithdrawal successful!\n";
            cout << "New balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";
            cout << "Remaining daily withdrawal limit: ₹" << fixed << setprecision(2) 
                 << (DAILY_WITHDRAWAL_LIMIT - acc.daily_withdrawal_total) << "\n";
            break;
        } catch (...) {
            cout << "Invalid amount. Please enter a valid number.\n";
        }
    }
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void transfer_money() {
    clear_screen();
    cout << "TRANSFER MONEY\n";
    cout << "----------------\n";
    
    string sender_acc, pin;
    cout << "Enter your account number: ";
    getline(cin, sender_acc);
    cout << "Enter your PIN: ";
    getline(cin, pin);
    
    if (!validate_account(sender_acc, pin)) {
        cout << "Authentication failed. Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    Account& sender = accounts[sender_acc];
    string recipient_acc;
    
    while (true) {
        cout << "Enter recipient account number: ";
        getline(cin, recipient_acc);
        
        if (recipient_acc == sender_acc) {
            cout << "You cannot transfer to your own account!\n";
            continue;
        }
        
        if (accounts.find(recipient_acc) == accounts.end()) {
            cout << "Recipient account not found.\n";
            continue;
        }
        
        break;
    }
    
    Account& recipient = accounts[recipient_acc];
    cout << "\nRecipient: " << recipient.name << "\n";
    cout << "Your current balance: ₹" << fixed << setprecision(2) << sender.balance << "\n";
    
    double amount;
    while (true) {
        cout << "Enter transfer amount: ";
        string amount_input;
        getline(cin, amount_input);
        
        if (!validate_amount(amount_input)) {
            cout << "Invalid amount. Please enter a positive number.\n";
            continue;
        }
        
        try {
            amount = stod(amount_input);
            
            if (amount <= 0) {
                cout << "Amount must be positive.\n";
                continue;
            }
            
            if (amount > sender.balance) {
                cout << "Insufficient balance. Current balance: ₹" << fixed << setprecision(2) << sender.balance << "\n";
                continue;
            }
            
            TransactionGuard guard;
            double original_sender_balance = sender.balance;
            double original_recipient_balance = recipient.balance;
            
            guard.add_rollback_action([&]() {
                sender.balance = original_sender_balance;
                recipient.balance = original_recipient_balance;
            });
            
            sender.balance -= amount;
            recipient.balance += amount;
            
            record_transaction(sender_acc, "TRANSFER_OUT", amount, 
                             "Transfer to " + recipient.name, sender_acc, recipient_acc);
            record_transaction(recipient_acc, "TRANSFER_IN", amount, 
                             "Transfer from " + sender.name, sender_acc, recipient_acc);
            
            save_all_accounts_to_file();
            log_audit_event("Transfer of " + to_string(amount) + " from " + sender_acc + " to " + recipient_acc);
            
            guard.commit();
            
            cout << "\nTransfer successful!\n";
            cout << "Sent: ₹" << fixed << setprecision(2) << amount << " to " << recipient.name << "\n";
            cout << "Your new balance: ₹" << fixed << setprecision(2) << sender.balance << "\n";
            break;
        } catch (...) {
            cout << "Invalid amount. Please enter a valid number.\n";
        }
    }
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void view_account_details() {
    clear_screen();
    cout << "ACCOUNT DETAILS\n";
    cout << "----------------\n";
    
    string acc_number, pin;
    cout << "Enter your account number: ";
    getline(cin, acc_number);
    cout << "Enter your PIN: ";
    getline(cin, pin);
    
    if (!validate_account(acc_number, pin)) {
        cout << "Authentication failed. Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    Account& acc = accounts[acc_number];
    
    cout << "\nAccount Details:\n";
    cout << "----------------\n";
    cout << "Account Number: " << acc.account_number << "\n";
    cout << "Name: " << acc.name << "\n";
    cout << "Account Type: " << acc.account_type << "\n";
    cout << "CIF Number: " << acc.cif_number << "\n";
    cout << "IFSC Code: " << acc.ifsc_code << "\n";
    cout << "MICR Code: " << acc.micr_code << "\n";
    cout << "Branch: " << acc.branch_name << "\n";
    cout << "Branch Address: " << acc.branch_address << "\n";
    cout << "Opening Date: " << acc.opening_date << "\n";
    cout << "Status: " << acc.status << "\n";
    cout << "Current Balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";
    
    if (!acc.last_transaction_date.empty() && acc.last_transaction_date.substr(0, 10) == get_current_date()) {
        cout << "Withdrawn today: ₹" << fixed << setprecision(2) << acc.daily_withdrawal_total << "\n";
        cout << "Remaining daily limit: ₹" << fixed << setprecision(2)
             << (DAILY_WITHDRAWAL_LIMIT - acc.daily_withdrawal_total) << "\n";
    }
    
    log_audit_event("Account details viewed for: " + acc_number);
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void view_transaction_history() {
    clear_screen();
    cout << "TRANSACTION HISTORY\n";
    cout << "--------------------\n";
    
    string acc_number, pin;
    cout << "Enter your account number: ";
    getline(cin, acc_number);
    cout << "Enter your PIN: ";
    getline(cin, pin);
    
    if (!validate_account(acc_number, pin)) {
        cout << "Authentication failed. Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    
    cout << "\nTransaction History for Account: " << acc_number << "\n";
    cout << "--------------------------------------------\n";
    
    bool found = false;
    for (const auto& tx : transactions) {
        if (tx.account_number == acc_number) {
            found = true;
            cout << "Date: " << tx.timestamp << "\n";
            cout << "Type: " << tx.type << "\n";
            if (!tx.from_account.empty()) cout << "From: " << tx.from_account << "\n";
            if (!tx.to_account.empty()) cout << "To: " << tx.to_account << "\n";
            cout << "Amount: ₹" << fixed << setprecision(2) << tx.amount << "\n";
            cout << "Description: " << tx.description << "\n";
            cout << "--------------------------------------------\n";
        }
    }
    
    if (!found) {
        cout << "No transactions found for this account.\n";
    }
    
    log_audit_event("Transaction history viewed for: " + acc_number);
    
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void change_pin() {
    clear_screen();
    cout << "CHANGE PIN\n";
    cout << "-----------\n";

    string acc_number, old_pin, new_pin;
    cout << "Enter your account number: ";
    getline(cin, acc_number);
    cout << "Enter your current PIN: ";
    getline(cin, old_pin);

    if (!validate_account(acc_number, old_pin)) {
        cout << "Authentication failed. Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    Account& acc = accounts[acc_number];

    while (true) {
        cout << "Enter new 4-digit PIN: ";
        getline(cin, new_pin);

        if (!is_strong_pin(new_pin)) {
            cout << "PIN must be 4 digits and not all the same number.\n";
            continue;
        }

        if (hash_pin(new_pin) == acc.pin_hash) {
            cout << "New PIN cannot be the same as old PIN.\n";
            continue;
        }

        string confirm_pin;
        cout << "Confirm new 4-digit PIN: ";
        getline(cin, confirm_pin);

        if (new_pin != confirm_pin) {
            cout << "PINs do not match. Please try again.\n";
            continue;
        }

        break;
    }

    TransactionGuard guard;
    string original_pin_hash = acc.pin_hash;
    guard.add_rollback_action([&]() { acc.pin_hash = original_pin_hash; });

    acc.pin_hash = hash_pin(new_pin);
    save_all_accounts_to_file();
    log_audit_event("PIN changed for account: " + acc_number);

    guard.commit();

    cout << "\nPIN changed successfully!\n";
    cout << "Press Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void close_account() {
    clear_screen();
    cout << "CLOSE ACCOUNT\n";
    cout << "-------------\n";

    string acc_number, pin;
    cout << "Enter your account number: ";
    getline(cin, acc_number);
    cout << "Enter your PIN: ";
    getline(cin, pin);

    if (!validate_account(acc_number, pin)) {
        cout << "Authentication failed. Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    Account& acc = accounts[acc_number];

    if (acc.balance > 0) {
        cout << "\nCannot close account with positive balance.\n";
        cout << "Current balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";
        cout << "Please withdraw all funds before closing the account.\n";
        cout << "Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << "\nAccount Details:\n";
    cout << "----------------\n";
    cout << "Account Number: " << acc.account_number << "\n";
    cout << "Name: " << acc.name << "\n";
    cout << "Balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";

    string confirmation;
    cout << "\nAre you sure you want to close this account? (yes/no): ";
    getline(cin, confirmation);
    transform(confirmation.begin(), confirmation.end(), confirmation.begin(), ::tolower);

    if (confirmation != "yes" && confirmation != "y") {
        cout << "Account closure cancelled.\n";
        cout << "Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    // Record final transaction
    record_transaction(acc_number, "ACCOUNT_CLOSED", 0.0, "Account closed by customer");

    // Remove account from memory
    accounts.erase(acc_number);

    // Save updated accounts file
    save_all_accounts_to_file();

    log_audit_event("Account closed: " + acc_number + " by " + acc.name);

    cout << "\nAccount closed successfully!\n";
    cout << "Thank you for banking with us.\n";
    cout << "Press Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void generate_account_statement() {
    clear_screen();
    cout << "ACCOUNT STATEMENT\n";
    cout << "------------------\n";

    string acc_number, pin;
    cout << "Enter your account number: ";
    getline(cin, acc_number);
    cout << "Enter your PIN: ";
    getline(cin, pin);

    if (!validate_account(acc_number, pin)) {
        cout << "Authentication failed. Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    Account& acc = accounts[acc_number];

    cout << "\nAPNA BANK - ACCOUNT STATEMENT\n";
    cout << "==============================\n";
    cout << "Account Number: " << acc.account_number << "\n";
    cout << "Account Holder: " << acc.name << "\n";
    cout << "Account Type: " << acc.account_type << "\n";
    cout << "Statement Date: " << get_current_datetime() << "\n";
    cout << "Current Balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";
    cout << "\nRecent Transactions:\n";
    cout << "-------------------\n";

    bool found = false;
    int count = 0;
    for (auto it = transactions.rbegin(); it != transactions.rend() && count < 10; ++it) {
        const Transaction& tx = *it;
        if (tx.account_number == acc_number) {
            found = true;
            cout << "Date: " << tx.timestamp << "\n";
            cout << "Type: " << tx.type << "\n";
            cout << "Amount: ₹" << fixed << setprecision(2) << tx.amount << "\n";
            cout << "Description: " << tx.description << "\n";
            cout << "Balance after transaction: ₹" << fixed << setprecision(2) << get_balance_after_transaction(acc_number, tx.timestamp) << "\n";
            cout << "---\n";
            count++;
        }
    }

    if (!found) {
        cout << "No transactions found.\n";
    }

    log_audit_event("Account statement generated for: " + acc_number);

    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void calculate_interest() {
    clear_screen();
    cout << "CALCULATE INTEREST\n";
    cout << "-------------------\n";

    string acc_number, pin;
    cout << "Enter your account number: ";
    getline(cin, acc_number);
    cout << "Enter your PIN: ";
    getline(cin, pin);

    if (!validate_account(acc_number, pin)) {
        cout << "Authentication failed. Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    Account& acc = accounts[acc_number];

    if (acc.account_type != "SAVINGS") {
        cout << "\nInterest calculation is only available for Savings accounts.\n";
        cout << "Your account type: " << acc.account_type << "\n";
        cout << "Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    // Simple interest calculation (4% annual interest)
    const double INTEREST_RATE = 0.04;
    double interest = acc.balance * INTEREST_RATE;

    cout << "\nInterest Calculation:\n";
    cout << "--------------------\n";
    cout << "Account Number: " << acc.account_number << "\n";
    cout << "Current Balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";
    cout << "Annual Interest Rate: " << (INTEREST_RATE * 100) << "%\n";
    cout << "Estimated Annual Interest: ₹" << fixed << setprecision(2) << interest << "\n";
    cout << "Balance after interest: ₹" << fixed << setprecision(2) << (acc.balance + interest) << "\n";

    string apply_interest;
    cout << "\nDo you want to apply the interest to your account? (yes/no): ";
    getline(cin, apply_interest);
    transform(apply_interest.begin(), apply_interest.end(), apply_interest.begin(), ::tolower);

    if (apply_interest == "yes" || apply_interest == "y") {
        TransactionGuard guard;
        double original_balance = acc.balance;
        guard.add_rollback_action([&]() { acc.balance = original_balance; });

        acc.balance += interest;
        record_transaction(acc_number, "INTEREST", interest, "Annual interest applied");
        save_all_accounts_to_file();
        log_audit_event("Interest of " + to_string(interest) + " applied to account: " + acc_number);

        guard.commit();

        cout << "\nInterest applied successfully!\n";
        cout << "New balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";
    } else {
        cout << "\nInterest calculation completed. No changes made to account.\n";
    }

    cout << "Press Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void pay_bills() {
    clear_screen();
    cout << "PAY BILLS\n";
    cout << "----------\n";

    string acc_number, pin;
    cout << "Enter your account number: ";
    getline(cin, acc_number);
    cout << "Enter your PIN: ";
    getline(cin, pin);

    if (!validate_account(acc_number, pin)) {
        cout << "Authentication failed. Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    Account& acc = accounts[acc_number];

    cout << "\nAvailable Bill Types:\n";
    cout << "1. Electricity Bill\n";
    cout << "2. Water Bill\n";
    cout << "3. Gas Bill\n";
    cout << "4. Internet Bill\n";
    cout << "5. Phone Bill\n";
    cout << "6. Other\n";

    string choice_input;
    cout << "\nEnter bill type (1-6): ";
    getline(cin, choice_input);

    string bill_type;
    try {
        int choice = stoi(choice_input);
        switch (choice) {
            case 1: bill_type = "Electricity"; break;
            case 2: bill_type = "Water"; break;
            case 3: bill_type = "Gas"; break;
            case 4: bill_type = "Internet"; break;
            case 5: bill_type = "Phone"; break;
            case 6: bill_type = "Other"; break;
            default:
                cout << "Invalid choice.\n";
                cout << "Press Enter to continue...";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return;
        }
    } catch (...) {
        cout << "Invalid choice.\n";
        cout << "Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    string bill_number;
    cout << "Enter bill number/reference: ";
    getline(cin, bill_number);

    double amount;
    while (true) {
        cout << "Enter bill amount: ";
        string amount_input;
        getline(cin, amount_input);

        if (!validate_amount(amount_input)) {
            cout << "Invalid amount. Please enter a positive number.\n";
            continue;
        }

        try {
            amount = stod(amount_input);
            if (amount <= 0) {
                cout << "Amount must be positive.\n";
                continue;
            }
            if (amount > acc.balance) {
                cout << "Insufficient balance. Current balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";
                continue;
            }
            break;
        } catch (...) {
            cout << "Invalid amount. Please enter a valid number.\n";
        }
    }

    TransactionGuard guard;
    double original_balance = acc.balance;
    guard.add_rollback_action([&]() { acc.balance = original_balance; });

    acc.balance -= amount;
    record_transaction(acc_number, "BILL_PAYMENT", amount, bill_type + " bill payment - " + bill_number);
    save_all_accounts_to_file();
    log_audit_event("Bill payment of " + to_string(amount) + " for " + bill_type + " from account: " + acc_number);

    guard.commit();

    cout << "\nBill payment successful!\n";
    cout << bill_type << " bill of ₹" << fixed << setprecision(2) << amount << " paid.\n";
    cout << "Reference: " << bill_number << "\n";
    cout << "New balance: ₹" << fixed << setprecision(2) << acc.balance << "\n";

    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}