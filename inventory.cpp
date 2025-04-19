#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>
#include <sqlite3.h>
using namespace std;

class Product {
public:
    int id;
    string name;
    double price;
    int quantity;

    Product() {}

    Product(int id, const string& name, double price, int quantity)
        : id(id), name(name), price(price), quantity(quantity) {}

    void display() const {
        cout << left << setw(10) << id << setw(20) << name
             << setw(10) << fixed << setprecision(2) << price << setw(10) << quantity << endl;
    }
};

class Inventory {
private:
    sqlite3* db;

public:
    Inventory() {
        if (sqlite3_open("inventory.db", &db)) {
            cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        } else {
            char* errMsg = 0;
            const char* createTableSQL = R"(
                CREATE TABLE IF NOT EXISTS products (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    name TEXT NOT NULL,
                    price REAL NOT NULL,
                    quantity INTEGER NOT NULL
                );
            )";
            if (sqlite3_exec(db, createTableSQL, 0, 0, &errMsg) != SQLITE_OK) {
                cerr << "SQL error: " << errMsg << endl;
                sqlite3_free(errMsg);
            }
        }
    }

    ~Inventory() {
        sqlite3_close(db);
    }

    void addProduct(const string& name, double price, int quantity) {
        string sql = "INSERT INTO products (name, price, quantity) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_double(stmt, 2, price);
            sqlite3_bind_int(stmt, 3, quantity);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                cout << "Product added successfully.\n";
            } else {
                cout << "Failed to add product.\n";
            }
            sqlite3_finalize(stmt);
        }
    }

    void updateStock(int id, int quantity) {
        string sql = "UPDATE products SET quantity = quantity + ? WHERE id = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, quantity);
            sqlite3_bind_int(stmt, 2, id);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                int rowsAffected = sqlite3_changes(db);
                if (rowsAffected > 0) {
                    cout << "Stock updated for ID: " << id << endl;
                } else {
                    cout << "Invalid product ID: " << id << endl;
                }
            } else {
                cout << "Failed to update stock.\n";
            }
            sqlite3_finalize(stmt);
        }
    }

    void removeProduct(int id) {
        string sql = "DELETE FROM products WHERE id = ?;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, id);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                int rowsAffected = sqlite3_changes(db);
                if (rowsAffected > 0) {
                    cout << "Product removed.\n";
                } else {
                    cout << "Invalid product ID: " << id << endl;
                }
            } else {
                cout << "Failed to remove product.\n";
            }
            sqlite3_finalize(stmt);
        }
    }

    void showAllProducts() const {
        const char* sql = "SELECT id, name, price, quantity FROM products;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            cout << left << setw(10) << "ID" << setw(20) << "Name"
                 << setw(10) << "Price" << setw(10) << "Quantity" << endl;
            cout << string(50, '-') << endl;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                double price = sqlite3_column_double(stmt, 2);
                int quantity = sqlite3_column_int(stmt, 3);

                Product p(id, name, price, quantity);
                p.display();
            }
            sqlite3_finalize(stmt);
        } else {
            cout << "Failed to fetch products.\n";
        }
    }

    void searchProduct(const string& name) const {
        string sql = "SELECT id, name, price, quantity FROM products WHERE name LIKE ?;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            string pattern = "%" + name + "%";
            sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);

            bool found = false;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                string pname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                double price = sqlite3_column_double(stmt, 2);
                int quantity = sqlite3_column_int(stmt, 3);

                Product p(id, pname, price, quantity);
                p.display();
                found = true;
            }
            if (!found) {
                cout << "No product found with the name: " << name << endl;
            }
            sqlite3_finalize(stmt);
        } else {
            cout << "Failed to search product.\n";
        }
    }
};

void showMenu() {
    cout << "\n--- Inventory & Stock Management ---\n";
    cout << "1. Add Product\n";
    cout << "2. Update Stock\n";
    cout << "3. Remove Product\n";
    cout << "4. Show All Products\n";
    cout << "5. Search Product\n";
    cout << "6. Exit\n";
    cout << "Choose an option: ";
}

int main() {
    Inventory inv;
    int choice;

    while (true) {
        showMenu();
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string name;
            double price;
            int quantity;
            cout << "Enter product name: ";
            getline(cin, name);
            cout << "Enter price: ";
            cin >> price;
            cout << "Enter quantity: ";
            cin >> quantity;
            inv.addProduct(name, price, quantity);
        } else if (choice == 2) {
            int id, quantity;
            cout << "Enter product ID: ";
            cin >> id;
            cout << "Enter quantity to add (negative to subtract): ";
            cin >> quantity;
            inv.updateStock(id, quantity);
        } else if (choice == 3) {
            int id;
            cout << "Enter product ID to remove: ";
            cin >> id;
            inv.removeProduct(id);
        } else if (choice == 4) {
            inv.showAllProducts();
        } else if (choice == 5) {
            string name;
            cout << "Enter product name to search: ";
            cin.ignore();
            getline(cin, name);
            inv.searchProduct(name);
        } else if (choice == 6) {
            cout << "Exiting..." << endl;
            break;
        } else {
            cout << "Invalid choice. Try again." << endl;
        }
    }

    return 0;
}