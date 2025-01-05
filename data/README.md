# Apna Bank Application

## Overview

Apna Bank is a simple banking application built using Python. It utilizes MongoDB for data storage, PrettyTable for displaying data in tabular format, and Twilio for SMS notifications. The application supports basic banking operations such as account creation, deposits, withdrawals, money transfers, balance inquiries, and transaction history viewing.

## Features

- Create new bank accounts
- Deposit funds
- Withdraw funds
- Transfer money between accounts
- Check account balance
- View account details
- Display transaction history
- Clear screen functionality for better UI

## Prerequisites

Ensure the following Python packages are installed:

- `pymongo`
- `prettytable`
- `twilio`
- `datetime`

MongoDB should be installed and running on `localhost:27017`.

## Setup

1. Install MongoDB and start the server.
2. Clone the repository or copy the provided script into a file, e.g., `apna_bank.py`.
3. Install the required Python packages:
   ```bash
   pip install pymongo prettytable twilio
   ```
4. Run the script:
   ```bash
   python apna_bank.py
   ```

## Usage

1. Upon starting the application, a menu with several options will be displayed.
2. Select an option by entering the corresponding number.
3. Follow the prompts to perform various banking operations.

### Main Menu Options

1. **Create a New Account**

   - Enter user details including name, gender, phone number, and age.
   - A unique account number and PIN will be generated.

2. **Deposit**

   - Enter account number and PIN.
   - Specify the amount to deposit.

3. **Withdraw**

   - Enter account number and PIN.
   - Specify the amount to withdraw.

4. **Send Money**

   - Enter sender and recipient account numbers.
   - Specify the transfer amount.

5. **Check Balance**

   - Enter account number and PIN.
   - View current balance.

6. **View Account Details**

   - Enter account number and PIN.
   - View account holder details.

7. **View Transaction History**

   - Enter account number and PIN.
   - Display recent transactions.

8. **Exit**

   - Exit the application.

## Logging

The application uses Python's built-in `logging` module to track errors and operations for debugging purposes. Logs will be printed to the console.

## SMS Notifications

The application uses Twilio for sending SMS notifications. Ensure Twilio credentials (`Account SID`, `Auth Token`) are properly set up in the environment if this feature is to be used.

## License

This project is licensed under the MIT License.

---

**Note:**

- Ensure MongoDB is running before starting the application.
- Twilio-related features require a valid Twilio account and credentials.

