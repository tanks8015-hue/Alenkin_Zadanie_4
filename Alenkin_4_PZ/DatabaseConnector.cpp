#include "DatabaseConnector.h"
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
        // Привязываем все 4 параметра (все они типа INT)
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
            // Если SQLExecute вернул ошибку (например, сработал THROW из-за нехватки товара)
            std::cout << "[ОШИБКА SQL] Транзакция отклонена сервером (возможно, не хватает товара на складе).\n";
        }
    }
    else {
        std::cout << "[ОШИБКА SQL] Не удалось подготовить вызов процедуры.\n";
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return false;
}
// Реальная проверка логина и пароля в БД
bool DatabaseConnector::AuthenticateUser(const std::wstring& username, const std::wstring& password) {
    if (!isConnected) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

    std::wstring query = L"SELECT UserID FROM Users WHERE Username = ? AND PasswordHash = ?";
    SQLPrepareW(hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);

    SQLLEN cbUser = SQL_NTS, cbPass = SQL_NTS;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, (SQLPOINTER)username.c_str(), 0, &cbUser);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 256, 0, (SQLPOINTER)password.c_str(), 0, &cbPass);

    bool isAuthenticated = false;
    if (SQLExecute(hStmt) == SQL_SUCCESS) {
        if (SQLFetch(hStmt) == SQL_SUCCESS) {
            isAuthenticated = true; // Пользователь найден
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return isAuthenticated;
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