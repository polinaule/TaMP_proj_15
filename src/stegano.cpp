// Программа для стеганографии в 24-битных BMP-файлах
// Встраивание сообщения в младшие биты (LSB) каждого байта пиксельных данных

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <cstring>


// Встраивание сообщения в 24-битный BMP с использованием метода LSB
// Параметры:
// input_bmp - путь к исходному BMP-файлу
// output_bmp - путь для сохранения результирующего BMP
// message - строка, которую нужно скрыть
// Возвращает true в случае успеха, false при ошибке
bool embed_lsb_bmp(const std::string& input_bmp,
    const std::string& output_bmp,
    const std::string& message)
{
    // Открываем исходный файл в бинарном режиме
    std::ifstream in(input_bmp, std::ios::binary);
    if (!in) return false;

    // Читаем весь файл в вектор байт
    std::vector<char> data((std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    in.close();

    // Проверяем минимальный размер заголовка BMP (54 байта для 24-бит)
    if (data.size() < 54) return false;

    // Смещение до пиксельных данных хранится по адресу 10 (4 байта, little-endian)
    int pixel_offset = *(int*)&data[10];
    if (pixel_offset < 54) pixel_offset = 54;  // защита от некорректных файлов


    // Подготовка сообщения: сначала 4 байта длины (little-endian),
    // затем сами символы сообщения.
    std::vector<uint8_t> msg;
    uint32_t len = static_cast<uint32_t>(message.size());

    // Упаковываем длину в 4 байта (младший байт первым)
    msg.push_back(len & 0xFF);
    msg.push_back((len >> 8) & 0xFF);
    msg.push_back((len >> 16) & 0xFF);
    msg.push_back((len >> 24) & 0xFF);

    // Добавляем байты самого сообщения
    msg.insert(msg.end(), message.begin(), message.end());


    // Оценка емкости изображения:
    // Каждый байт пиксельных данных может хранить 1 бит сообщения.
    // Поэтому для сообщения размером msg.size() байт нужно
    // msg.size() * 8 байт в пиксельной области.

    size_t max_bytes = data.size() - pixel_offset;           // доступно байт под пиксели
    size_t required_bits = msg.size() * 8;                   // сколько бит нужно сохранить
    if (required_bits > max_bytes) return false;             // недостаточно места


    // Встраивание битов сообщения в LSB каждого байта пикселей.
    // Для каждого байта сообщения берем его биты (от старшего к младшему)
    // и заменяем LSB соответствующего пиксельного байта.
    for (size_t i = 0; i < msg.size(); ++i) {
        uint8_t byte = msg[i];               // текущий байт сообщения
        for (int bit = 0; bit < 8; ++bit) {
            // Индекс пиксельного байта, в который запишем текущий бит
            size_t idx = pixel_offset + i * 8 + bit;

            // Получаем ссылку на байт пиксельных данных (изменяем через reinterpret_cast)
            uint8_t& pixel_byte = reinterpret_cast<uint8_t&>(data[idx]);

            // Извлекаем бит сообщения (слева направо, бит 7 -> бит 0)
            uint8_t bit_val = (byte >> (7 - bit)) & 1;

            // Обнуляем младший бит и записываем новый
            pixel_byte = (pixel_byte & 0xFE) | bit_val;
        }
    }

    // Записываем измененные данные в выходной файл
    std::ofstream out(output_bmp, std::ios::binary);
    out.write(data.data(), data.size());
    return true;
}


// Извлечение скрытого сообщения из 24-битного BMP (LSB-метод).
// Параметр:
// bmp_file - путь к BMP-файлу, возможно содержащему скрытое сообщение
// Возвращает извлеченную строку (пустую строку при ошибке).
std::string extract_lsb_bmp(const std::string& bmp_file)
{
    // Открываем файл и читаем все данные
    std::ifstream in(bmp_file, std::ios::binary);
    if (!in) return "";
    std::vector<char> data((std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());
    in.close();

    if (data.size() < 54) return "";          // некорректный заголовок

    // Смещение до пиксельных данных (поле по адресу 10)
    int pixel_offset = *(int*)&data[10];
    if (pixel_offset < 54) pixel_offset = 54;


    // Сначала извлекаем длину сообщения (4 байта = 32 бита).
    // Длина хранится в первых 32 байтах пиксельной области.
    // Каждый байт дает 1 бит.
    uint32_t len = 0;
    for (int i = 0; i < 32; ++i) {
        size_t idx = pixel_offset + i;               // индекс байта в пиксельных данных
        uint8_t bit = (static_cast<uint8_t>(data[idx])) & 1;   // извлекаем LSB
        len = (len << 1) | bit;                      // собираем число, бит за битом
    }


    // Извлекаем само сообщение: len байт, каждый байт = 8 бит из следующих
    // пиксельных байтов. Отступ после длины: 32 байта.
    std::vector<uint8_t> msg;
    for (size_t i = 0; i < len; ++i) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; ++bit) {
            size_t idx = pixel_offset + 32 + i * 8 + bit;
            uint8_t b = (static_cast<uint8_t>(data[idx])) & 1;
            byte = (byte << 1) | b;                // собираем байт
        }
        msg.push_back(byte);
    }

    // Возвращаем строку, собранную из извлеченных байтов
    return std::string(msg.begin(), msg.end());
}

// Простой тест: требует наличия файла test_input.bmp (24-бит BMP)
int main()
{
    std::cout << "Стеганография (LSB в 24-бит BMP).\n";
    std::cout << "Пожалуйста, предоставьте 24-битный BMP-файл с именем 'test_input.bmp'.\n";
    std::cout << "Если файл не найден, тест будет пропущен.\n";

    std::ifstream check("test_input.bmp");
    if (check) {
        check.close();

        std::string msg = "Hello, world!";          // тестовое сообщение
        if (embed_lsb_bmp("test_input.bmp", "test_output.bmp", msg)) {
            std::string extracted = extract_lsb_bmp("test_output.bmp");
            assert(extracted == msg);               // проверяем совпадение
            std::cout << "Тест стеганографии пройден успешно!\n";
        }
        else {
            std::cout << "Ошибка при встраивании сообщения.\n";
        }
    }
    else {
        std::cout << "Тест пропущен: файл test_input.bmp не найден.\n";
    }

    return 0;
}