# Apna Bank

Apna Bank is a Python-based banking management system integrated with MongoDB, designed to handle basic banking operations like creating accounts, depositing, withdrawing, and checking balances securely.

## Features

- **Create a New Account**: Users can create a new bank account by providing personal information. Each account is assigned a unique account number and a secure PIN.
- **Deposit Funds**: Allows users to deposit amounts between 500 and 100,000 into their accounts.
- **Withdraw Funds**: Users can withdraw funds with sufficient balance, with a minimum withdrawal amount of 500.
- **Check Account Balance**: Displays the current balance in the user's account.
- **Data Persistence**: All user data is securely stored in MongoDB for fast and reliable access.

## Technologies Used

- **Programming Language**: Python
- **Database**: MongoDB
- **Python Libraries**: `pymongo` for database operations

## Prerequisites

Make sure you have the following software installed:

- Python 3.x
- MongoDB Server
- Required Python packages:
  ```bash
  pip install pymongo
