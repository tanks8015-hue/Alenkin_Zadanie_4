#include <iostream>
#include "DatabaseConnector.h"
#include "Validator.h"

int main() {
    setlocale(LC_ALL, "Russian");
    std::wstring connStr = L"DRIVER={ODBC Driver 17 for SQL Server};SERVER=DESKTOP-PKN6175\\SQLEXPRESS;DATABASE=Alenkin_Zadanie_4;Trusted_Connection=yes;";

    std::cout << "Подключение к базе данных...\n";
    if (DatabaseConnector::GetInstance().Connect(connStr)) {
        std::cout << "Успешное подключение!\n";
        std::string testEmail = "manager@factory.com";
        if (Validator::IsValidEmail(testEmail)) {
            std::cout << "Email '" << testEmail << "' валиден.\n";
        }
        else {
            std::cout << "Ошибка формата Email!\n";
        }
        std::wstring newCat = L"Стекло";
        if (DatabaseConnector::GetInstance().AddCategorySafe(newCat)) {
            std::cout << "Категория успешно добавлена в БД без риска инъекции!\n";
        }
        else {
            std::cout << "Ошибка при добавлении категории.\n";
        }

        DatabaseConnector::GetInstance().Disconnect();
    }
    else {
        std::cout << "Ошибка подключения к БД.\n";
    }

    return 0;
}