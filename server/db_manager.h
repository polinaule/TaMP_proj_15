// db_manager.h
#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <string>
#include <sqlite3.h>
#include <tuple>
#include <vector>

// Класс-синглтон для работы с базой данных SQLite
// Обеспечивает регистрацию и проверку логинов/паролей
class DBManager {
public:
    // Получение единственного экземпляра класса (синглтон)
    static DBManager& getInstance();

    // Открытие базы данных и создание таблицы users (если не существует)
    // db_path - путь к файлу базы данных (по умолчанию "app.db")
    bool init(const std::string& db_path = "app.db");

    // Регистрация нового пользователя
    // login - имя пользователя (должно быть уникальным)
    // password_hash - хеш пароля (например, SHA-384)
    bool registerUser(const std::string& login, const std::string& password_hash);

    // Проверка учетных данных пользователя
    // При успехе возвращает true, а также заполняет user_id и role
    bool loginUser(const std::string& login, const std::string& password_hash,
        int& user_id, std::string& role);

    // Закрытие соединения с базой данных
    ~DBManager();

    // Роли и админ-функции
    std::vector<std::tuple<int, std::string, std::string>> getAllUsers();
    bool deleteUser(const std::string& login);
    bool setUserRole(const std::string& login, const std::string& new_role);
    bool isAdmin(const std::string& login, const std::string& password_hash);

private:
    DBManager();                              // Приватный конструктор
    DBManager(const DBManager&) = delete;     // Запрет копирования
    DBManager& operator=(const DBManager&) = delete; // Запрет присваивания

    sqlite3* db;       // Указатель на объект базы данных SQLite
    bool is_open;      // Флаг: открыта ли база
};

#endif