#ifndef DES_H
#define DES_H

#include <cstdint>
void des_encrypt_block(const uint8_t input[8], uint8_t output[8], const uint8_t key[8]);
void des_decrypt_block(const uint8_t input[8], uint8_t output[8], const uint8_t key[8]);

#endif
