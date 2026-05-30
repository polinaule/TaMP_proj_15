// stegano.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <cstring>
#include <cstdint>

// Встраивание сообщения в 24-битный BMP (младшие биты каждого байта RGB)
bool embed_lsb_bmp(const std::string& input_bmp, const std::string& output_bmp, const std::string& message) {
    std::ifstream in(input_bmp, std::ios::binary);
    if (!in) return false;
    std::vector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    if (data.size() < 54) return false;
    int pixel_offset = *(int*)&data[10];
    if (pixel_offset < 54) pixel_offset = 54;

    std::vector<uint8_t> msg;
    uint32_t len = static_cast<uint32_t>(message.size());
    msg.push_back(len & 0xFF);
    msg.push_back((len >> 8) & 0xFF);
    msg.push_back((len >> 16) & 0xFF);
    msg.push_back((len >> 24) & 0xFF);
    msg.insert(msg.end(), message.begin(), message.end());

    size_t required_bits = msg.size() * 8;
    if (required_bits > (data.size() - pixel_offset)) return false;

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

// Извлечение скрытого сообщения из 24-битного BMP
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
        uint8_t bit = (static_cast<uint8_t>(data[idx])) & 1;
        len = (len << 1) | bit;
    }

    std::vector<uint8_t> msg;
    for (size_t i = 0; i < len; ++i) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; ++bit) {
            size_t idx = pixel_offset + 32 + i * 8 + bit;
            uint8_t b = (static_cast<uint8_t>(data[idx])) & 1;
            byte = (byte << 1) | b;
        }
        msg.push_back(byte);
    }
    return std::string(msg.begin(), msg.end());
}

// Простой тест (требует test_input.bmp)
int main() {
    std::cout << "Stegano test: Please provide a 24-bit BMP file named 'test_input.bmp'.\n";
    std::cout << "If not found, test is skipped.\n";

    std::ifstream check("test_input.bmp");
    if (check) {
        check.close();
        std::string msg = "Hello, world!";
        if (embed_lsb_bmp("test_input.bmp", "test_output.bmp", msg)) {
            std::string extracted = extract_lsb_bmp("test_output.bmp");
            assert(extracted == msg);
            std::cout << "Stegano test passed.\n";
        } else {
            std::cout << "Stegano embed failed.\n";
        }
    } else {
        std::cout << "Skipping stegano test (no input BMP).\n";
    }
    return 0;
}
