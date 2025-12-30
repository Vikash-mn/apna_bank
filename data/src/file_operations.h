#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <string>
#include "banking.h"

// File operations
void create_data_directory();
void log_audit_event(const std::string& event);
void load_accounts_from_file();
void load_transactions_from_file();
void save_all_accounts_to_file();
void save_transaction_to_file(const Transaction& tx);

#endif