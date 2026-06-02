#include <iostream>
#include <cstring>
#include <cassert>
#include <cstdint>

// Фиксированный 8-байтовый ключ для демонстрации
static const uint8_t des_key[8] = { 0x13, 0x34, 0x57, 0x79, 0x9B, 0xBC, 0xDF, 0xF1 };

// Учебное "шифрование" блока 8 байт – просто XOR с ключом
void des_encrypt_block(const uint8_t input[8], uint8_t output[8], const uint8_t key[8]) {
    for (int i = 0; i < 8; ++i)
        output[i] = input[i] ^ key[i];
}

// Расшифрование – симметрично (XOR)
void des_decrypt_block(const uint8_t input[8], uint8_t output[8], const uint8_t key[8]) {
    for (int i = 0; i < 8; ++i)
        output[i] = input[i] ^ key[i];
}

// Тест: шифруем блок, потом расшифровываем, сравниваем
#ifdef DES_TEST
int main() {
    uint8_t plain[8] = { 'H','e','l','l','o','!','!','!' };
    uint8_t cipher[8], decrypted[8];

    des_encrypt_block(plain, cipher, des_key);
    des_decrypt_block(cipher, decrypted, des_key);

    assert(memcmp(plain, decrypted, 8) == 0);
    std::cout << "DES test passed.\n";
    return 0;
}
#endif