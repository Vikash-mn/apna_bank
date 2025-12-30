#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <functional>
#include "../src/banking.h"
#include "../src/security.h"
#include "../src/utils.h"

using namespace std;

// Test function prototypes
void test_pin_validation();
void test_hashing();
void test_validation_functions();
void test_utility_functions();
void test_account_operations();
void test_transaction_operations();
void test_edge_cases();

// Global test counters
int tests_passed = 0;
int tests_failed = 0;

void run_test(const string& test_name, function<void()> test_func) {
    cout << "🧪 Testing: " << test_name << "... ";
    try {
        test_func();
        cout << "✅ PASSED" << endl;
        tests_passed++;
    } catch (const exception& e) {
        cout << "❌ FAILED: " << e.what() << endl;
        tests_failed++;
    } catch (...) {
        cout << "❌ FAILED: Unknown error" << endl;
        tests_failed++;
    }
}

void test_pin_validation() {
    // Test valid PINs
    assert(is_strong_pin("1234") == true);
    assert(is_strong_pin("5678") == true);
    assert(is_strong_pin("9876") == true);
    
    // Test invalid PINs (all same digits)
    assert(is_strong_pin("1111") == false);
    assert(is_strong_pin("2222") == false);
    assert(is_strong_pin("0000") == false);
    
    // Test invalid lengths
    assert(is_strong_pin("123") == false);
    assert(is_strong_pin("12345") == false);
    assert(is_strong_pin("") == false);
    
    // Test non-digit PINs
    assert(is_strong_pin("abcd") == false);
    assert(is_strong_pin("12a4") == false);
}

void test_hashing() {
    string pin1 = "1234";
    string pin2 = "1234";
    string pin3 = "5678";
    
    // Same PIN should produce same hash
    assert(hash_pin(pin1) == hash_pin(pin2));
    
    // Different PINs should produce different hashes
    assert(hash_pin(pin1) != hash_pin(pin3));
    
    // Hash should not be empty
    assert(!hash_pin(pin1).empty());
    
    // Hash should be consistent
    string hash1 = hash_pin(pin1);
    string hash2 = hash_pin(pin1);
    assert(hash1 == hash2);
}

void test_validation_functions() {
    // Phone number validation
    assert(validate_phone_number("1234567890") == true);
    assert(validate_phone_number("9876543210") == true);
    assert(validate_phone_number("12345") == false); // Too short
    assert(validate_phone_number("12345678901") == false); // Too long
    assert(validate_phone_number("abcdefghij") == false); // Non-digits
    assert(validate_phone_number("123 456 789") == false); // Spaces
    
    // Email validation
    assert(validate_email("test@example.com") == true);
    assert(validate_email("user.name@domain.co.uk") == true);
    assert(validate_email("invalid-email") == false);
    assert(validate_email("missing@domain") == false);
    assert(validate_email("@domain.com") == false);
    assert(validate_email("user@.com") == false);
    
    // Amount validation
    assert(validate_amount("100.50") == true);
    assert(validate_amount("500") == true);
    assert(validate_amount("0") == false); // Zero amount
    assert(validate_amount("-100") == false); // Negative amount
    assert(validate_amount("abc") == false); // Non-numeric
    assert(validate_amount("") == false); // Empty string
    assert(validate_amount("100.123") == false); // Too many decimals
}

void test_utility_functions() {
    // Date generation
    string date = get_current_date();
    assert(date.length() == 10); // YYYY-MM-DD format
    assert(date[4] == '-');
    assert(date[7] == '-');
    
    // DateTime generation
    string datetime = get_current_datetime();
    assert(datetime.length() > 16); // Includes time
    assert(datetime.find(' ') != string::npos); // Contains space between date and time
    
    // Random string generation
    string numbers = generate_random_string(10, "0123456789");
    assert(numbers.length() == 10);
    
    // Check if all characters are from the specified charset
    for (char c : numbers) {
        assert(isdigit(c));
    }
    
    string letters = generate_random_string(5, "ABCDE");
    assert(letters.length() == 5);
    for (char c : letters) {
        assert(c >= 'A' && c <= 'E');
    }
    
    // Account number generation (basic test)
    string acc1 = generate_account_number();
    string acc2 = generate_account_number();
    assert(acc1.length() == 16); // "APNA" + 12 digits
    assert(acc2.length() == 16);
    assert(acc1.substr(0, 4) == "APNA");
    assert(acc2.substr(0, 4) == "APNA");
    
    // CIF, IFSC, MICR generation
    string cif = generate_cif_number();
    string ifsc = generate_ifsc_code();
    string micr = generate_micr_code();
    
    assert(cif.length() == 10);
    assert(ifsc.length() == 11);
    assert(micr.length() == 9);
}

void test_account_operations() {
    // Test Account structure creation
    Account test_acc;
    test_acc.account_number = "TEST123456789012";
    test_acc.name = "Test User";
    test_acc.pin_hash = hash_pin("1234");
    test_acc.balance = 1000.0;
    test_acc.status = "ACTIVE";
    
    assert(test_acc.account_number == "TEST123456789012");
    assert(test_acc.name == "Test User");
    assert(test_acc.balance == 1000.0);
    assert(test_acc.status == "ACTIVE");
    
    // Test Transaction structure
    Transaction test_tx;
    test_tx.account_number = "TEST123456789012";
    test_tx.type = "DEPOSIT";
    test_tx.amount = 500.0;
    test_tx.description = "Test deposit";
    
    assert(test_tx.account_number == "TEST123456789012");
    assert(test_tx.type == "DEPOSIT");
    assert(test_tx.amount == 500.0);
    assert(test_tx.description == "Test deposit");
}

void test_transaction_operations() {
    // Test TransactionGuard (basic functionality)
    bool rollback_called = false;
    
    {
        TransactionGuard guard;
        guard.add_rollback_action([&rollback_called]() {
            rollback_called = true;
        });
        // Guard goes out of scope without commit - rollback should be called
    }
    
    assert(rollback_called == true);
    
    // Test with commit
    rollback_called = false;
    {
        TransactionGuard guard;
        guard.add_rollback_action([&rollback_called]() {
            rollback_called = true;
        });
        guard.commit();
        // Rollback should NOT be called after commit
    }
    
    assert(rollback_called == false);
}

void test_edge_cases() {
    cout << "Testing edge cases..." << endl;
    
    // Empty inputs
    assert(validate_phone_number("") == false);
    assert(validate_email("") == false);
    assert(validate_amount("") == false);
    assert(is_strong_pin("") == false);
    
    // Boundary values
    assert(validate_amount("0.01") == true); // Minimum positive amount
    assert(validate_amount("999999.99") == true); // Large amount
    
    // Special characters in emails
    assert(validate_email("test+tag@example.com") == true);
    assert(validate_email("test.user@sub.domain.com") == true);
}

void display_test_results() {
    cout << "\n" << string(50, '=') << endl;
    cout << "📊 TEST RESULTS SUMMARY" << endl;
    cout << string(50, '=') << endl;
    cout << "✅ Tests Passed: " << tests_passed << endl;
    cout << "❌ Tests Failed: " << tests_failed << endl;
    cout << "📈 Total Tests: " << (tests_passed + tests_failed) << endl;
    
    if (tests_failed == 0) {
        cout << "🎉 ALL TESTS PASSED! System is ready." << endl;
    } else {
        cout << "⚠️  Some tests failed. Please check the implementation." << endl;
    }
    cout << string(50, '=') << endl;
}

int main() {
    cout << "🚀 BANKING SYSTEM UNIT TESTS" << endl;
    cout << "=============================" << endl << endl;
    
    // Run all test suites
    run_test("PIN Validation", test_pin_validation);
    run_test("Hashing Functions", test_hashing);
    run_test("Validation Functions", test_validation_functions);
    run_test("Utility Functions", test_utility_functions);
    run_test("Account Operations", test_account_operations);
    run_test("Transaction Operations", test_transaction_operations);
    run_test("Edge Cases", test_edge_cases);
    
    // Display final results
    display_test_results();
    
    return tests_failed > 0 ? 1 : 0;
}