#include <iostream>
#include <string>
#include <limits>
#include "banking.h"
#include "utils.h"
#include "security.h"
#include "file_operations.h"
using namespace std;
int main() {
    initialize_system();
    while (true) {
        display_main_menu();
        string choice_input;
        getline(cin, choice_input);
        try {
            int choice = stoi(choice_input);
            switch (choice) {
                case 1: create_account(); break;
                case 2: deposit(); break;
                case 3: withdraw(); break;
                case 4: transfer_money(); break;
                case 5: view_account_details(); break;
                case 6: view_transaction_history(); break;
                case 7: change_pin(); break;
                case 8: close_account(); break;
                case 9: generate_account_statement(); break;
                case 10: calculate_interest(); break;
                case 11: pay_bills(); break;
                case 12:
                    cout << "\nThank you for banking with us!\n";
                    log_audit_event("System shutdown");
                    return 0;
                default:
                    cout << "Invalid choice. Please try again.\n";
                    cout << "Press Enter to continue...";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        } catch (...) {
            cout << "Invalid input. Please enter a number.\n";
            cout << "Press Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    return 0;
}