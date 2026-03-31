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
bool SafeGetInt(const std::string& prompt, int& outValue) {
    std::string input;
    std::cout << prompt;
    std::cin >> input;
    try {
        outValue = std::stoi(input); 
        ClearInput(); 
        return true;
    }
    catch (...) {
        std::cout << "[ОШИБКА] Введены недопустимые символы! Ожидается целое число. Отмена операции...\n";
        ClearInput();
        return false; 
    }
}

bool SafeGetDouble(const std::string& prompt, double& outValue) {
    std::string input;
    std::cout << prompt;
    std::cin >> input;
    try {
        outValue = std::stod(input);
        ClearInput();
        return true;
    }
    catch (...) {
        std::cout << "[ОШИБКА] Неверный формат! Ожидается число (например 1500.50). Отмена операции...\n";
        ClearInput();
        return false;
    }
}
int GetIntInput(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            return value;
        }
        else {
            std::cout << "[ОШИБКА] Ожидается число! Пожалуйста, не вводите буквы.\n";
            ClearInput();
        }
    }
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
    std::cout << "7. Управление заказами (Просмотр, Статусы, Удаление)\n";
    std::cout << "8. Удалить деталь (Только для Администратора)\n";
    std::cout << "9. Аналитика: Топ-5 прибыльных деталей (Оконные функции)\n";
    std::cout << "10. Изменить цену детали (Проверка AuditLog)\n";
    std::cout << "11. Управление категориями (CRUD)\n";
    std::cout << "12. Управление складами (CRUD)\n";
    std::cout << "13. Массовая переоценка категории (Скидки/Инфляция)\n";
    std::cout << "14. Управление сотрудниками (Только Админ)\n";
    std::cout << "0. Выход\n";
    std::cout << "=================================================\n";
    std::cout << "Выберите действие: ";
}
void RunValidatorTests() {
    std::cout << "\n--- Запуск автоматических тестов (Валидация) ---\n";


    std::cout << "[+] Тест 1 (Позитивный): Корректный Email 'supplier@tech.com' -> ";
    std::cout << (Validator::IsValidEmail("supplier@tech.com") ? "УСПЕХ" : "ОШИБКА") << "\n";

    std::cout << "[+] Тест 2 (Позитивный): Корректная цена '1500.50' -> ";
    std::cout << (Validator::IsValidPrice("1500.50") ? "УСПЕХ" : "ОШИБКА") << "\n";
 
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
    std::string menuInput;

    while (choice != 0) {
        ShowMenu();

        std::cin >> menuInput;
        try {
            choice = std::stoi(menuInput);
        }
        catch (...) {
            std::cout << "[ОШИБКА] Неверный ввод! Введите цифру пункта меню.\n";
            ClearInput();
            continue;
        }

        switch (choice) {
        case 1: {
            std::string login, password;
            std::cout << "\n--- АВТОРИЗАЦИЯ ---\n";
            std::cout << "Введите логин: "; std::cin >> login;
            std::cout << "Введите пароль: "; std::cin >> password;

            std::wstring wLogin = ConvertToWideChar(login);
            std::wstring wPassword = ConvertToWideChar(password);

            currentUserRole = DatabaseConnector::GetInstance().AuthenticateUser(wLogin, wPassword);

            if (currentUserRole > 0) {
                std::cout << "[УСПЕХ] Вы успешно авторизовались! Ваша роль ID: " << currentUserRole << "\n";
                DatabaseConnector::GetInstance().CheckLowStockAlerts();
            }
            else {
                std::cout << "[ОШИБКА] Неверный логин или пароль!\n";
            }
            break;
        }
        case 2: {
            std::string name;
            double price;
            int catId, supId;

            std::cout << "\n--- ДОБАВЛЕНИЕ ДЕТАЛИ ---\n";
            std::cout << "Введите название детали: ";
            std::cin >> name;
            if (!SafeGetDouble("Введите цену (формат 0.00): ", price)) break;

            std::cout << "\nДоступные категории:\n";
            DatabaseConnector::GetInstance().ShowCategoriesFromDB();

            if (!SafeGetInt("\nВведите ID категории из списка выше: ", catId)) break;
            if (!SafeGetInt("Введите ID поставщика: ", supId)) break;

            if (DatabaseConnector::GetInstance().AddPartSafe(ConvertToWideChar(name), catId, supId, price)) {
                std::cout << "[УСПЕХ] Деталь '" << name << "' успешно добавлена в базу данных!\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось добавить деталь в БД (проверьте ID категории/поставщика).\n";
            }
            break;
        }
        case 3: {
            std::cout << "\n--- МНОГОКРИТЕРИАЛЬНЫЙ ПОИСК И ПАГИНАЦИЯ ---\n";
            double minP, maxP;
            int catId, page;

            if (!SafeGetDouble("1. Введите минимальную цену: ", minP)) break;
            if (!SafeGetDouble("2. Введите максимальную цену: ", maxP)) break;
            if (!SafeGetInt("3. Введите ID категории: ", catId)) break;
            if (!SafeGetInt("4. Введите номер страницы: ", page)) break;

            if (page < 1) page = 1;

            std::cout << "\nРезультаты (Страница " << page << ", лимит 5 записей на страницу):\n";
            DatabaseConnector::GetInstance().SearchPartsPaginated(minP, maxP, catId, page, 5);
            break;
        }
        case 4: {
            std::cout << "\n--- ОФОРМЛЕНИЕ ЗАКАЗА ---\n";
            int uId, pId, qty, wId;

            if (!SafeGetInt("Введите ID пользователя (вас): ", uId)) break;
            if (!SafeGetInt("Введите ID детали для заказа: ", pId)) break;
            if (!SafeGetInt("Введите количество: ", qty)) break;
            if (!SafeGetInt("Введите ID склада для списания: ", wId)) break;

            if (DatabaseConnector::GetInstance().CreateOrderTransaction(uId, pId, qty, wId)) {
                std::cout << "[УСПЕХ] Заказ успешно оформлен! Остатки списаны.\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось оформить заказ! (Возможно не хватает остатков).\n";
            }
            break;
        }
        case 5:
            std::cout << "\n--- ЭКСПОРТ В CSV ---\n";
            if (DatabaseConnector::GetInstance().ExportOrdersToCSV("report_orders.csv")) {
                std::cout << "[УСПЕХ] Отчет сформирован и сохранен в файл 'report_orders.csv'.\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось создать отчет.\n";
            }
            break;
        case 6:
            RunValidatorTests();
            break;
        case 7: {
            std::cout << "\n--- УПРАВЛЕНИЕ ЗАКАЗАМИ ---\n";
            std::cout << "1. Показать все заказы\n2. Завершить заказ (Completed)\n3. Удалить заказ (Только Админ)\n";
            int subChoice;
            if (!SafeGetInt("Выберите действие: ", subChoice)) break;

            if (subChoice == 1) {
                DatabaseConnector::GetInstance().ShowOrdersFromDB();
            }
            else if (subChoice == 2) {
                int oId;
                if (!SafeGetInt("Введите ID заказа для завершения: ", oId)) break;
                if (DatabaseConnector::GetInstance().CompleteOrder(oId)) {
                    std::cout << "[УСПЕХ] Заказ #" << oId << " переведен в статус 'Completed'.\n";
                }
                else {
                    std::cout << "[ОШИБКА] Не удалось обновить заказ.\n";
                }
            }
            else if (subChoice == 3) {
                if (currentUserRole != 1) {
                    std::cout << "[ОТКАЗ В ДОСТУПЕ] Только Администратор может удалять заказы!\n";
                    break;
                }
                int oId;
                if (!SafeGetInt("Введите ID заказа для удаления: ", oId)) break;
                if (DatabaseConnector::GetInstance().DeleteOrder(oId)) {
                    std::cout << "[УСПЕХ] Заказ успешно удален.\n";
                }
                else {
                    std::cout << "[ОШИБКА] Не удалось удалить заказ.\n";
                }
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
            if (!SafeGetInt("Введите ID детали для удаления: ", pId)) break;

            if (DatabaseConnector::GetInstance().DeletePartSafe(pId)) {
                std::cout << "[УСПЕХ] Деталь #" << pId << " успешно удалена.\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось удалить деталь (Связи FK).\n";
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
            double newPrice;

            if (!SafeGetInt("Введите ID детали: ", pId)) break;
            if (!SafeGetDouble("Введите новую цену: ", newPrice)) break;

            if (DatabaseConnector::GetInstance().UpdatePartPrice(pId, newPrice)) {
                std::cout << "[УСПЕХ] Цена обновлена! Сработал SQL-триггер AuditLog.\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось обновить цену.\n";
            }
            break;
        }
        case 11: {
            std::cout << "\n--- УПРАВЛЕНИЕ КАТЕГОРИЯМИ ---\n";
            std::cout << "1. Показать категории\n2. Добавить категорию\n3. Удалить категорию (Админ)\n";
            int subChoice;
            if (!SafeGetInt("Выберите действие: ", subChoice)) break;

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
            }
            else if (subChoice == 3) {
                if (currentUserRole != 1) {
                    std::cout << "[ОТКАЗ В ДОСТУПЕ] Только Администратор может удалять!\n";
                    break;
                }
                int catId;
                if (!SafeGetInt("Введите ID категории для удаления: ", catId)) break;
                if (DatabaseConnector::GetInstance().DeleteCategory(catId)) {
                    std::cout << "[УСПЕХ] Категория удалена.\n";
                }
                else {
                    std::cout << "[ОШИБКА] Не удалось удалить (работает запрет FK).\n";
                }
            }
            break;
        }
        case 12: {
            std::cout << "\n--- УПРАВЛЕНИЕ СКЛАДАМИ ---\n";
            std::cout << "1. Показать склады\n2. Добавить склад\n3. Удалить склад (Админ)\n";
            int subChoice;
            if (!SafeGetInt("Выберите действие: ", subChoice)) break;

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
            }
            else if (subChoice == 3) {
                if (currentUserRole != 1) {
                    std::cout << "[ОТКАЗ В ДОСТУПЕ] Только Администратор может удалять!\n";
                    break;
                }
                int wId;
                if (!SafeGetInt("Введите ID склада для удаления: ", wId)) break;
                if (DatabaseConnector::GetInstance().DeleteWarehouse(wId)) {
                    std::cout << "[УСПЕХ] Склад успешно удален.\n";
                }
            }
            break;
        }
        case 13: {
            std::cout << "\n--- МАССОВАЯ ПЕРЕОЦЕНКА ТОВАРОВ ---\n";
            if (currentUserRole != 1) {
                std::cout << "[ОТКАЗ В ДОСТУПЕ] Только Администратор!\n";
                break;
            }
            int catId;
            double percent;

            if (!SafeGetInt("Введите ID категории: ", catId)) break;
            if (!SafeGetDouble("Введите процент (наценка 10, скидка -15): ", percent)) break;

            if (DatabaseConnector::GetInstance().BulkUpdateCategoryPrice(catId, percent)) {
                std::cout << "[УСПЕХ] Цены обновлены на " << percent << "%!\n";
            }
            else {
                std::cout << "[ОШИБКА] Не удалось обновить цены.\n";
            }
            break;
        }
        case 14: {
            std::cout << "\n--- УПРАВЛЕНИЕ СОТРУДНИКАМИ ---\n";
            if (currentUserRole != 1) {
                std::cout << "[ОТКАЗ В ДОСТУПЕ] Только Администратор!\n";
                break;
            }
            std::cout << "1. Показать сотрудников\n2. Добавить сотрудника\n3. Уволить (Удалить)\n";
            int subChoice;
            if (!SafeGetInt("Выберите действие: ", subChoice)) break;

            if (subChoice == 1) {
                DatabaseConnector::GetInstance().ShowUsersFromDB();
            }
            else if (subChoice == 2) {
                std::string newUser, newPass;
                int rId;
                std::cout << "Введите логин нового сотрудника: "; std::cin >> newUser;
                std::cout << "Введите пароль: "; std::cin >> newPass;
                if (!SafeGetInt("Введите ID роли (1-Admin, 2-Manager, 3-Worker): ", rId)) break;

                if (DatabaseConnector::GetInstance().AddUser(ConvertToWideChar(newUser), ConvertToWideChar(newPass), rId)) {
                    std::cout << "[УСПЕХ] Сотрудник добавлен.\n";
                }
            }
            else if (subChoice == 3) {
                int uId;
                if (!SafeGetInt("Введите ID сотрудника для увольнения: ", uId)) break;
                if (DatabaseConnector::GetInstance().DeleteUser(uId)) {
                    std::cout << "[УСПЕХ] Сотрудник удален.\n";
                }
                else {
                    std::cout << "[ОШИБКА] У сотрудника есть оформленные заказы (FK).\n";
                }
            }
            break;
        }
        case 0:
            std::cout << "Завершение работы программы...\n";
            break;
        default:
            std::cout << "[ОШИБКА] Неверный пункт меню. Попробуйте снова.\n";
            break;
        }
    }

    DatabaseConnector::GetInstance().Disconnect();
    return 0;
}