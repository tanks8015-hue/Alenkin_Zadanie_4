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