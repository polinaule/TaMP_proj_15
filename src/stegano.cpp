// Программа для стеганографии в 24-битных BMP-файлах
// Встраивание сообщения в младшие биты (LSB) каждого байта пиксельных данных

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

bool embed_lsb_bmp(const std::string& input_bmp, const std::string& output_bmp, const std::string& message) {
    std::ifstream in(input_bmp, std::ios::binary);
    if (!in) return false;
    std::vector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    if (data.size() < 54) return false;
    int pixel_offset = *(int*)&data[10];
    if (pixel_offset < 54) pixel_offset = 54;

    std::vector<uint8_t> msg;
    uint32_t len = message.size();
    msg.push_back(len & 0xFF);
    msg.push_back((len >> 8) & 0xFF);
    msg.push_back((len >> 16) & 0xFF);
    msg.push_back((len >> 24) & 0xFF);
    msg.insert(msg.end(), message.begin(), message.end());

    size_t max_bytes = data.size() - pixel_offset;
    size_t required_bits = msg.size() * 8;
    if (required_bits > max_bytes) return false;

    for (size_t i = 0; i < msg.size(); ++i) {
        uint8_t byte = msg[i];
        for (int bit = 0; bit < 8; ++bit) {
            size_t idx = pixel_offset + i * 8 + bit;
            uint8_t& pixel_byte = reinterpret_cast<uint8_t&>(data[idx]);
            uint8_t bit_val = (byte >> (7 - bit)) & 1;
            pixel_byte = (pixel_byte & 0xFE) | bit_val;
        }
    }
    std::ofstream out(output_bmp, std::ios::binary);
    out.write(data.data(), data.size());
    return true;
}

std::string extract_lsb_bmp(const std::string& bmp_file) {
    std::ifstream in(bmp_file, std::ios::binary);
    if (!in) return "";
    std::vector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    if (data.size() < 54) return "";
    int pixel_offset = *(int*)&data[10];
    if (pixel_offset < 54) pixel_offset = 54;

    uint32_t len = 0;
    for (int i = 0; i < 32; ++i) {
        size_t idx = pixel_offset + i;
        if (idx >= data.size()) return "";
        uint8_t bit = (static_cast<uint8_t>(data[idx])) & 1;
        len = (len << 1) | bit;
    }
    if (len == 0 || len > 1024 * 1024) return "";

    std::vector<uint8_t> msg;
    for (size_t i = 0; i < len; ++i) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; ++bit) {
            size_t idx = pixel_offset + 32 + i * 8 + bit;
            if (idx >= data.size()) return "";
            uint8_t b = (static_cast<uint8_t>(data[idx])) & 1;
            byte = (byte << 1) | b;
        }
        msg.push_back(byte);
    }
    return std::string(msg.begin(), msg.end());
}

// Простой тест: требует наличия файла test_input.bmp (24-бит BMP)
#ifdef STEGANO_TEST
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
#endif