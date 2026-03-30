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
    void ShowPartsFromDB();
    bool ExportOrdersToCSV(const std::string& filename);
    bool AuthenticateUser(const std::wstring& username, const std::wstring& password);
    bool AddPartSafe(const std::wstring& partName, int categoryId, int supplierId, double price);
    void ShowOrdersFromDB();
    bool CompleteOrder(int orderId);
    void SearchPartsPaginated(double minPrice, double maxPrice, int categoryId, int pageNumber, int rowsPerPage);
};