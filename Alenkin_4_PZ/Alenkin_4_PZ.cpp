#include <iostream>
#include <string>
#include <limits>
#include "DatabaseConnector.h"
#include "Validator.h"
#include <windows.h>
void ClearInput() {
    std::cin.clear();
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
}
std::wstring ConvertToWideChar(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(1251, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(1251, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
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
    std::cout << "7. Просмотр и завершение заказов (Update/Status)\n";
    std::cout << "8. Удалить деталь (Только для Администратора)\n";
    std::cout << "9. Аналитика: Топ-5 прибыльных деталей (Оконные функции)\n";
    std::cout << "10. Изменить цену детали (Проверка AuditLog)\n";
    std::cout << "11. Управление категориями (CRUD)\n";
    std::cout << "12. Управление складами (CRUD)\n";
    std::cout << "13. Массовая переоценка категории (Скидки/Инфляция)\n";
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

    // Негативные 
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

    int currentUserRole = 0;
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

            std::wstring wLogin = ConvertToWideChar(login);
            std::wstring wPassword = ConvertToWideChar(password);

            currentUserRole = DatabaseConnector::GetInstance().AuthenticateUser(wLogin, wPassword);

            if (currentUserRole > 0) {
                std::cout << "[УСПЕХ] Вы успешно авторизовались! Ваша роль ID: " << currentUserRole << "\n";
            }
            else {
                std::cout << "[ОШИБКА] Неверный логин или пароль!\n";
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
                std::wstring wName = ConvertToWideChar(name);
                double parsedPrice = std::stod(price);
                if (DatabaseConnector::GetInstance().AddPartSafe(wName, 1, 1, parsedPrice)) {
                    std::cout << "[УСПЕХ] Деталь '" << name << "' успешно добавлена в базу данных по цене " << price << ".\n";
                }
                else {
                    std::cout << "[ОШИБКА] Не удалось добавить деталь в БД.\n";
                }
            }
            break;
        }
        case 3: {
            std::cout << "\n--- МНОГОКРИТЕРИАЛЬНЫЙ ПОИСК И ПАГИНАЦИЯ ---\n";
            double minP, maxP;
            int catId, page;

            std::cout << "1. Введите минимальную цену (например, 0): ";
            std::cin >> minP;
            std::cout << "2. Введите максимальную цену (например, 10000): ";
            std::cin >> maxP;
            std::cout << "3. Введите ID категории для поиска (например, 1): ";
            std::cin >> catId;
            std::cout << "4. Введите номер страницы (начиная с 1): ";
            std::cin >> page;

            if (page < 1) page = 1;

            std::cout << "\nРезультаты (Страница " << page << ", лимит 5 записей на страницу):\n";
            DatabaseConnector::GetInstance().SearchPartsPaginated(minP, maxP, catId, page, 5);
            break;
        };

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
            if (DatabaseConnector::GetInstance().ExportOrdersToCSV("report_orders.csv")) {
                std::cout << "[УСПЕХ] Отчет сформирован из БД и сохранен в файл 'report_orders.csv'.\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось создать отчет.\n";
            }
            break;
        case 6:
            RunValidatorTests();
            break;
        case 7: {
            std::cout << "\n--- СПИСОК ЗАКАЗОВ ---\n";
            DatabaseConnector::GetInstance().ShowOrdersFromDB();

            std::cout << "\nХотите завершить заказ? (Введите ID или 0 для отмены): ";
            int orderId;
            if (!(std::cin >> orderId) || orderId == 0) {
                ClearInput();
                break;
            }

            if (DatabaseConnector::GetInstance().CompleteOrder(orderId)) {
                std::cout << "[УСПЕХ] Заказ #" << orderId << " успешно переведен в статус 'Completed'.\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось обновить заказ. Возможно, он уже завершен или ID неверен.\n";
            }
            break;
        }
        case 8: {
            std::cout << "\n--- УДАЛЕНИЕ ДЕТАЛИ ---\n";
            if (currentUserRole != 1) { 
                std::cout << "[ОТКАЗ В ДОСТУПЕ] Эту операцию может выполнять только Администратор!\n";
                break;
            }

            int pId;
            std::cout << "Введите ID детали для удаления: ";
            std::cin >> pId;

            if (DatabaseConnector::GetInstance().DeletePartSafe(pId)) {
                std::cout << "[УСПЕХ] Деталь #" << pId << " успешно удалена из БД.\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось удалить деталь (Возможно, на неё есть ссылки в заказах!).\n";
            }
            break;
        }
        case 9:
            std::cout << "\n--- АНАЛИТИКА (ОКОННЫЕ ФУНКЦИИ) ---\n";
            DatabaseConnector::GetInstance().ShowTopProfitableParts();
            break;
        case 10: {
            std::cout << "\n--- ИЗМЕНЕНИЕ ЦЕНЫ (АУДИТ) ---\n";
            int pId;
            std::string priceStr;

            std::cout << "Введите ID детали: ";
            std::cin >> pId;
            std::cout << "Введите новую цену (формат 0.00): ";
            std::cin >> priceStr;

            if (!Validator::IsValidPrice(priceStr)) {
                std::cout << "[ОШИБКА] Неверный формат цены!\n";
                break;
            }

            double newPrice = std::stod(priceStr);

            if (DatabaseConnector::GetInstance().UpdatePartPrice(pId, newPrice)) {
                std::cout << "[УСПЕХ] Цена обновлена! SQL-триггер автоматически записал это в AuditLog.\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось обновить цену (неверный ID).\n";
            }
            break;
        }
        case 11: {
            std::cout << "\n--- УПРАВЛЕНИЕ КАТЕГОРИЯМИ ---\n";
            std::cout << "1. Показать все категории\n2. Добавить категорию\n3. Удалить категорию (Только Админ)\nВыберите: ";
            int subChoice;
            if (!(std::cin >> subChoice)) { ClearInput(); break; }

            if (subChoice == 1) {
                DatabaseConnector::GetInstance().ShowCategoriesFromDB();
            }
            else if (subChoice == 2) {
                std::string catName;
                std::cout << "Введите название новой категории: ";
                std::cin >> catName;
                if (DatabaseConnector::GetInstance().AddCategorySafe(ConvertToWideChar(catName))) {
                    std::cout << "[УСПЕХ] Категория добавлена.\n";
                }
                else {
                    std::cout << "[ОШИБКА] Не удалось добавить категорию.\n";
                }
            }
            else if (subChoice == 3) {
                if (currentUserRole != 1) {
                    std::cout << "[ОТКАЗ В ДОСТУПЕ] Только Администратор может удалять категории!\n";
                    break;
                }
                int catId;
                std::cout << "Введите ID категории для удаления: ";
                std::cin >> catId;
                if (DatabaseConnector::GetInstance().DeleteCategory(catId)) {
                    std::cout << "[УСПЕХ] Категория удалена.\n";
                }
                else {
                    std::cout << "[ОШИБКА] Не удалось удалить! Возможно, к этой категории привязаны детали (сработал запрет FK).\n";
                }
            }
            break;
        }

        case 12: {
            std::cout << "\n--- УПРАВЛЕНИЕ СКЛАДАМИ ---\n";
            std::cout << "1. Показать все склады\n2. Добавить склад\n3. Удалить склад (Только Админ)\nВыберите: ";
            int subChoice;
            if (!(std::cin >> subChoice)) { ClearInput(); break; }

            if (subChoice == 1) {
                DatabaseConnector::GetInstance().ShowWarehousesFromDB();
            }
            else if (subChoice == 2) {
                std::string locName;
                std::cout << "Введите адрес/название нового склада: ";
                std::cin >> locName;
                if (DatabaseConnector::GetInstance().AddWarehouse(ConvertToWideChar(locName))) {
                    std::cout << "[УСПЕХ] Склад добавлен.\n";
                }
                else {
                    std::cout << "[ОШИБКА] Не удалось добавить склад.\n";
                }
            }
            else if (subChoice == 3) {
                if (currentUserRole != 1) { // Проверка на админа 
                    std::cout << "[ОТКАЗ В ДОСТУПЕ] Только Администратор может удалять склады!\n";
                    break;
                }
                int wId;
                std::cout << "Введите ID склада для удаления: ";
                std::cin >> wId;
                if (DatabaseConnector::GetInstance().DeleteWarehouse(wId)) {
                    std::cout << "[УСПЕХ] Склад успешно удален (каскадное удаление запасов выполнено).\n";
                }
                else {
                    std::cout << "[ОШИБКА] Не удалось удалить склад.\n";
                }
            }
            break;
        }
        case 13: {
            std::cout << "\n--- МАССОВАЯ ПЕРЕОЦЕНКА ТОВАРОВ ---\n";
            if (currentUserRole != 1) { // Проверка на Администратора
                std::cout << "[ОТКАЗ В ДОСТУПЕ] Только Администратор имеет право массово менять цены!\n";
                break;
            }

            int catId;
            double percent;
            std::cout << "Введите ID категории: ";
            std::cin >> catId;
            std::cout << "Введите процент (например, 10 для наценки, -15 для скидки): ";
            std::cin >> percent;

            if (DatabaseConnector::GetInstance().BulkUpdateCategoryPrice(catId, percent)) {
                std::cout << "[УСПЕХ] Цены для всех деталей в категории #" << catId << " успешно обновлены на " << percent << "%!\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось обновить цены (неверный ID или в категории нет деталей).\n";
            }
            break;
        }
        case 0:
            std::cout << "Завершение работы программы...\n";
            break;
        default:
            std::cout << "Неверный пункт меню. Попробуйте снова.\n";
            break;
        }
    }

    DatabaseConnector::GetInstance().Disconnect();
    return 0;
}