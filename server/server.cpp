// server.cpp
#include "db_manager.h"
#include "sha384.h"
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <thread>
#include <chrono>
#include "../src/chord.h"
#include "../src/des.h"
#include "../src/stegano.h"
#include <sstream>
#include <cmath>
#include <iomanip>
#include <tuple>

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

// Обработка одного клиентского соединения
// Читает команду, вызывает методы DBManager, отправляет ответ
void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes <= 0) {
        closesocket(client_socket);
        return;
    }
    buffer[bytes] = '\0';
    std::string request(buffer);
    std::istringstream iss(request);
    std::string command;
    iss >> command;   // читаем только команду

    std::string response;

    if (command == "REGISTER") {
        std::string login, password;
        if (!(iss >> login >> password)) {
            response = "ERROR Usage: REGISTER login password\n";
        }
        else {
            std::string pass_hash = sha384_hash(password);
            if (DBManager::getInstance().registerUser(login, pass_hash)) {
                response = "OK Registration successful\n";
            }
            else {
                response = "ERROR Registration failed (user exists or DB error)\n";
            }
        }
    }
    else if (command == "LOGIN") {
        std::string login, password;
        if (!(iss >> login >> password)) {
            response = "ERROR Usage: LOGIN login password\n";
        }
        else {
            std::string pass_hash = sha384_hash(password);
            int user_id;
            std::string role;
            if (DBManager::getInstance().loginUser(login, pass_hash, user_id, role)) {
                response = "OK Login successful. ID=" + std::to_string(user_id) + ", role=" + role + "\n";
            }
            else {
                response = "ERROR Invalid credentials\n";
            }
        }
    }
    else if (command == "CHORD") {
        double a, b, eps;
        if (!(iss >> a >> b >> eps)) {
            response = "ERROR Invalid parameters. Usage: CHORD a b eps\n";
        }
        else {
            auto func = [](double x) { return x * x - 4; };
            double root = chord_method(a, b, eps, func);
            if (std::isnan(root)) {
                response = "ERROR Chord method failed (root not bracketed)\n";
            }
            else {
                response = "OK Root = " + std::to_string(root) + "\n";
            }
        }
    }
    else if (command == "DES_ENCRYPT") {
        std::string key_str, plain_str;
        if (!(iss >> key_str >> plain_str)) {
            response = "ERROR Usage: DES_ENCRYPT key8bytes plain8bytes\n";
        }
        else if (key_str.size() != 8 || plain_str.size() != 8) {
            response = "ERROR Key and plaintext must be 8 characters each\n";
        }
        else {
            uint8_t key[8], plain[8], cipher[8];
            memcpy(key, key_str.c_str(), 8);
            memcpy(plain, plain_str.c_str(), 8);
            des_encrypt_block(plain, cipher, key);
            std::stringstream hex;
            for (int i = 0; i < 8; ++i)
                hex << std::hex << (int)cipher[i];
            response = "OK Cipher: " + hex.str() + "\n";
        }
    }
    else if (command == "STEGANO_EMBED") {
        std::string input_bmp, output_bmp, message;
        if (!(iss >> input_bmp >> output_bmp)) {
            response = "ERROR Usage: STEGANO_EMBED input.bmp output.bmp \"message\"\n";
        }
        else {
            std::getline(iss, message);
            // убираем кавычки, если есть
            size_t first = message.find_first_not_of(" \t");
            if (first != std::string::npos && message[first] == '"') {
                message = message.substr(first + 1);
                size_t last = message.find_last_of('"');
                if (last != std::string::npos) message = message.substr(0, last);
            }
            if (embed_lsb_bmp(input_bmp, output_bmp, message)) {
                response = "OK Message embedded into " + output_bmp + "\n";
            }
            else {
                response = "ERROR Failed to embed (check file existence, format, or capacity)\n";
            }
        }
    }
    else if (command == "STEGANO_EXTRACT") {
        response = "OK Extracted\n";
    }
    else if (command == "ADMIN_GET_USERS") {
        std::string admin_login, admin_pass;
        if (!(iss >> admin_login >> admin_pass)) {
            response = "ERROR Usage: ADMIN_GET_USERS login password\n";
        }
        else {
            std::string pass_hash = sha384_hash(admin_pass);
            if (DBManager::getInstance().isAdmin(admin_login, pass_hash)) {
                auto users = DBManager::getInstance().getAllUsers();
                response = "OK Users:\n";
                for (const auto& u : users) {
                    response += std::to_string(std::get<0>(u)) + " " + std::get<1>(u) + " " + std::get<2>(u) + "\n";
                }
            }
            else {
                response = "ERROR Access denied or invalid credentials\n";
            }
        }
    }
    else if (command == "ADMIN_DELETE_USER") {
        std::string admin_login, admin_pass, target_login;
        if (!(iss >> admin_login >> admin_pass >> target_login)) {
            response = "ERROR Usage: ADMIN_DELETE_USER admin_login admin_pass target_login\n";
        }
        else {
            std::string pass_hash = sha384_hash(admin_pass);
            if (DBManager::getInstance().isAdmin(admin_login, pass_hash)) {
                if (target_login == admin_login) {
                    response = "ERROR Cannot delete yourself\n";
                }
                else if (DBManager::getInstance().deleteUser(target_login)) {
                    response = "OK User " + target_login + " deleted\n";
                }
                else {
                    response = "ERROR User not found or delete failed\n";
                }
            }
            else {
                response = "ERROR Access denied or invalid credentials\n";
            }
        }
    }
    else if (command == "ADMIN_SET_ROLE") {
        std::string admin_login, admin_pass, target_login, new_role;
        if (!(iss >> admin_login >> admin_pass >> target_login >> new_role)) {
            response = "ERROR Usage: ADMIN_SET_ROLE admin_login admin_pass target_login new_role\n";
        }
        else {
            if (new_role != "user" && new_role != "admin") {
                response = "ERROR Role must be 'user' or 'admin'\n";
            }
            else {
                std::string pass_hash = sha384_hash(admin_pass);
                if (DBManager::getInstance().isAdmin(admin_login, pass_hash)) {
                    if (DBManager::getInstance().setUserRole(target_login, new_role)) {
                        response = "OK Role of " + target_login + " set to " + new_role + "\n";
                    }
                    else {
                        response = "ERROR User not found or update failed\n";
                    }
                }
                else {
                    response = "ERROR Access denied or invalid credentials\n";
                }
            }
        }
    }
    else {
        response = "ERROR Unknown command\n";
    }

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