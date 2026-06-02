// db_manager.cpp
#include "db_manager.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <tuple>

// Конструктор: инициализируем указатель базы данных нулем, флаг закрыт
DBManager::DBManager() : db(nullptr), is_open(false) {}

// Деструктор: закрываем соединение с базой, если оно было открыто
DBManager::~DBManager() {
    if (db) sqlite3_close(db);
}

// Возврат единственного экземпляра синглтона
DBManager& DBManager::getInstance() {
    static DBManager instance;
    return instance;
}

// Открытие базы данных и создание таблицы users.
bool DBManager::init(const std::string& db_path) {
    // Попытка открыть базу данных (создаст файл, если его нет)
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    is_open = true;

    // SQL-запрос для создания таблицы users
    // id - первичный ключ с автоинкрементом
    // login - уникальное имя пользователя
    // password_hash - храним не пароль, а его хеш
    // role - роль пользователя
    const char* create_sql = "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "login TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "role TEXT DEFAULT 'user');";
    char* errmsg = nullptr;
    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errmsg << std::endl;
        sqlite3_free(errmsg);
        return false;
    }
    return true;
}

// Регистрация нового пользователя
bool DBManager::registerUser(const std::string& login, const std::string& password_hash) {
    if (!is_open) return false;
    // SQL-запрос на вставку новой записи
    std::string sql = "INSERT INTO users (login, password_hash) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    // Привязываем параметры: первый ? = login, второй ? = password_hash
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    // SQLITE_DONE означает успешное выполнение INSERT
    return (rc == SQLITE_DONE);
}

// Проверка логина и пароля
// Возвращает true, если найдена запись с таким логином и хешем пароля
// При успехе также возвращает id пользователя и его роль
bool DBManager::loginUser(const std::string& login, const std::string& password_hash,
    int& user_id, std::string& role) {
    if (!is_open) return false;
    // Ищем запись, где login и password_hash совпадают
    std::string sql = "SELECT id, role FROM users WHERE login = ? AND password_hash = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);

    bool found = false;
    // Если есть хотя бы одна строка результата, берем ее
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user_id = sqlite3_column_int(stmt, 0);          // id - первый столбец
        role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)); // role - второй
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

std::vector<std::tuple<int, std::string, std::string>> DBManager::getAllUsers() {
    std::vector<std::tuple<int, std::string, std::string>> users;
    if (!is_open) return users;
    std::string sql = "SELECT id, login, role FROM users ORDER BY id;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return users;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string login = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        users.push_back({ id, login, role });
    }
    sqlite3_finalize(stmt);
    return users;
}

bool DBManager::deleteUser(const std::string& login) {
    if (!is_open) return false;
    std::string sql = "DELETE FROM users WHERE login = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool DBManager::setUserRole(const std::string& login, const std::string& new_role) {
    if (!is_open) return false;
    std::string sql = "UPDATE users SET role = ? WHERE login = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, new_role.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, login.c_str(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

bool DBManager::isAdmin(const std::string& login, const std::string& password_hash) {
    if (!is_open) return false;
    std::string sql = "SELECT role FROM users WHERE login = ? AND password_hash = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);
    bool is_admin = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        is_admin = (role == "admin");
    }
    sqlite3_finalize(stmt);
    return is_admin;
}