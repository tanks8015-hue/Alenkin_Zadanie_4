#pragma once
#include <windows.h>
#include <sqlext.h>
#include <string>
#include <iostream>
class DatabaseConnector {
private:
    SQLHENV hEnv;
    SQLHDBC hDbc;
    bool isConnected;
    DatabaseConnector();
    ~DatabaseConnector();
public:
    static DatabaseConnector& GetInstance() {
        static DatabaseConnector instance;
        return instance;
    }
    bool Connect(const std::wstring& connectionString);
    void Disconnect();
    bool AddCategorySafe(const std::wstring& categoryName);
    bool CreateOrderTransaction(int userId, int partId, int quantity, int warehouseId);
};