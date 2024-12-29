#include <iostream>
#include <string>
#include <vector>

class Account {
private:
    std::string owner;
    double balance;

public:
    Account(const std::string& ownerName, double initialBalance) : owner(ownerName), balance(initialBalance) {}

    void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
            std::cout << owner << ", deposit of $" << amount << " successful. New balance: $" << balance << std::endl;
        } else {
            std::cout << "Invalid deposit amount!" << std::endl;
        }
    }

    void withdraw(double amount) {
        if (amount > 0 && amount <= balance) {
            balance -= amount;
            std::cout << owner << ", withdrawal of $" << amount << " successful. New balance: $" << balance << std::endl;
        } else {
            std::cout << "Invalid withdrawal amount or insufficient funds!" << std::endl;
        }
    }

    void displayBalance() const {
        std::cout << owner << "'s balance: $" << balance << std::endl;
    }

    std::string getOwner() const {
        return owner;
    }
};

void showMenu() {
    std::cout << "\nBanking System Menu:\n";
    std::cout << "1. Create Account\n";
    std::cout << "2. Deposit\n";
    std::cout << "3. Withdraw\n";
    std::cout << "4. Display Balance\n";
    std::cout << "5. Exit\n";
    std::cout << "Choose an option: ";
}

int main() {
    std::vector<Account> accounts;
    int choice;

    do {
        showMenu();
        std::cin >> choice;

        switch (choice) {
            case 1: {
                std::string name;
                double initialBalance;
                std::cout << "Enter account owner's name: ";
                std::cin >> name;
                std::cout << "Enter initial balance: ";
                std::cin >> initialBalance;
                accounts.emplace_back(name, initialBalance);
                std::cout << "Account created successfully!" << std::endl;
                break;
            }
            case 2: {
                std::string name;
                double amount;
                std::cout << "Enter account owner's name: ";
                std::cin >> name;
                bool found = false;
                for (auto& account : accounts) {
                    if (account.getOwner() == name) {
                        std::cout << "Enter deposit amount: ";
                        std::cin >> amount;
                        account.deposit(amount);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    std::cout << "Account not found!" << std::endl;
                }
                break;
            }
            case 3: {
                std::string name;
                double amount;
                std::cout << "Enter account owner's name: ";
                std::cin >> name;
                bool found = false;
                for (auto& account : accounts) {
                    if (account.getOwner() == name) {
                        std::cout << "Enter withdrawal amount: ";
                        std::cin >> amount;
                        account.withdraw(amount);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    std::cout << "Account not found!" << std::endl;
                }
                break;
            }
            case 4: {
                std::string name;
                std::cout << "Enter account owner's name: ";
                std::cin >> name;
                bool found = false;
                for (const auto& account : accounts) {
                    if (account.getOwner() == name) {
                        account.displayBalance();
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    std::cout << "Account not found!" << std::endl;
                }
                break;
            }
            case 5:
                std::cout << "Exiting the program. Goodbye!" << std::endl;
                break;
            default:
                std::cout << "Invalid option! Please try again." << std::endl;
        }
    } while (choice != 5);

    return 0;
}
