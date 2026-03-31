#include "DatabaseConnector.h"
#include <fstream>
#include <iomanip>

DatabaseConnector::DatabaseConnector() : hEnv(SQL_NULL_HENV), hDbc(SQL_NULL_HDBC), isConnected(false) {
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
}

DatabaseConnector::~DatabaseConnector() {
    Disconnect();
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

bool DatabaseConnector::Connect(const std::wstring& connectionString) {
    SQLWCHAR retConString[1024];
    SQLSMALLINT retConStringLen;

    SQLRETURN ret = SQLDriverConnectW(hDbc, NULL, (SQLWCHAR*)connectionString.c_str(), SQL_NTS,
        retConString, 1024, &retConStringLen, SQL_DRIVER_NOPROMPT);

    if (SQL_SUCCEEDED(ret)) {
        isConnected = true;
        return true;
    }
    return false;
}

void DatabaseConnector::Disconnect() {
    if (isConnected) {
        SQLDisconnect(hDbc);
        isConnected = false;
    }
}

bool DatabaseConnector::AddCategorySafe(const std::wstring& categoryName) {
    if (!isConnected) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"INSERT INTO Categories (CategoryName) VALUES (?)";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLLEN cbCategoryName = SQL_NTS;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR,
        100, 0, (SQLPOINTER)categoryName.c_str(), 0, &cbCategoryName);
    SQLRETURN ret = SQLExecute(hStmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return SQL_SUCCEEDED(ret);
}
bool DatabaseConnector::CreateOrderTransaction(int userId, int partId, int quantity, int warehouseId) {
    if (!isConnected) {
        std::cout << "Нет подключения к БД!\n";
        return false;
    }

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"{CALL sp_CreateOrder(?, ?, ?, ?)}";
    SQLRETURN ret = SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

    if (SQL_SUCCEEDED(ret)) {
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL);
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &partId, 0, NULL);
        SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &quantity, 0, NULL);
        SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &warehouseId, 0, NULL);

        // Выполняем процедуру
        ret = SQLExecute(hStmt);

        if (SQL_SUCCEEDED(ret)) {
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            return true;
        }
        else {
            std::cout << "[ОШИБКА SQL] Транзакция отклонена сервером (возможно, не хватает товара на складе).\n";
        }
    }
    else {
        std::cout << "[ОШИБКА SQL] Не удалось подготовить вызов процедуры.\n";
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return false;
}
int DatabaseConnector::AuthenticateUser(const std::wstring& username, const std::wstring& password) {
    if (!isConnected) return 0;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"SELECT RoleID FROM Users WHERE Username = ? AND PasswordHash = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

    SQLLEN cbUser = SQL_NTS, cbPass = SQL_NTS;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)username.c_str(), 0, &cbUser);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 256, 0, (SQLPOINTER)password.c_str(), 0, &cbPass);

    int roleId = 0;
    if (SQLExecute(hStmt) == SQL_SUCCESS) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLLEN cbRole;
            SQLGetData(hStmt, 1, SQL_C_SLONG, &roleId, 0, &cbRole);
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return roleId; 
}

bool DatabaseConnector::AddPartSafe(const std::wstring& partName, int categoryId, int supplierId, double price) {
    if (!isConnected) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    std::wstring query = L"INSERT INTO Parts (PartName, CategoryID, SupplierID, Price) VALUES (?, ?, ?, ?)";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

    SQLLEN cbName = SQL_NTS;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)partName.c_str(), 0, &cbName);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &categoryId, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &supplierId, 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &price, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    bool success = SQL_SUCCEEDED(ret);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return success;
}
void DatabaseConnector::ShowPartsFromDB() {
    if (!isConnected) {
        std::cout << "Нет подключения к БД!\n";
        return;
    }

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    std::wstring query = L"SELECT p.PartID, p.PartName, c.CategoryName, p.Price FROM Parts p JOIN Categories c ON p.CategoryID = c.CategoryID";

    if (SQLExecDirectW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS) == SQL_SUCCESS) {
        std::cout << "\nID | Название детали          | Категория        | Цена\n";
        std::cout << "---------------------------------------------------------\n";

        SQLINTEGER id;
        SQLCHAR name[100], category[100];
        SQLDOUBLE price;
        SQLLEN cbId, cbName, cbCategory, cbPrice;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cbId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, name, sizeof(name), &cbName);
            SQLGetData(hStmt, 3, SQL_C_CHAR, category, sizeof(category), &cbCategory);
            SQLGetData(hStmt, 4, SQL_C_DOUBLE, &price, 0, &cbPrice);

            std::cout << std::left << std::setw(3) << id << "| "
                << std::setw(25) << name << "| "
                << std::setw(17) << category << "| "
                << price << "\n";
        }
    }
    else {
        std::cout << "[ОШИБКА SQL] Не удалось выполнить выборку данных.\n";
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
bool DatabaseConnector::ExportOrdersToCSV(const std::string& filename) {
    if (!isConnected) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    std::wstring query = L"SELECT o.OrderID, u.Username, p.PartName, od.Quantity, od.UnitPrice "
        L"FROM Orders o "
        L"JOIN Users u ON o.UserID = u.UserID "
        L"JOIN OrderDetails od ON o.OrderID = od.OrderID "
        L"JOIN Parts p ON od.PartID = p.PartID";

    if (SQLExecDirectW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS) == SQL_SUCCESS) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            return false;
        }
        file << "ID Заказа;Сотрудник;Деталь;Количество;Цена за шт.\n";

        SQLINTEGER orderId, quantity;
        SQLCHAR username[50], partName[100];
        SQLDOUBLE unitPrice;
        SQLLEN cbOrderId, cbUsername, cbPartName, cbQuantity, cbUnitPrice;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_SLONG, &orderId, 0, &cbOrderId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, username, sizeof(username), &cbUsername);
            SQLGetData(hStmt, 3, SQL_C_CHAR, partName, sizeof(partName), &cbPartName);
            SQLGetData(hStmt, 4, SQL_C_SLONG, &quantity, 0, &cbQuantity);
            SQLGetData(hStmt, 5, SQL_C_DOUBLE, &unitPrice, 0, &cbUnitPrice);

            file << orderId << ";" << username << ";" << partName << ";"
                << quantity << ";" << unitPrice << "\n";
        }

        file.close();
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return true;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return false;
}
void DatabaseConnector::ShowOrdersFromDB() {
    if (!isConnected) return;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"SELECT o.OrderID, u.Username, o.OrderDate, o.Status "
        L"FROM Orders o JOIN Users u ON o.UserID = u.UserID "
        L"ORDER BY o.OrderDate DESC";

    if (SQLExecDirectW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS) == SQL_SUCCESS) {
        std::cout << "\nID  | Пользователь     | Дата заказа         | Статус\n";
        std::cout << "-----------------------------------------------------------\n";

        SQLINTEGER id;
        SQLCHAR user[50], date[30], status[30];
        SQLLEN cbId, cbUser, cbDate, cbStatus;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cbId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, user, sizeof(user), &cbUser);
            SQLGetData(hStmt, 3, SQL_C_CHAR, date, sizeof(date), &cbDate);
            SQLGetData(hStmt, 4, SQL_C_CHAR, status, sizeof(status), &cbStatus);

            std::cout << std::left << std::setw(4) << id << "| "
                << std::setw(17) << user << "| "
                << std::setw(20) << date << "| "
                << status << "\n";
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
bool DatabaseConnector::CompleteOrder(int orderId) {
    if (!isConnected) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"UPDATE Orders SET Status = 'Completed' WHERE OrderID = ? AND Status = 'Processing'";

    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &orderId, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    SQLLEN rowCount;
    SQLRowCount(hStmt, &rowCount);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return (SQL_SUCCEEDED(ret) && rowCount > 0);
}
void DatabaseConnector::SearchPartsPaginated(double minPrice, double maxPrice, int categoryId, int pageNumber, int rowsPerPage) {
    if (!isConnected) return;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    int offset = (pageNumber - 1) * rowsPerPage;
    std::wstring query = L"SELECT p.PartID, p.PartName, c.CategoryName, p.Price "
        L"FROM Parts p JOIN Categories c ON p.CategoryID = c.CategoryID "
        L"WHERE p.Price >= ? AND p.Price <= ? AND p.CategoryID = ? "
        L"ORDER BY p.PartID "
        L"OFFSET ? ROWS FETCH NEXT ? ROWS ONLY";

    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &minPrice, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &maxPrice, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &categoryId, 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &offset, 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &rowsPerPage, 0, NULL);

    if (SQLExecute(hStmt) == SQL_SUCCESS) {
        std::cout << "\nID | Название детали          | Категория        | Цена\n";
        std::cout << "---------------------------------------------------------\n";

        SQLINTEGER id;
        SQLCHAR name[100], category[100];
        SQLDOUBLE price;
        SQLLEN cbId, cbName, cbCategory, cbPrice;
        bool hasData = false;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            hasData = true;
            SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cbId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, name, sizeof(name), &cbName);
            SQLGetData(hStmt, 3, SQL_C_CHAR, category, sizeof(category), &cbCategory);
            SQLGetData(hStmt, 4, SQL_C_DOUBLE, &price, 0, &cbPrice);

            std::cout << std::left << std::setw(3) << id << "| "
                << std::setw(25) << name << "| "
                << std::setw(17) << category << "| "
                << price << "\n";
        }

        if (!hasData) {
            std::cout << "По вашему запросу ничего не найдено или страница пуста.\n";
        }
    }
    else {
        std::cout << "[ОШИБКА SQL] Не удалось выполнить многокритериальный поиск.\n";
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
bool DatabaseConnector::DeletePartSafe(int partId) {
    if (!isConnected) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    std::wstring query = L"DELETE FROM Parts WHERE PartID = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &partId, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    SQLLEN rowCount = 0;
    SQLRowCount(hStmt, &rowCount);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return (SQL_SUCCEEDED(ret) && rowCount > 0);
}
void DatabaseConnector::ShowTopProfitableParts() {
    if (!isConnected) return;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"SELECT TOP 5 PartID, PartName, TotalRevenue, RANK() OVER (ORDER BY TotalRevenue DESC) as RevenueRank "
        L"FROM (SELECT p.PartID, p.PartName, SUM(od.Quantity * od.UnitPrice) as TotalRevenue "
        L"FROM Parts p JOIN OrderDetails od ON p.PartID = od.PartID "
        L"GROUP BY p.PartID, p.PartName) as SalesData";

    if (SQLExecDirectW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS) == SQL_SUCCESS) {
        std::cout << "\nРейтинг | ID | Название детали          | Общая выручка\n";
        std::cout << "--------------------------------------------------------\n";

        SQLINTEGER id, rank;
        SQLCHAR name[100];
        SQLDOUBLE revenue;
        SQLLEN cbId, cbName, cbRev, cbRank;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cbId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, name, sizeof(name), &cbName);
            SQLGetData(hStmt, 3, SQL_C_DOUBLE, &revenue, 0, &cbRev);
            SQLGetData(hStmt, 4, SQL_C_SLONG, &rank, 0, &cbRank);

            std::cout << std::left << std::setw(8) << rank << "| "
                << std::setw(3) << id << "| "
                << std::setw(25) << name << "| "
                << revenue << "\n";
        }
    }
    else {
        std::cout << "[ОШИБКА SQL] Не удалось собрать аналитику (возможно, еще нет проданных товаров).\n";
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}
bool DatabaseConnector::UpdatePartPrice(int partId, double newPrice) {
    if (!isConnected) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    std::wstring query = L"UPDATE Parts SET Price = ? WHERE PartID = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &newPrice, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &partId, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);

    SQLLEN rowCount = 0;
    if (SQL_SUCCEEDED(ret)) {
        SQLRowCount(hStmt, &rowCount);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return (rowCount > 0);
}
// ================= КАТЕГОРИИ =================

void DatabaseConnector::ShowCategoriesFromDB() {
    if (!isConnected) return;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"SELECT CategoryID, CategoryName FROM Categories";

    if (SQLExecDirectW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS) == SQL_SUCCESS) {
        std::cout << "\nID | Название категории\n";
        std::cout << "-----------------------\n";
        SQLINTEGER id;
        SQLCHAR name[100];
        SQLLEN cbId, cbName;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cbId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, name, sizeof(name), &cbName);
            std::cout << std::left << std::setw(3) << id << "| " << name << "\n";
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

bool DatabaseConnector::DeleteCategory(int categoryId) {
    if (!isConnected) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"DELETE FROM Categories WHERE CategoryID = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &categoryId, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    SQLLEN rowCount = 0;
    if (SQL_SUCCEEDED(ret)) SQLRowCount(hStmt, &rowCount);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return (rowCount > 0);
}

// ================= СКЛАДЫ =================

void DatabaseConnector::ShowWarehousesFromDB() {
    if (!isConnected) return;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"SELECT WarehouseID, Location FROM Warehouses";

    if (SQLExecDirectW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS) == SQL_SUCCESS) {
        std::cout << "\nID | Локация склада\n";
        std::cout << "-----------------------------------\n";
        SQLINTEGER id;
        SQLCHAR loc[200];
        SQLLEN cbId, cbLoc;
        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cbId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, loc, sizeof(loc), &cbLoc);
            std::cout << std::left << std::setw(3) << id << "| " << loc << "\n";
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

bool DatabaseConnector::AddWarehouse(const std::wstring& location) {
    if (!isConnected) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"INSERT INTO Warehouses (Location) VALUES (?)";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLLEN cbLoc = SQL_NTS;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 200, 0, (SQLPOINTER)location.c_str(), 0, &cbLoc);

    SQLRETURN ret = SQLExecute(hStmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return SQL_SUCCEEDED(ret);
}

bool DatabaseConnector::DeleteWarehouse(int warehouseId) {
    if (!isConnected) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"DELETE FROM Warehouses WHERE WarehouseID = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &warehouseId, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    SQLLEN rowCount = 0;
    if (SQL_SUCCEEDED(ret)) SQLRowCount(hStmt, &rowCount);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return (rowCount > 0);
}
bool DatabaseConnector::BulkUpdateCategoryPrice(int categoryId, double percentage) {
    if (!isConnected) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"UPDATE Parts SET Price = Price * (1.0 + ? / 100.0) WHERE CategoryID = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &percentage, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &categoryId, 0, NULL);
    SQLRETURN ret = SQLExecute(hStmt);
    SQLLEN rowCount = 0;
    if (SQL_SUCCEEDED(ret)) {
        SQLRowCount(hStmt, &rowCount);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return (rowCount > 0);
}
// ================= СОТРУДНИКИ =================

void DatabaseConnector::ShowUsersFromDB() {
    if (!isConnected) return;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"SELECT u.UserID, u.Username, r.RoleName "
        L"FROM Users u JOIN Roles r ON u.RoleID = r.RoleID";

    if (SQLExecDirectW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS) == SQL_SUCCESS) {
        std::cout << "\nID | Логин сотрудника     | Должность (Роль)\n";
        std::cout << "----------------------------------------------\n";
        SQLINTEGER id;
        SQLCHAR username[50], roleName[50];
        SQLLEN cbId, cbUser, cbRole;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            SQLGetData(hStmt, 1, SQL_C_SLONG, &id, 0, &cbId);
            SQLGetData(hStmt, 2, SQL_C_CHAR, username, sizeof(username), &cbUser);
            SQLGetData(hStmt, 3, SQL_C_CHAR, roleName, sizeof(roleName), &cbRole);

            std::cout << std::left << std::setw(3) << id << "| "
                << std::setw(21) << username << "| "
                << roleName << "\n";
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

bool DatabaseConnector::AddUser(const std::wstring& username, const std::wstring& password, int roleId) {
    if (!isConnected) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    std::wstring query = L"INSERT INTO Users (Username, PasswordHash, RoleID) VALUES (?, ?, ?)";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

    SQLLEN cbUser = SQL_NTS, cbPass = SQL_NTS;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)username.c_str(), 0, &cbUser);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 256, 0, (SQLPOINTER)password.c_str(), 0, &cbPass);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &roleId, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return SQL_SUCCEEDED(ret);
}

bool DatabaseConnector::DeleteUser(int userId) {
    if (!isConnected) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    std::wstring query = L"DELETE FROM Users WHERE UserID = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    SQLLEN rowCount = 0;
    if (SQL_SUCCEEDED(ret)) SQLRowCount(hStmt, &rowCount);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return (rowCount > 0);
}
bool DatabaseConnector::DeleteOrder(int orderId) {
    if (!isConnected) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    std::wstring query = L"DELETE FROM Orders WHERE OrderID = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &orderId, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    SQLLEN rowCount = 0;
    if (SQL_SUCCEEDED(ret)) {
        SQLRowCount(hStmt, &rowCount);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return (rowCount > 0);
}
// Доп. функция 1: Автоматический контроль критических остатков
void DatabaseConnector::CheckLowStockAlerts() {
    if (!isConnected) return;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    // Ищем детали, которых на любом из складов осталось 50 или меньше
    std::wstring query = L"SELECT p.PartName, w.Location, i.Quantity "
        L"FROM Inventory i "
        L"JOIN Parts p ON i.PartID = p.PartID "
        L"JOIN Warehouses w ON i.WarehouseID = w.WarehouseID "
        L"WHERE i.Quantity <= 50";

    if (SQLExecDirectW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS) == SQL_SUCCESS) {
        bool hasAlerts = false;
        SQLCHAR name[100], location[200];
        SQLINTEGER qty;
        SQLLEN cbName, cbLoc, cbQty;

        while (SQLFetch(hStmt) == SQL_SUCCESS) {
            if (!hasAlerts) {
                // Если нашли хотя бы одну деталь, выводим заголовок ахтунга
                std::cout << "\n[ВНИМАНИЕ] СИСТЕМА ОПОВЕЩЕНИЙ: КРИТИЧЕСКИЕ ОСТАТКИ НА СКЛАДАХ!\n";
                std::cout << "---------------------------------------------------------\n";
                hasAlerts = true;
            }
            SQLGetData(hStmt, 1, SQL_C_CHAR, name, sizeof(name), &cbName);
            SQLGetData(hStmt, 2, SQL_C_CHAR, location, sizeof(location), &cbLoc);
            SQLGetData(hStmt, 3, SQL_C_SLONG, &qty, 0, &cbQty);

            std::cout << " -> Деталь '" << name << "' на складе '" << location
                << "' заканчивается! Остаток: " << qty << " шт.\n";
        }

        if (hasAlerts) {
            std::cout << "Рекомендуется срочно связаться с поставщиками!\n";
            std::cout << "---------------------------------------------------------\n";
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}