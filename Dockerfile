# Базовый образ Ubuntu
FROM ubuntu:22.04

# Установка необходимых пакетов
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libsqlite3-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Рабочая директория
WORKDIR /app

# Копируем исходники
COPY server/ server/
COPY src/ src/

# Компиляция сервера
RUN g++ -std=c++17 server/db_manager.cpp server/server.cpp src/chord.cpp src/des.cpp src/stegano.cpp src/sha384.cpp -o server.exe -lsqlite3 -lssl -lcrypto -lpthread

# Открываем порт
EXPOSE 12345

# Запуск
CMD ["./server.exe"]
