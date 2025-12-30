import random
import string
import os
import datetime
import time
from pymongo import MongoClient
from prettytable import PrettyTable
import logging
from twilio.rest import Client
from twilio.base.exceptions import TwilioException


client = MongoClient('localhost', 27017)
db = client['Apna_Bank']
collection1 = db['Bank_account']
history_collection = db['history']

account_numbers = []
account_pins = []

# Set up logging for better debugging and traceability

def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')

# Helper Functions
def generate_account_number():
    user_account_number = "APNA" + ''.join(random.choices(string.digits, k=12))
    if user_account_number in account_numbers:
        return generate_account_number()
    account_numbers.append(user_account_number)
    return user_account_number

def generate_account_pin():
    pin = str(random.randint(1000, 9999))
    if pin in account_pins:
        return generate_account_pin()
    account_pins.append(pin)
    return pin

def validate_user(account_number, pin):
    return collection1.find_one({"account_number": account_number, "pin": pin})


# Function to show transaction history for a user account based on AccountNo
def show_transaction_history(account_number, pin):
    # Verify account using account_number and pin
    account = validate_user(account_number, pin)
    if not account:
        print("Invalid account number or PIN.")
        return

    # Fetch transactions involving the given account_number, using "AccountNo"
    transaction_history = list(history_collection.find({
        "AccountNo": account_number  # Search for transactions using "AccountNo"
    }).sort("time", -1))  # Sort by time, descending

    if transaction_history:
        # Initialize the passbook table
        passbook = PrettyTable(["AccountNo", "Type", "From (Name)", "To (Name)", "Amount", "Date & Time"])

        for transaction in transaction_history:
            # Get the account names for "From" and "To"
            from_account = transaction.get("From", "N/A")
            to_account = transaction.get("To", "N/A")
            from_name = "N/A"
            to_name = "N/A"

            try:
                # Fetch the names of the accounts involved in the transaction
                if from_account != "N/A":
                    from_account_data = collection1.find_one({"account_number": from_account})
                    from_name = from_account_data["name"] if from_account_data else "Unknown"
                if to_account != "N/A":
                    to_account_data = collection1.find_one({"account_number": to_account})
                    to_name = to_account_data["name"] if to_account_data else "Unknown"
            except Exception as e:
                print(f"Error fetching account names: {e}")

            # Add the transaction to the passbook
            passbook.add_row([
                transaction["AccountNo"],
                transaction.get("Type", "N/A"),
                f"{from_account} ({from_name})",
                f"{to_account} ({to_name})",
                transaction.get("Amount", "N/A"),
                transaction.get("time", "Unknown").strftime("%Y-%m-%d %H:%M:%S")  # Format the timestamp
            ])

        # Display the passbook
        print(f"\n---------- Passbook for Account: {account_number} ----------")
        print(passbook)
    else:
        print("No transaction history found for this account.")
# Main Application
print("Welcome to Apna Bank")
print("---------------------------------------------")
while True:
    print("\nPlease choose an option:")
    print("1) Create a new account")
    print("2) Deposit")
    print("3) Withdraw")
    print("4) Send money")
    print("5) Check balance")
    print("6) View account details")
    print("7) View transaction history")
    print("8) Exit")

    try:
        option = int(input("Enter your choice: "))

        if option == 1:
            clear_screen()
            user_name = input("Enter your name: ").strip().upper()
            while True:
                gender = input("Enter your gender (M for Male, F for Female, O for Other): ").strip().upper()
                if gender in ['M', 'F', 'O']:
                    break
                print("Invalid input. Please enter 'M', 'F', or 'O'.")
            while True:
                phone_number = input("Enter your 10-digit phone number: ").strip()
                try:
                    age = int(input("Enter your age: "))
                    if len(phone_number) == 10 and phone_number.isdigit() and age >= 18:
                        new_account_number = generate_account_number()
                        new_account_pin = generate_account_pin()
                        collection1.insert_one({
                            "name": user_name,
                            "phone_number": phone_number,
                            "gender": gender,
                            "age": age,
                            "account_number": new_account_number,
                            "pin": new_account_pin,
                            "balance": 0
                        })
                        print(f"Account created successfully!\nAccount Number: {new_account_number}\nPIN: {new_account_pin}")
                        break
                    else:
                        print("Invalid details or age below 18. Try again.")
                except ValueError:
                    print("Invalid input. Please enter valid details.")

        elif option == 2:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")
            existing_account = validate_user(account_number, pin)
            if existing_account:
                while True:
                    try:
                        amount = int(input("Enter the amount to deposit (min 500, max 100,000): "))
                        if 500 <= amount <= 100000:
                            new_balance = existing_account["balance"] + amount
                            collection1.update_one({"account_number": account_number}, {"$set": {"balance": new_balance}})
                            print(f"Deposit successful! New Balance: {new_balance}")
                            history_collection.insert_one({
                                "AccountNo": account_number,
                                "Type": "credit-self",
                                "Amount": amount,
                                "time": datetime.datetime.now()
                            })
                            break
                        print("Invalid amount. Please try again.")
                    except ValueError:
                        print("Please enter a valid numeric amount.")
            else:
                print("Invalid account number or PIN.")

        elif option == 3:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")
            existing_account = validate_user(account_number, pin)
            if existing_account:
                while True:
                    try:
                        withdraw_amount = int(input("Enter withdrawal amount (min 500): "))
                        if withdraw_amount >= 500 and withdraw_amount <= existing_account["balance"]:
                            new_balance = existing_account["balance"] - withdraw_amount
                            collection1.update_one({"account_number": account_number}, {"$set": {"balance": new_balance}})
                            print(f"Withdrawal successful! New Balance: {new_balance}")
                            history_collection.insert_one({
                                "AccountNo": account_number,
                                "Type": "debit-self",
                                "Amount": withdraw_amount,
                                "time": datetime.datetime.now()
                            })
                            break
                        print("Invalid amount or insufficient balance.")
                    except ValueError:
                        print("Please enter a valid numeric amount.")
            else:
                print("Invalid account number or PIN.")

        elif option == 4:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")
            existing_account = validate_user(account_number, pin)
            if existing_account:
                recipient_account = input("Enter the recipient's account number: ")
                recipient = collection1.find_one({"account_number": recipient_account})
                if recipient:
                    while True:
                        try:
                            SendingAmount = int(input("Enter transfer amount: "))
                            if SendingAmount > 0 and SendingAmount <= existing_account["balance"]:
                                collection1.update_one({"account_number": account_number}, {"$inc": {"balance": -SendingAmount}})
                                collection1.update_one({"account_number": recipient_account}, {"$inc": {"balance": SendingAmount}})
                                print(f"Successfully transferred ₹{SendingAmount} to {recipient['name']}.")
                                history_collection.insert_many([
                                    {"AccountNo":account_number,"Type": "debit", "From": account_number, "To": recipient_account, "Amount": SendingAmount, "time": datetime.datetime.now()},
                                    {"RecipientAccountNO":recipient_account,"Type": "credit", "From": account_number, "To": recipient_account, "Amount": SendingAmount, "time": datetime.datetime.now()}
                                ])
                                break
                            print("Invalid amount or insufficient balance.")
                        except ValueError:
                            print("Invalid input. Please enter a numeric amount.")
                else:
                    print("Recipient account not found.")
            else:
                print("Invalid account number or PIN.")

        elif option == 5:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")
            existing_account = validate_user(account_number, pin)
            if existing_account:
                print(f"Your account balance is: {existing_account['balance']}")
            else:
                print("Invalid account number or PIN.")

        elif option == 6:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")
            existing_account = validate_user(account_number, pin)
            if existing_account:
                print("\n---------- Account Details ----------")
                print(f"Name: {existing_account['name']}")
                print(f"Phone: {existing_account['phone_number']}")
                print(f"Gender: {existing_account['gender']}")
                print(f"Age: {existing_account['age']}")
                print(f"Account: {existing_account['account_number']}")
                print(f"Balance: {existing_account['balance']}")
            else:
                print("Invalid account number or PIN.")

        elif option == 7:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")
            show_transaction_history(account_number,pin)
        elif option == 8:
            print("Thank you for banking with us. Goodbye!")
            break

        else:
            print("Invalid choice. Please select an option between 1 and 8.")
    except ValueError:
        print("Invalid input. Please enter a number.")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
