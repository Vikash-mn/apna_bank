import random
import os
import datetime
from pymongo import MongoClient

client = MongoClient('localhost', 27017)
db = client['Apna_Bank']
collection1 = db['Bank_account']
history_collection = db['history']

userAccounts = {}
account_numbers = []
account_pins = []

# To clear the screen
def clear_screen():
    if os.name == 'nt':
        os.system('cls')
    else:
        os.system('clear')

# Function to generate a unique account number
def generate_account_number():
    user_account_number = (
        "A" +
        "P" +
        "N" +
        "A" +
        str(random.randint(1, 9)) +
        str(random.randint(0, 9)) +
        str(random.randint(0, 9)) +
        str(random.randint(0, 9)) +
        str(random.randint(1, 9)) +
        str(random.randint(0, 9)) +
        str(random.randint(0, 9)) +
        str(random.randint(0, 9)) +
        str(random.randint(1, 9)) +
        str(random.randint(0, 9)) +
        str(random.randint(0, 9)) +
        str(random.randint(0, 9))
    )
    if user_account_number in account_numbers:
        return generate_account_number()
    else:
        account_numbers.append(user_account_number)
    return user_account_number

# Function to generate a unique account pin
def generate_account_pin():
    pin = str(random.randint(1000, 9999))
    if pin in account_pins:
        return generate_account_pin()
    else:
        account_pins.append(pin)
    return pin

# Function to create a new bank account
def create(name, phone_number, gender, age):
    existing_account = collection1.find_one({"name": name, "phone_number": phone_number})
    if existing_account:
        print(f"An account with the name '{name}' and phone number '{phone_number}' already exists.")
    else:
        new_account_number = generate_account_number()
        new_account_pin = generate_account_pin()
        print(f"Account successfully created for: {name}\nPhone number: {phone_number}\nGender: {gender}\nAge: {age}\nAccount number: {new_account_number}\nPIN: {new_account_pin}.")

        user_data = {
            "name": name,
            "phone_number": phone_number,
            "gender": gender,
            "age": age,
            "account_number": new_account_number,
            "pin": new_account_pin,
            "balance": 0
        }
        collection1.insert_one(user_data)

# Main program loop
print("Welcome to Apna Bank")
print("---------------------------------------------")
while True:
    print("\nPlease choose an option:")
    print("1) Create a new account")
    print("2) Deposit")
    print("3) Withdraw")
    print("4) Check balance")
    print("5) View account details")
    print("6) View transaction history")
    print("7) Exit")

    try:
        option = int(input("Enter your choice: "))

        if option == 1:
            user_name = input("Enter your name: ")
            gender = input("Enter your gender (M, F, O): ").upper()
            if gender not in ['M', 'F', 'O']:
                print("Invalid gender input. Please enter 'M' for Male, 'F' for Female, or 'O' for Other.")
                continue

            while True:
                phone_number = input("Enter your phone number (without +91 and 0): ")
                try:
                    age = int(input("Enter your age: "))
                    if age >= 18:
                        if len(phone_number) == 10 and phone_number.isdigit():
                            create(user_name, phone_number, gender, age)
                            break
                        else:
                            print("Invalid phone number. Please enter a 10-digit number.")
                    else:
                        print("You are under the age limit to create an account.")
                except ValueError:
                    print("Invalid age. Please enter a valid number.")

        elif option == 2:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")

            existing_account = collection1.find_one({"account_number": account_number, "pin": pin})
            if existing_account:
                print("---------- Welcome to the Deposit Sector ----------")
                print("Rules:\n 1) Minimum amount is 500 \n 2) Maximum amount is 100,000")

                while True:
                    try:
                        amount = int(input("Enter the amount to deposit: "))
                        if amount < 500:
                            print("The amount must be at least 500. Please try again.")
                        elif amount > 100000:
                            print("The amount cannot exceed 100,000. Please try again.")
                        else:
                            transaction_type = "credit"
                            new_balance = existing_account["balance"] + amount
                            collection1.update_one({"account_number": account_number}, {"$set": {"balance": new_balance}})
                            print(f"You have successfully deposited {amount}. Your new balance is: {new_balance}")

                            # Record the transaction in the history
                            history_collection.insert_one({
                                "Account_no": account_number,
                                "Amount": amount,
                                "Type": transaction_type,
                                "time": datetime.datetime.now()
                            })
                            break
                    except ValueError:
                        print("Invalid input. Please enter a numeric value.")

            else:
                print("Invalid account number or PIN. Please try again.")

        elif option == 3:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")

            existing_account = collection1.find_one({"account_number": account_number, "pin": pin})
            if existing_account:
                print("---------- Welcome to the Withdrawal Sector ----------")

                while True:
                    try:
                        withdraw_amount = int(input("Enter the amount to withdraw: "))
                        if withdraw_amount > existing_account["balance"]:
                            print("Insufficient balance. Please enter a lower amount.")
                        elif withdraw_amount < 500:
                            print("The minimum withdrawal amount is 500. Please try again.")
                        else:
                            transaction_type = "debit"
                            new_balance = existing_account["balance"] - withdraw_amount
                            collection1.update_one({"account_number": account_number}, {"$set": {"balance": new_balance}})
                            print(f"Withdrawal successful! You have withdrawn {withdraw_amount}. Your new balance is: {new_balance}")

                            # Record the transaction in the history
                            history_collection.insert_one({
                                "Account_no": account_number,
                                "Amount": withdraw_amount,
                                "Type": transaction_type,
                                "time": datetime.datetime.now()
                            })
                            break
                    except ValueError:
                        print("Invalid input. Please enter a numeric value.")

            else:
                print("Invalid account number or PIN. Please try again.")

        elif option == 4:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")

            existing_account = collection1.find_one({"account_number": account_number, "pin": pin})
            if existing_account:
                print(f"Your account balance is: {existing_account['balance']}")
            else:
                print("Invalid account number or PIN. Please try again.")

        elif option == 5:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")

            existing_account = collection1.find_one({"account_number": account_number, "pin": pin})
            if existing_account:
                print("\n---------- Account Details ----------")
                print(f"Name: {existing_account['name']}")
                print(f"Phone Number: {existing_account['phone_number']}")
                print(f"Gender: {existing_account['gender']}")
                print(f"Age: {existing_account['age']}")
                print(f"Account Number: {existing_account['account_number']}")
                print(f"Balance: {existing_account['balance']}")
                print("-------------------------------------")
            else:
                print("Invalid account number or PIN. Please try again.")

        elif option == 6:
            clear_screen()
            account_number = input("Enter your account number: ")
            pin = input("Enter your PIN: ")

            existing_account = collection1.find_one({"account_number": account_number, "pin": pin})
            if existing_account:
                print("\n---------- Transaction History ----------")
                transaction_history = list(history_collection.find({"Account_no": account_number}))  # Corrected line

                if transaction_history:
                    for transaction in transaction_history:
                        print(f"Date & Time: {transaction['time']}")
                        print(f"Type: {transaction['Type']}")
                        print(f"Amount: {transaction['Amount']}")
                        print("-------------------------------------")

                    # Show total remaining balance
                    remaining_balance = existing_account['balance']
                    print(f"Total Remaining Balance in Account: {remaining_balance}")
                else:
                    print("No transaction history found for this account.")
            else:
                print("Invalid account number or PIN. Please try again.")

        elif option == 7:
            print("Thank you for using Apna Bank. Goodbye!")
            break

        else:
            print("Invalid option. Please choose a valid option (1-7).")
    except ValueError:
        print("Invalid input. Please enter a number.")
