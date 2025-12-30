#include "file_operations.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <direct.h>
#include <sys/stat.h>
#include <algorithm>

using namespace std;

void create_data_directory() {
    #ifdef _WIN32
        _mkdir(DATA_DIR.c_str());
    #else
        mkdir(DATA_DIR.c_str(), 0777);
    #endif
}

void log_audit_event(const string& event) {
    try {
        ofstream audit_log(AUDIT_LOG_FILE, ios::app);
        if (audit_log.is_open()) {
            audit_log << get_current_datetime() << " - " << event << "\n";
            audit_log.close();
        }
    } catch (const exception& e) {
        cerr << "Audit log error: " << e.what() << endl;
    }
}

void load_accounts_from_file() {
    try {
        ifstream file(ACCOUNTS_FILE);
        if (!file.is_open()) {
            cerr << "Warning: Could not open accounts file. Starting with empty database.\n";
            return;
        }

        string line;
        // Skip header
        getline(file, line);

        while (getline(file, line)) {
            istringstream iss(line);
            string token;
            Account acc;

            try {
                getline(iss, acc.account_number, ';');
                getline(iss, acc.name, ';');
                getline(iss, acc.pin_hash, ';');
                getline(iss, acc.gender, ';');
                getline(iss, acc.phone_number, ';');

                iss >> acc.age;
                iss.ignore();

                getline(iss, acc.account_type, ';');
                getline(iss, acc.email, ';');
                getline(iss, acc.address, ';');
                getline(iss, acc.cif_number, ';');
                getline(iss, acc.ifsc_code, ';');
                getline(iss, acc.micr_code, ';');
                getline(iss, acc.branch_name, ';');
                getline(iss, acc.branch_address, ';');

                iss >> acc.balance;
                iss.ignore();

                getline(iss, acc.status, ';');
                getline(iss, acc.opening_date, ';');

                iss >> acc.failed_attempts;
                iss.ignore();

                getline(iss, acc.last_transaction_date, ';');
                iss >> acc.daily_withdrawal_total;

                accounts[acc.account_number] = acc;
            } catch (const exception& e) {
                cerr << "Error parsing account data: " << e.what() << endl;
                continue;
            }
        }
        file.close();
    } catch (const exception& e) {
        cerr << "Error loading accounts: " << e.what() << endl;
    }
}

void load_transactions_from_file() {
    try {
        ifstream file(TRANSACTIONS_FILE);
        if (!file.is_open()) {
            cerr << "Warning: Could not open transactions file. Starting with empty transaction history.\n";
            return;
        }

        string line;
        // Skip header
        getline(file, line);

        while (getline(file, line)) {
            try {
                istringstream iss(line);
                Transaction tx;

                getline(iss, tx.account_number, ';');
                getline(iss, tx.type, ';');
                getline(iss, tx.from_account, ';');
                getline(iss, tx.to_account, ';');
                iss >> tx.amount;
                iss.ignore();
                getline(iss, tx.timestamp, ';');
                getline(iss, tx.description);

                transactions.push_back(tx);
            } catch (const exception& e) {
                cerr << "Error parsing transaction data: " << e.what() << endl;
                continue;
            }
        }
        file.close();
    } catch (const exception& e) {
        cerr << "Error loading transactions: " << e.what() << endl;
    }
}

void save_all_accounts_to_file() {
    try {
        ofstream file(ACCOUNTS_FILE);
        if (!file.is_open()) {
            cerr << "Error opening accounts file for writing!" << endl;
            return;
        }

        try {
            file << "account_number;name;pin_hash;gender;phone_number;age;account_type;email;address;"
                 << "cif_number;ifsc_code;micr_code;branch_name;branch_address;balance;status;opening_date;"
                 << "failed_attempts;last_transaction_date;daily_withdrawal_total\n";

            for (const auto& pair : accounts) {
                const Account& acc = pair.second;
                file << acc.account_number << ";"
                     << acc.name << ";"
                     << acc.pin_hash << ";"
                     << acc.gender << ";"
                     << acc.phone_number << ";"
                     << acc.age << ";"
                     << acc.account_type << ";"
                     << acc.email << ";"
                     << acc.address << ";"
                     << acc.cif_number << ";"
                     << acc.ifsc_code << ";"
                     << acc.micr_code << ";"
                     << acc.branch_name << ";"
                     << acc.branch_address << ";"
                     << acc.balance << ";"
                     << acc.status << ";"
                     << acc.opening_date << ";"
                     << acc.failed_attempts << ";"
                     << acc.last_transaction_date << ";"
                     << acc.daily_withdrawal_total << "\n";
            }
        } catch (const exception& e) {
            cerr << "Error writing to accounts file: " << e.what() << endl;
        }

        file.close();
    } catch (const exception& e) {
        cerr << "Error saving accounts: " << e.what() << endl;
    }
}

void save_transaction_to_file(const Transaction& tx) {
    try {
        ofstream file(TRANSACTIONS_FILE, ios::app);
        if (!file.is_open()) {
            cerr << "Error opening transactions file for writing!" << endl;
            return;
        }

        try {
            if (file.tellp() == 0) {
                file << "account_number;type;from_account;to_account;amount;timestamp;description\n";
            }

            file << tx.account_number << ";"
                 << tx.type << ";"
                 << tx.from_account << ";"
                 << tx.to_account << ";"
                 << tx.amount << ";"
                 << tx.timestamp << ";"
                 << tx.description << "\n";
        } catch (const exception& e) {
            cerr << "Error writing transaction: " << e.what() << endl;
        }

        file.close();
    } catch (const exception& e) {
        cerr << "Error saving transaction: " << e.what() << endl;
    }
}