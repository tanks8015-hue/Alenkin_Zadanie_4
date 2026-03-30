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
    // Реальная выборка данных из БД (замена фейка в case 3)
    void ShowPartsFromDB();
    // Реальный экспорт в CSV прямо из таблиц (замена фейка в case 5)
    bool ExportOrdersToCSV(const std::string& filename);
    bool AuthenticateUser(const std::wstring& username, const std::wstring& password);
    bool AddPartSafe(const std::wstring& partName, int categoryId, int supplierId, double price);
};