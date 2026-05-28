// server.cpp
#include "db_manager.h"
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <thread>
#include <chrono>

// Раздельная компиляция для Windows и POSIX (Linux/macOS)
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

const int PORT = 12345;           // Порт, на котором слушает сервер
const int BUFFER_SIZE = 4096;     // Размер буфера для приема данных

// Заглушка для хеш-функции SHA-384
// В реальном проекте нужно подключить OpenSSL и вычислять настоящий хеш
// Здесь для демонстрации просто добавляем суффикс "_dummyhash"
std::string sha384_hash(const std::string& input) {
    return input + "_dummyhash";
}

// Обработка одного клиентского соединения
// Читает команду, вызывает методы DBManager, отправляет ответ
void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes <= 0) {
        closesocket(client_socket);
        return;
    }
    buffer[bytes] = '\0';   // Добавляем завершающий ноль, чтобы работать как со строкой
    std::string request(buffer);
    std::istringstream iss(request);
    std::string command, login, password;
    iss >> command >> login >> password;

    DBManager& db = DBManager::getInstance();
    std::string response;

    if (command == "REGISTER") {
        // При регистрации сохраняем не пароль, а его хеш
        std::string pass_hash = sha384_hash(password);
        if (db.registerUser(login, pass_hash)) {
            response = "OK Registration successful\n";
        }
        else {
            response = "ERROR Registration failed (user exists or DB error)\n";
        }
    }
    else if (command == "LOGIN") {
        // Для входа также хешируем полученный пароль и сравниваем с хранимым хешем
        std::string pass_hash = sha384_hash(password);
        int user_id;
        std::string role;
        if (db.loginUser(login, pass_hash, user_id, role)) {
            response = "OK Login successful. ID=" + std::to_string(user_id) +
                ", role=" + role + "\n";
        }
        else {
            response = "ERROR Invalid credentials\n";
        }
    }
    else {
        response = "ERROR Unknown command\n";
    }

    // Отправляем ответ клиенту
    send(client_socket, response.c_str(), response.size(), 0);
    closesocket(client_socket);
}

int main() {
#ifdef _WIN32
    // Инициализация Winsock (требуется только на Windows)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // Инициализируем базу данных (создем файл app.db и таблицу users)
    if (!DBManager::getInstance().init("app.db")) {
        std::cerr << "Failed to init database" << std::endl;
        return 1;
    }
    std::cout << "Database ready." << std::endl;

    // Создаем TCP-сокет
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    // Привязываем сокет к любому сетевому интерфейсу и заданному порту
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);   // преобразование в сетевой порядок байт
    if (bind(listen_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(listen_sock);
        return 1;
    }

    // Начинаем слушать входящие соединения (очередь до 5 клиентов)
    if (listen(listen_sock, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(listen_sock);
        return 1;
    }
    std::cout << "Server listening on port " << PORT << std::endl;

    // Основной цикл: принимаем соединения и обрабатываем их по очереди (синхронно)
    while (true) {
        SOCKET client_sock = accept(listen_sock, nullptr, nullptr);
        if (client_sock == INVALID_SOCKET) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        // Обрабатываем клиента
        handle_client(client_sock);
    }

    // Закрываем слушающий сокет
    closesocket(listen_sock);
#ifdef _WIN32
    WSACleanup();   // Очистка Winsock
#endif
    return 0;
}